#include "Solver.hh"

void Solver::init(int nL, int nC){
	percentage=0.0;
	watches= new std::vector<Watcher>[(2*nL)+1];
	watches=watches+nL;
	indexClauses= new std::vector<Clause*>[(2*nL)+1];
	indexClauses=indexClauses+nL;
	reason= new Clause[(2*nL)+1];
	reason=reason+nL;
	assignments.resize(nL+1, U);
	levels.resize(nL+1, 0);
	nLiterals=nL;nClauses=nC;
	activity = new float[(2*nL)+1];	
	for(int i=0; i<nL;++i){
		*activity=0;
		activity=activity+1;
	}
	int i;
	#pragma omp parallel for private(i)
	for(i=0; i<nL+1;++i) activity[i]=0;

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
			indexClauses[x.val()].push_back(clauses.back());
			activity[x.val()]++;
		}
		if(size==1){
			enqueue(lits_val[0]);
			return;
		}

		Clause* c = clauses.back();
		//check if new clause sussumes others
		for(size_t i=0; i<c->size();++i){
			int x = c->at(i).val();
			for(unsigned int i=0; i<indexClauses[x].size();++i){
				if(!c->isDel()&&indexClauses[x][i]->isDel()){
					if(*indexClauses[x][i]<*c){
						deletedClauses++;
						c->del();
						break;
					}else if(*c<*indexClauses[x][i]){
						deletedClauses++;
						indexClauses[x][i]->del();
					}
				}
			}
		}
		/*
		for(int i=0; i<clauses.size();++i){

			if(!c->isDel()&&clauses[i]->isDel()){
				if(*clauses[i]<*c){
					deletedClauses++;
					c->del();
					break;
				}else if(*c<*clauses[i]){
					deletedClauses++;
					clauses[i]->del();
				}
			}
		}
		*/

		for(auto x : clauses){
			if(x->satisfied(this)&&!x->isDel()){
				deletedClauses++;
			       	x->del();
			}
		}
		if(!c->isDel())attachWatcher(clauses.back());
	}	
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
	indexClauses=indexClauses-nLiterals;
	delete [] indexClauses;
	activity=activity-nLiterals;
	delete [] activity;
	for(size_t i=0; i<learnts.size();i++) delete learnts[i];
	for(size_t i=0; i<clauses.size();i++) delete clauses[i];
};

Clause* Solver::propagate(){

	while(!propQ.empty()){
		Literal p = propQ.front();
		propQ.pop();
		bool not_conflict=true;
		int p_index=p.val();
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
		}else{
			Clause unit_c;unit_c.addLiteral(p);
			reason[p.val()]=unit_c;
		}
		trail.push_back(p);
		propQ.push(p);
		return true;
	}
};
#if PROVE
void Solver::completeResolvs(Clause* conflict){
	btree<Clause> tree;
	std::vector<Clause> tmp;
	Clause c_tmp=*conflict;
	for(int i=trail.size()-1;i>=0;i--){
		tmp.push_back(c_tmp);
		tmp.push_back(reason[trail[i].val()]);
		c_tmp=c_tmp&reason[trail[i].val()];
	}
	int i=tmp.size()-1;
	Clause empty=c_tmp;
	
	tree.addRdx(empty);
	for(;i>=0;--i){
		tree.insert(tmp[i-1], tmp[i]);
		--i;
	}
	prove.push_back(tree);
	return;
};
#endif

int Solver::analyze(Clause* conflict, std::vector<Literal>& to_learn, bool unsat){

	std::vector<Literal> p_reason;
	Literal p(0);
	std::vector<bool> seen(nLiterals+1, false);
	seen[p.index()]=true;
	Clause c = *conflict;
	int counter=0;
	int btLevel=0;
	to_learn.push_back(p);//!< leave place for asserting literal

#if PROVE
	btree<Clause> tree;
	std::vector<Clause> tmp;
	Clause c_tmp=*conflict;
	bool first=true;
#endif

	do{
		clear(p_reason);
		c.calcReason(this, p, p_reason);
		for(size_t i=0; i<p_reason.size();++i){
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
#if PROVE
		if(!first){
		tmp.push_back(c_tmp);
		tmp.push_back(c);
		c_tmp=c_tmp&c;
		}
		first=false;
#endif

		do{
			assert(trail.size()>0);
			p = trail.back();
			c = reason[p.val()];
			undoOne();
		}while(!seen[p.index()]);
		counter--;
	}while(counter>0);
	to_learn[0]=~p;
	if(to_learn.size()==1) btLevel=0;
	else{
		int max_i=1;
		for(size_t i=2;i<to_learn.size();i++){
			if(levels[to_learn[i].index()]>levels[to_learn[max_i].index()])max_i=i;
		}
		Literal p=to_learn[max_i];
		to_learn[max_i]=to_learn[1];
		to_learn[1]=p;
		btLevel=levels[p.index()];
	}
#if PROVE
	int i=tmp.size()-1;
	Clause empty=c_tmp;
	
	tree.addRdx(empty);
	for(;i>=0;--i){
		tree.insert(tmp[i-1], tmp[i]);
		--i;
	}
	prove.push_back(tree);
#endif

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

int Solver::select(){
	int max_so_far=-10;
	var x = 0;
	for(int i=-nLiterals;i<nLiterals+1;++i){
		if(i==0) continue;
		if(max_so_far<activity[i]&&assignments[i]==U){
			x=i;
			max_so_far=activity[i];
		}
	}
	return x;
};

void Solver::reset_scores(){
	for(int i=-nLiterals;i<nLiterals;++i){
		activity[i]=0;
	}
};

bool Solver::CDCL(){
	while(true){
#if VERBOSE
		if(percentage*4*100<=100){
		printf("[");
		int i=0;	
		for(;i<=(int)((percentage*4)*41);++i)printf("=");
		printf(">");
		for(;i<=41;++i)printf(" ");
		printf("%3d%%]\r",(int)((percentage*4)*100));
		}
#endif
		Clause* conflict=propagate();
		if(conflict!=nullptr){ //!< Exist a conflict
#if VERBOSE
#endif
			nConflicts++;
			std::vector<Literal> to_learn;int btLevel;
			if(decisionLevel()==root_level){	
#if PROVE
				completeResolvs(conflict);
#endif
				return false;}
			btLevel=analyze(conflict, to_learn);
			backtrack(btLevel);
			record(to_learn);
		}else{
#if ASSERT
			assert(canBeSAT());
#endif
			if(decisionLevel()==0){
				simplify();
				decayActivity();
#if VERBOSE
				percentage=((float)nAssigns())/((float)nLiterals);
#endif
			}
			if(learnts.size()-nAssigns()>=learnts.size()) reduceLearnts();
			if(nAssigns()==nLiterals) return true;
			else if(nConflicts>max_conflict) {
				nRestarts++;
				backtrack(root_level);
				lubyActivity();
			}
			else{
				Literal branching_literal(select());
				assume(branching_literal);
			}
		}
	};
	assert(false);
};

void Solver::backtrack(int btLevel){
	while(decisionLevel()>btLevel) cancel();
};

void Solver::simplify(){
	for(auto x:clauses){
		if(x->satisfied(this)&&!x->isDel()){
			deletedClauses++;
			x->del();
		}
	}
};

void Solver::reduceLearnts(){};

void Solver::cancel(){
	int c=trail.size()-trail_lim.back();
	for(;c!=0;c--) undoOne();
	trail_lim.pop_back();
};

int Solver::nAssigns(){
	int counter=0;
	#pragma omp parallel for
	for(size_t i=1; i<assignments.size();++i){
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
	#pragma omp parallel for
	for(i=-MAX;i<MAX+1;++i){
		if(activity[i]<=0)continue;
		activity[i]-=dc_factor;
	}
};

void Solver::reward(Clause* c){
	for(size_t i=0; i<c->size();++i){
		activity[c->at(i).val()]+=BONUS;
	}
};

//Method to verify
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
