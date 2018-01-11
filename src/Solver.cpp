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
	curr_level=0;
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
			Literal l(x);
			learnts[learnt_size]->addLiteral(l);
			activity[l.val()]++;
		}
		std::cout<<*learnts.back()<<" )\n";
		reward(learnts.back());
		return;
	}else{
		for(auto x : lits_val){
			Literal l(x);
			clauses->addLiteral(l);
			activity[l.val()]++;
		}
#if VERBOSE
		std::cout<<*clauses<<" )\n";
#endif	
		if(size==1){
			enqueue(clauses->at(0), clauses);
		}else{
			watches[-clauses->at(0).val()].push_back(clauses);
			watches[-clauses->at(1).val()].push_back(clauses);
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
		std::cout<<"Prop( "<<p<<" ) <- "<<*c<<"\n";
#endif
		if(p.sign())assignments[p.index()]=F;
		else assignments[p.index()]=T;
		levels[p.index()]=curr_level;
		assert(c->size()>0);
		reason[p.val()]=*c;
		trail.push_back(p);
		propQ.push(p);
		undos[p.val()].push_back(c);
		return true;
	}
};

int Solver::analyze(Clause* conflict, std::vector<Literal>& to_learn){
#if ASSERT
	assert(curr_level>0);
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
				if(curr_level==levels[q.index()]) counter++; //!< q need to be processed
				else if(curr_level>0){ //!< exluding curr_level = 0
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

bool Solver::CDCL(){

	while(true){
		Clause* conflict=propagate();
		if(conflict==nullptr){
#if ASSERT
			for(int i=-nClauses;i<0;++i){
				bool sat=false;
				for(int j=0; j<clauses[i].size();++j){
					if(value(clauses[i][j])==T||value(clauses[i][j])==U)
						sat=true;
				}
			assert(sat);
			}
#endif
			if(isAllSigned()) return true;
			else assume();

		}else{
			if(curr_level==0)
				return false;
			std::vector<Literal> to_learn;
			int bt_level = analyze(conflict, to_learn);
			record(to_learn);
			if(curr_level==0) return false;
			else backtrack(bt_level);
		}
		decayActivity();
	};
};

void Solver::backtrack(int btLevel){
	while(curr_level>btLevel) cancel();
};

void Solver::cancel(){
	Literal p=trail.back();
	while(reason[p.val()].size()==0){
		undoOne();	
		p=trail.back();
	}
	trail.pop_back(); //!< delete decision
	curr_level--;
};

void Solver::restart(){
#if VERBOSE
	std::cout<<"Restarting module...\n";
#endif
	backtrack(0);
}

bool Solver::isAllSigned(){
	int i;int MAX=nLiterals+1;bool yes=true;
	#pragma omp parallel for private(i, MAX) 
	for(i=1;i<MAX;i++){
		if(assignments[i]!=U)yes=yes&&true;
		else yes=yes&&false;
	}
	return yes;
};

bool Solver::assume(){
	int max=-1;int index=0;
	for(int i=-nLiterals; i<nLiterals+1;++i){
		if(i==0||i==-0) continue;
		if(assignments[i]==U&&activity[i]>max){
			max=activity[index];
			index=i;
		}
	}
	Literal l(index);
#if VERBOSE
	std::cout<<"decide("<<l<<")\n";
#endif
	trail.push_back(l);
	propQ.push(l);
	
	if(index>0) assignments[l.index()]=T;
	else assignments[l.index()]=F;
	
	curr_level++;
	levels[l.index()]=curr_level;
	return true;
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

void Solver::reward(Clause* c){
	for(int i=0; i<c->size();++i){
		activity[c->at(i).val()]+=BONUS;
	}
};
