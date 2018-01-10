#include "Solver.hh"

void Solver::init(int nL, int nC){
	watches= new std::vector<Clause*>[(2*nL)+1];
	watches=watches+nL;
	reason= new Clause[(2*nL)+1];
	reason=reason+nL;
	assignments.resize(nL+1, lbool(U));
	levels.resize(nL+1, 0);
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

	undos.resize(nL+1);

	initialized=true;
}

void Solver::newClause(std::vector<int>& lits_val, bool learnt){
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
	}else{
		for(auto x : lits_val){
			Literal l(x);
			clauses->addLiteral(l);
			activity[l.val()]++;
		}
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

Solver::Solver(){};

Solver::~Solver(){
	watches=watches-nLiterals;
	delete [] watches;
	clauses=clauses-nClauses;
	delete [] clauses;
	reason=reason-nLiterals;
	delete [] reason;
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
				//clear propQ
				clear(propQ);
				//copy remaining watched literals
				for(i;i<size;++i) watches[p_index].push_back(tmp[i]);
				return tmp[i];
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
		return true;
	}
};

int Solver::analyze(Clause* conflict){

};

bool Solver::stop_criterion(){

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
			else pickALiteral();

		}else{
			int bt_level = analyze(conflict);
			if(curr_level==0) return false;
			else backtrack(bt_level);
		}
		decayActivity();
	};
};

void Solver::backtrack(int btLevel){

};

void Solver::undo(Literal l){
	
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

void Solver::pickALiteral(){
	int max=-1;int index=0;
	for(int i=-nLiterals; i<nLiterals+1;++i){
		if(i==0||i==-0) continue;
		if(assignments[i]==U&&activity[i]>max){
			max=activity[index];
			index=i;
		}
	}
	activity[index]=0;	
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
