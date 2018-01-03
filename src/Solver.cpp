#include "Solver.hh"

void Solver::init(int nL, int nC){
	int size=nL+1;
	watched_list = new std::vector<Clause*>[size*2];
	watched_list = watched_list+size;
	antecedents = new Clause[size];
	assignments.resize(size, U);
	levels.resize(size, 0);
	clauses=new Clause[nC];
	nLiterals=nL;nClauses=nC;
	curr_level=0;
	activity = new float[2*size];	
	for(int i=0; i<size;++i){
		*activity=0;
		activity=activity+1;
	}
	int i;
	#pragma omp parallel for private(i)
	for(i=0; i<size;++i) activity[i]=0;

	initialized=true;
}

void Solver::newClause(std::vector<int>& lits_val){
	if(!initialized){std::cerr<<"Solver not initialized!!\n\tSystem aborting...\n";exit(1);}
	int size=lits_val.size();
	assert(size>0);
#if VERBOSE
	std::cout<<"newClause( ";
#endif
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
		watched_list[-clauses->at(0).val()].push_back(clauses);
		watched_list[-clauses->at(1).val()].push_back(clauses);
	}
	clauses=clauses+1;
	
}

Solver::Solver(){};

Solver::~Solver(){
	clauses=clauses-nClauses;
	delete [] clauses;
	watched_list = watched_list - (nLiterals+1);
	delete [] watched_list;
	delete [] antecedents;
	activity=activity-(nLiterals+1);
	delete [] activity;
	for(int i=0; i<learnts.size();i++) delete learnts[i];
};

bool Solver::propagate(){
	while(!propQ.empty()){
		Literal p = propQ.front();
		propQ.pop();
		bool not_conflict=true;
		int p_index=p.val();
		std::vector<Clause*> c_str=watched_list[p_index];
		std::vector<Clause*> v_empty;
		std::swap(watched_list[p_index], v_empty);
		for(int i=0; i<c_str.size();++i){
			not_conflict=c_str[i]->propagate(this, &p);
			if(!not_conflict){
				//clear propQ
				std::queue<Literal> q_empty;
				std::swap(propQ, q_empty);
				//copy remaining watched literals
				for(i;i<c_str.size();++i) watched_list[p_index].push_back(c_str[i]);
				return false;
			}
		}
	}       
	return true; 
}; 
	
	
	
bool Solver::enqueue(Literal p, Clause* c){ 
	if(value(p)!=U){ 
		if(value(p)==F){ 
#if VERBOSE 
			std::cout<<"############ CONFLICT ############\n "<<*c<<"\n##################################\n"; 
#endif
			conflict=*c;
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
		antecedents[p.index()]=*c;
		trace.push_back(p);
		propQ.push(p);
		return true;
	}
};

void Solver::firstUIP(){
	int trace_size=trace.size()-1;
	Clause ref=conflict;
	while(!stop_criterion()&&trace_size>0){
		if(antecedents[trace[trace_size].index()].size()>0&&ref.atLeastOne(antecedents[trace[trace_size].index()])) 
			conflict=conflict.resolve(antecedents[trace[trace_size].index()], this);
		trace_size--;
	}
};

int Solver::analyze(){
	firstUIP();
#if VERBOSE
	std::cout<<"Conflict resolved into: "<<conflict<<"\n";
#endif
	learnts.push_back(new Clause);
	*learnts[learnts.size()-1]=conflict;


#if VERBOSE
	std::cout<<"\nSearching for NBC...\n";
#endif
	int max;
	if(conflict.size()>0){
		max=0;
		int index=0;
		for(int i=0; i<conflict.size();++i){
			if(levels[conflict[i].index()]>max&&levels[conflict[i].index()]!=curr_level){
				max=levels[conflict[i].index()];
				index=i;
			}
		}
	}else max=0; //!< Unitary resolved implies backjump to 0 level.
	assert(max>=0);
#if VERBOSE
	std::cout<<"Found the 1UIP level: "<<max<<"\n";
#endif
	return 0;
};


bool Solver::stop_criterion(){
	int n_setted=0;
	for(int i=0; i<conflict.size();++i){
		if(levels[conflict[i].index()]==curr_level) n_setted++;
	}
	return (n_setted>1)?false:true;
};

bool Solver::CDCL(){

	while(true){
		bool not_conflict=propagate();
		if(not_conflict){
			if(isAllSigned()) return true;
			else pickALiteral();

		}else{
			int bt_level = analyze();
			if(curr_level==0) return false;
			else backtrack(bt_level);
		}
		decayActivity();
	};
};

void Solver::backtrack(int btLevel){

	int MAX=trace.size()-1;
	for(int i=MAX;i>=0;i--){
		Literal curr=trace.back();trace.pop_back();
		if(levels[curr.index()]==btLevel) break;
		//undo assignment
		assignments[curr.index()]=U;
		//undo antecedent
		if(antecedents[curr.index()].size()>0){
			Clause empty;
			antecedents[curr.index()]=empty;
		}
		//undo level
		levels[curr.index()]=0;
	}
	curr_level=btLevel;

	if(learnts.back()->size()==1){
		enqueue(learnts.back()->at(0), learnts.back());
	}else{
		watched_list[-learnts.back()->at(0).val()].push_back(learnts.back());
		watched_list[-learnts.back()->at(1).val()].push_back(learnts.back());
	}

};

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
		if(assignments[std::abs(i)]==U&&activity[i]>max){
			max=activity[index];
			index=i;
		}
	}
	Literal l(index);
#if VERBOSE
	std::cout<<"decide("<<l<<")\n";
#endif
	trace.push_back(l);
	propQ.push(l);
	assignments[l.index()]=T;
	curr_level++;
	levels[l.index()]=curr_level;
};

void Solver::decayActivity(){
	int i;int MAX=nLiterals+1;const float dc_factor=DECAY_FACTOR;
	#pragma omp parallel for
	for(i=-MAX;i<MAX;++i) activity[i]=(activity[i]/dc_factor);
};
