#ifndef COMMANDLINEINTERFACE_HH
#define COMMANDLINEINTERFACE_HH
#include <iostream>
#include "Config.h"
#include "System.hh"
#include "Datastructure.hh"
#include "Solver.hh"
#include <vector>
#include <ctime>
#include<sys/resource.h>

namespace tabular{
	using namespace std;
	static clock_t clk;
	static size_t mem;
	static struct rusage rus;
	void printInit(Solver* solver){
		cout<<"\n\n";
		int i=0;
		cout<<"|";
		for(;i<15;++i) cout<<"=";
		cout<<"< SAT-IVA STATS >";
		i=0;
		for(;i<15;++i) cout<<"=";
		cout<<"|\n";
		i=0;
		cout<<"|";
		for(;i<16;++i) cout<<"-";
		cout<<" Problem data ";
		for(;i<33;++i) cout<<"-";
		cout<<"|\n";
		//print clauses
		cout<<"|\t#Clauses ";
		i=9;
		cout<<"\t"<<solver->nC();
		cout<<"\t\t\t";
		cout<<"|\n";
		cout<<"|\t#Literals ";
		cout<<"\t"<<solver->nL();
		cout<<"\t\t\t";
		cout<<"|\n";
		cout<<"|\tHeuristics: VSIDS, 1UIP";
		cout<<"\t\t\t";
		cout<<"|\n";
		cout<<"|-----------------------------------------------|\n";
	};

	void printLaunch(Solver* solver){
		cout<<"|--------- Launching CDCL procedure ------------|\n";
		clk=clock();
		getrusage(RUSAGE_SELF,&rus);
	};
	void clear(){
		for(int i=0;i<100;++i)cout<<"\n";
	};
	void printModel(Solver* solver){
		getrusage(RUSAGE_SELF, &rus);
		clk=clock()-clk;
		std::vector<lbool> model=solver->getModel();
		cout<<"|                     SAT                       |\n";

		cout<<"|-----------------------------------------------|\n";
		cout<<"|-------------------< MODEL >-------------------|\n";
		cout<<"{ ";
		for(int i=0; i<model.size();++i){if(model[i]!=U) std::cout<<((model[i]==T)?"x_":"-x_")<<i<<"; ";};
		cout<<"}\n";

		cout<<"|-----------------------------------------------|\n";
		cout<<"|        Memory usage: ";
		cout<<system_util::memUsedPeak()<<" MB\t\t\t|\n";
		cout<<"|        CPU time: "<<((float)clk)/CLOCKS_PER_SEC<<" s\t\t\t|\n";
		cout<<"|        Learnt clauses: "<<solver->nLearnts()<<"\t\t\t|\n";
		cout<<"|        Conflict clauses: "<<solver->nConflict()<<"\t\t\t|\n";
		cout<<"|        Decisions: "<<solver->nDecision()<<"\t\t\t\t|\n";
		cout<<"|        Propagations: "<<solver->nPropagation()<<"\t\t\t|\n";

	};
	void printProve(Solver* solver){
		getrusage(RUSAGE_SELF, &rus);
		clk=clock()-clk;
		double mem_used=system_util::memUsedPeak();
		std::vector<lbool> model=solver->getModel();
		cout<<"|                     UNSAT                     |\n";
		cout<<"|-----------------------------------------------|\n";
		cout<<"|-------------------< PROVE >-------------------|\n";
		std::vector<std::vector<Clause> > prove = solver->getProve();
		for(int i = 0; i<prove.size();++i){
			int level=0;
			for(auto x = prove[i].begin();x!=prove[i].end();++x){
				cout<<"|";
				for(int j = (level%2==0)?level:level-1; j>0; j--) cout<<"----";
				cout<<*x<<"\n";
				level++;
			}
		}
		cout<<"|-----------------------------------------------|\n";
		cout<<"|        Memory usage: ";
		cout<<system_util::memUsedPeak()<<" MB\t\t\t|\n";
		cout<<"|        CPU time: "<<((float)clk)/CLOCKS_PER_SEC<<" s\t\t\t|\n";
		cout<<"|        Learnt clauses: "<<solver->nLearnts()<<"\t\t\t|\n";
		cout<<"|        Conflict clauses: "<<solver->nConflict()<<"\t\t\t|\n";
		cout<<"|        Decisions: "<<solver->nDecision()<<"\t\t\t\t|\n";
		cout<<"|        Propagations: "<<solver->nPropagation()<<"\t\t\t|\n";
	};

	void printEnd(){
		cout<<"|-----------------------------------------------|\n";
		cout<<"\n\n";
	};
};

#endif
