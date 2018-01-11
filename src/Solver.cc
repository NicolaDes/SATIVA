#include "Solver.hh"

void Solver::init(int nL, int nC){
	watches= new std::vector<Clause*>[(2*nL)+1];
	watches=watches+nL;
	reason= new Clause[(2*nL)+1];
	reason=reason+nL;
	assignments.resize(nL+1, lbool(U));
	levels.resize(nL+1, 0);
	undos = new std::vector<Clause*>[(2*nL)+1];
	undos = undos + nL;
	clauses=new Clause[nC];
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
#if VERBOSE
	std::cout<<"newClause( ";
#endif
	if(learnt){
		int learnt_size=learnts.size();
		learnts.push_back(new Clause);
		for(auto x : lits_val){
			learnts[learnt_size]->addLiteral(x);
			activity[x.val()]++;
		}
		std::cout<<*learnts.back()<<" )\n";
		reward(learnts.back());
		return;
	}else{
		for(auto x : lits_val){
			clauses->addLiteral(x);
			activity[x.val()]++;
		}

		if(size==1){
#if VERBOSE
			std::cout<<"Unit clause will not be added, only propagated!\n";
#endif
#if ASSERT
			assert(value(lits_val[0])==U);
#endif
			enqueue(lits_val[0]);
		}else{
			watches[-clauses->at(0).val()].push_back(clauses);
			watches[-clauses->at(1).val()].push_back(clauses);
#if VERBOSE
			std::cout<<*clauses<<" )\n";
#endif	
		}

		clauses=clauses+1;
	}	
}

Solver::Solver(){};

Solver::~Solver(){
	watches=watches-nLiterals;
	delete [] watches;
	clauses=clauses-nClauses;
	delete [] clauses;
	reason=reason-nLiterals;
	delete [] reason;
	undos=undos -nLiterals;
	delete [] undos;
	activity=activity-nLiterals;
	delete [] activity;
	for(int i=0; i<learnts.size();i++) delete learnts[i];
};

Clause* Solver::propagate(){

	while(!propQ.empty()){
		Literal p = propQ.front();
		propQ.pop();
		bool not_conflict=true;
		int p_index=p.val();
		int size=watches[p_index].size();
		std::vector<Clause*> tmp=watches[p_index];
		clear(watches[p_index]);
		
		for(int i=0; i<size;++i){
			not_conflict=tmp[i]->propagate(this, &p);
			if(!not_conflict){
				int j=i;
				//clear propQ
				clear(propQ);
				//copy remaining watched literals
				for(i;i<size;++i) watches[p_index].push_back(tmp[i]);
				return tmp[j];
			}
		}
	}       
	return nullptr; 
}; 
	
	
	
bool Solver::enqueue(Literal p, Clause* c){ 
	if(value(p)!=U){ 
		if(value(p)==F){
#if VERBOSE
			std::cout<<"Trovato un conflitto generato da: "<<*c<<"\n";
#endif
			return false;
		}else{
#if VERBOSE
			std::cout<<"Existing consistent assignment, don't enqueue\n"<<"\t lit: "<<p<<", clause: "<<*c<<"\n";
#endif
			return true;
		}
	}else{
#if VERBOSE
		if(c!=nullptr)
			std::cout<<"Prop( "<<p<<" ) <- "<<*c<<"\n";
#endif
		if(p.sign())assignments[p.index()]=F;
		else assignments[p.index()]=T;
		levels[p.index()]=decisionLevel();
#if ASSERT
		if(c!=nullptr)
			assert(c->size()>0);
#endif
		if(c!=nullptr){
			reason[p.val()]=*c;
			undos[p.val()].push_back(c);
		}
		trail.push_back(p);
		propQ.push(p);
		return true;
	}
};

int Solver::analyze(Clause* conflict, std::vector<Literal>& to_learn){
#if ASSERT
	assert(decisionLevel()>0);
#endif
#if VERBOSE
	std::cout<<"Starting to analyze conflict "<<*conflict<<"...\n";
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
#if ASSERT
			assert(trail.size()>0);
#endif
			p = trail.back();
			c = reason[p.val()];
			undoOne();
		}while(!seen[p.index()]);
		counter--;
	}while(counter>0);
	to_learn[0]=~p;
	return btLevel;
};

void Solver::record(std::vector<Literal>& clause){
#if VERBOSE
	std::cout<<"Learning a new clause...\n";
#endif
	newClause(clause, true);
#if VERBOSE
	std::cout<<"Clause: "<<*learnts.back()<<" was learned!\n";
#endif
	enqueue(clause[0], learnts.back());
};

void Solver::undoOne(){
	Literal p = trail.back();
	assignments[p.index()]=U;
	Clause empty;*(reason+p.val())=empty;
	levels[p.index()]=-1;

	while(undos[p.val()].size()>0){
		undos[p.val()].back()->undo(this, p);
		undos[p.val()].pop_back();
	}
	trail.pop_back();
};

Literal Solver::select(){
	int max_so_far=-1;
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
	
	while(true){
		Clause* conflict=propagate();
		if(conflict!=nullptr){ //!< Exist a conflict
			std::vector<Literal> to_learn;int btLevel;
			if(decisionLevel()==root_level) return false;
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
			else if(nClauses==max_conflict) backtrack(root_level);
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

void Solver::simplify(){};

void Solver::reduceLearnts(){};

void Solver::cancel(){
	int c=trail.size()-trail_lim.back();
	for(;c!=0;c--) undoOne();
	trail_lim.pop_back();
};

void Solver::restart(){
#if VERBOSE
	std::cout<<"Restarting module...\n";
#endif
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
#if VERBOSE
	std::cout<<"Assume( "<<p<<" )\n";
#endif
	return enqueue(p);
};

void Solver::decayActivity(){
	int i;int MAX=nLiterals;const float dc_factor=DECAY_FACTOR;
	#pragma omp parallel for
	for(i=-MAX;i<MAX+1;++i){
		activity[i]=(activity[i]/dc_factor);
	}
};

bool Solver::isSAT(){
	for(int i=-nClauses;i<0;++i){
		bool sat=false;
		for(int j=0; j<clauses[i].size();++j){
			sat=sat||(value(clauses[i].at(j))==T?true:false);
		}
		if(!sat){
			std::cout<<"Clause "<<i<<" is not satisfied!\n";       
			return false;
		}
	}
	return true;
};

bool Solver::canBeSAT(){
	for(int i=-nClauses;i<0;++i){
		bool sat=false;
		for(int j=0; j<clauses[i].size();++j){
			if(value(clauses[i][j])==T||value(clauses[i][j])==U)
				sat=true;
		}
		if(!sat)return false;
	}
	return true;
};

void Solver::reward(Clause* c){
	for(int i=0; i<c->size();++i){
		activity[c->at(i).val()]+=BONUS;
	}
};
