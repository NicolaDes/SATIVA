#include "Solver.hh"

void Solver::init(int nL, int nC){
	watches= new std::vector<Watcher>[(2*nL)+1];
	watches=watches+nL;
	reason= new Clause[(2*nL)+1];
	reason=reason+nL;
	assignments.resize(nL+1, lbool(U));
	levels.resize(nL+1, 0);
	undos = new std::vector<Clause*>[(2*nL)+1];
	undos = undos + nL;
	nLiterals=nL;nClauses=nC;
	activity = new float[(2*nL)+1];	
	for(int i=0; i<nL;++i){
		*activity=0;
		activity=activity+1;
	}
	int i;
	#pragma omp parallel for private(i)
	for(i=0; i<nL+1;++i) activity[i]=0;

	max_conflict=4000;

	initialized=true;
}

void Solver::newClause(std::vector<Literal>& lits_val, bool learnt){
	if(!initialized){std::cerr<<"Solver not initialized!!\n\tSystem aborting...\n";exit(1);}
	int size=lits_val.size();
	assert(size>0);

	
	if(learnt){
#if ASSERT
		assert(value(lits_val[0])==U);
		for(int i=1;i<lits_val.size();++i){
			assert(value(lits_val[i])==F);
		}
#endif
		int learnt_size=learnts.size();
		learnts.push_back(new Clause);
		for(auto x : lits_val){
			learnts.back()->addLiteral(x);
			activity[x.val()]++;
		}

		if(size==1){
		Clause c; c.addLiteral(lits_val[0]);
		return;
		}
		reward(learnts.back());
		assert(learnts.back()->at(0)!=learnts.back()->at(1));
		attachWatcher(learnts.back());
	}else{

		clauses.push_back(new Clause);
		for(auto x : lits_val){
			clauses.back()->addLiteral(x);
			activity[x.val()]++;
		}
		if(size==1){
			enqueue(lits_val[0]);
			return;
		}

		for(auto x : clauses){
			if(x->satisfied(this)&&!x->isDel()){
				deletedClauses++;
			       	x->del();
			}
		}
//		attachWatcher(clauses.back());
	}	
}

void Solver::sussume(Clause& c){
	if(trail.size()==0)return;
	int nSussumption=0;
	for(auto it=clauses.begin();it!=clauses.end();++it){
		if(c<**it&&!(*it)->isDel()){
			(*it)->del();
			deletedClauses++;
//			(*it)->detach(this);
			nSussumption++;
		}
	}
	if(nSussumption>0)std::cout<<"nSussumptions: "<<nSussumption;
	nClauses-=nSussumption;
}

void Solver::attachWatcher(Clause* clause){
	watches[-clause->at(0).val()].push_back(Watcher(clause, clause->at(0)));
	watches[-clause->at(1).val()].push_back(Watcher(clause, clause->at(1)));
#if ASSERT
assertWatches(-clause->at(0).val());
assertWatches(-clause->at(1).val());
#endif
};

Solver::Solver(){};

Solver::~Solver(){
	watches=watches-nLiterals;
	delete [] watches;
	reason=reason-nLiterals;
	delete [] reason;
	undos=undos -nLiterals;
	delete [] undos;
	activity=activity-nLiterals;
	delete [] activity;
	for(int i=0; i<learnts.size();i++) delete learnts[i];
	for(int i=0; i<clauses.size();i++) delete clauses[i];
};

Clause* Solver::propagate(){

	while(!propQ.empty()){
		Literal p = propQ.front();
		propQ.pop();
		bool not_conflict=true;
		int p_index=p.val();
		int size=watches[p_index].size();
		std::vector<Watcher> tmp=watches[p_index];
		clear(watches[p_index]);
		for(auto x = tmp.begin();x!=tmp.end();++x){
			not_conflict=x->propagate(this, &p);
			if(!not_conflict){
				//clear propQ
				clear(propQ);
				//copy remaining watched literals
				auto i = x;
				x++;
				for(;x!=tmp.end();++x){
				       	watches[p_index].push_back(*x);
				}
				return i->cref;
			}
		}
	}       
	return nullptr; 
}; 
	
	
	
bool Solver::enqueue(Literal p, Clause* c){ 
	if(value(p)!=U){ 
		if(value(p)==F){
			nConflicts++;
			return false;
		}else{
			return true;
		}
	}else{
		nPropagations++;
		if(p.sign())assignments[p.index()]=F;
		else assignments[p.index()]=T;
		levels[p.index()]=decisionLevel();
		if(c!=nullptr)
			assert(c->size()>0);
		if(c!=nullptr){
			reason[p.val()]=*c;
		}
		trail.push_back(p);
		propQ.push(p);
		return true;
	}
};

int Solver::analyze(Clause* conflict, std::vector<Literal>& to_learn, bool unsat){
#if PROVE
	std::vector<Clause> resolution;
	resolution.push_back(*conflict);
#endif
	std::vector<Literal> p_reason;
	Literal p(0);
	std::vector<bool> seen(nLiterals+1, false);
	seen[p.index()]=true;
	Clause c = *conflict;
	int counter=0;
	int btLevel=0;
	to_learn.push_back(p);//!< leave place for asserting literal

	do{
		clear(p_reason);
		c.calcReason(this, p, p_reason);
		for(int i=0; i<p_reason.size();++i){
			Literal q = p_reason[i];
			if(!seen[q.index()]){
				seen[q.index()]=true;
				if(decisionLevel()==levels[q.index()]) counter++; //!< q need to be processed
				else if(decisionLevel()>0){ //!< exluding curr_level = 0
				       to_learn.push_back(~q);
				       btLevel=MAX(btLevel, levels[q.index()]);
				}
			}
		}
		do{
			assert(trail.size()>0);
			p = trail.back();
			c = reason[p.val()];
#if PROVE
			if(c.size()==0) goto end;
			resolution.push_back(c);
			resolution.push_back(resolution[resolution.size()-2]&c);
end:
#endif
			undoOne();
		}while(!seen[p.index()]);
		counter--;
	}while(counter>0);
	to_learn[0]=~p;
#if PROVE
	resolvents.push_back(resolution);
#endif	
	if(to_learn.size()==1) btLevel=0;
	else{
		int max_i=1;
		for(int i=2;i<to_learn.size();i++){
			if(levels[to_learn[i].index()]>levels[to_learn[max_i].index()])max_i=i;
		}
		Literal p=to_learn[max_i];
		to_learn[max_i]=to_learn[1];
		to_learn[1]=p;
		btLevel=levels[p.index()];
	}
	return btLevel;
};

void Solver::record(std::vector<Literal>& clause){
	newClause(clause, true);
	enqueue(clause[0], learnts.back());
};

void Solver::undoOne(){
	Literal p = trail.back();
	assignments[p.index()]=U;
	Clause empty;*(reason+p.val())=empty;
	levels[p.index()]=-1;
	trail.pop_back();
};

Literal Solver::select(){
	int max_so_far=-10;
	var x = 0;
	for(int i=-nLiterals;i<nLiterals+1;++i){
		if(i==0) continue;
		if(max_so_far<activity[i]&&assignments[i]==U){
			x=i;
			max_so_far=activity[i];
		}
	}
	Literal branch_variable(x);
	return branch_variable;
};

bool Solver::CDCL(){
	int nConflict=0;
	
//	sussume(clauses);

	for(auto x=clauses.begin();x!=clauses.end();++x){
		if((*x)->size()>1)
			attachWatcher(*x);
	}
	
	while(true){
		Clause* conflict=propagate();
		if(conflict!=nullptr){ //!< Exist a conflict
			nConflict++;
			std::vector<Literal> to_learn;int btLevel;
			if(decisionLevel()==root_level){ 
				analyze(conflict, to_learn);return false;}
			btLevel=analyze(conflict, to_learn);
			backtrack(btLevel);
			record(to_learn);
			decayActivity();
		}else{
#if ASSERT
			assert(canBeSAT());
#endif
			if(decisionLevel()==0) simplify();
			if(learnts.size()-nAssigns()>=learnts.size()) reduceLearnts();
			if(nAssigns()==nLiterals) return true;
			else if(nConflict>max_conflict) {
				nRestarts++;
				max_conflict+=4000;
				backtrack(root_level);
			}
			else{
				Literal p = select();
				assume(p);
			}
		}
	};
	assert(false);
};

void Solver::backtrack(int btLevel){
	while(decisionLevel()>btLevel) cancel();
};

void Solver::simplify(){
	int tmp=deletedClauses;
	for(auto x:clauses){
		if(x->satisfied(this)&&!x->isDel()){
			deletedClauses++;
			x->del();
		}
	}
	if(deletedClauses-tmp>0) std::cout<<"Deleted clauses: "<<deletedClauses-tmp<<"\n";
};

void Solver::reduceLearnts(){};

void Solver::cancel(){
	int c=trail.size()-trail_lim.back();
	for(;c!=0;c--) undoOne();
	trail_lim.pop_back();
};

void Solver::restart(){
	nRestarts++;
	backtrack(0);
}

int Solver::nAssigns(){
	int counter=0;
	for(int i=1; i<assignments.size();++i){
		if(assignments[i]!=U) counter++;
	}
	return counter;
};

bool Solver::assume(Literal& p){
	trail_lim.push_back(trail.size());
	nDecisions++;
	return enqueue(p);
};

void Solver::decayActivity(){
	int i;int MAX=nLiterals;const float dc_factor=DECAY_FACTOR;
	for(i=-MAX;i<MAX+1;++i){
		if(activity[i]<=0)continue;
		activity[i]-=dc_factor;
	}
};

#if ASSERT
bool Solver::isSAT(){
	for(int i=-nClauses;i<0;++i){
		bool sat=false;
		for(int j=0; j<clauses[i]->size();++j){
			sat=sat||(value(clauses[i]->at(j))==T?true:false);
		}
		if(!sat){
			return false;
		}
	}
	return true;
};

bool Solver::canBeSAT(){
	for(int i=0;i<nClauses;++i){
		bool sat=false;
		for(int j=0; j<clauses[i]->size();++j){
			if(value(clauses[i]->at(j))==T||value(clauses[i]->at(j))==U)
				sat=true;
		}
		if(!sat)return false;
	}
	for(int i=0; i<learnts.size();++i){
		bool sat=false;
		     for(int j=0;j<learnts[i]->size();++j){
			if(value(learnts[i]->at(j))==T||value(learnts[i]->at(j))==U)sat=true;
		     }
		if(!sat) return false;
	}
	return true;
};
#endif

void Solver::reward(Clause* c){
	for(int i=0; i<c->size();++i){
		activity[c->at(i).val()]+=BONUS;
	}
};

#if ASSERT
void Solver::assertWatches(int index){
	for(auto x : watches[index]){
		int count=0;
		for(auto y : watches[index]){
			if(x==y) count++;

		assert(count<2);
		}
	}
};
#endif
