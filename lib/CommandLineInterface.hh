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
#include <fstream>

namespace tabular{
	using namespace std;
	static clock_t clk;
	static size_t mem;
	static struct rusage rus;
	void printInit(Solver* solver){
		cout<<"\n\n";
		int i=0;
		cout<<"|";
		for(;i<13;++i) cout<<"=";
		cout<<"< SAT-IVA STATS ";
		printf("%.1f", VERSION);
		cout<<" >";
		i=0;
		for(;i<13;++i) cout<<"=";
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

		cout<<"|===============================================|\n\n";
		cout<<"        Memory usage: ";
		cout<<system_util::memUsedPeak()<<" MB\n";
		cout<<"        CPU time: "<<((float)clk)/CLOCKS_PER_SEC<<" s\n";
		cout<<"        Learnt clauses: "<<solver->nLearnts()<<"\n";
		cout<<"        #Restarts: "<<solver->nRestart()<<"\n";
		cout<<"        Decisions: "<<solver->nDecision()<<"\n";
		cout<<"        Propagations: "<<solver->nPropagation()<<"\n";

	};
	//TODO: temporaneo, per il gdb
	void printLearnt(std::vector<Clause*>& v){
		for(int i=0; i<v.size();++i){
			std::cout<<*v[i]<<"\n";
		}
	};
	void printProve(Solver* solver){
		getrusage(RUSAGE_SELF, &rus);
		clk=clock()-clk;
		double mem_used=system_util::memUsedPeak();
		std::vector<lbool> model=solver->getModel();
		cout<<"|                     UNSAT                     |\n";
		cout<<"|-----------------------------------------------|\n";
#if PROVE
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
#endif
		cout<<"|===============================================|\n\n";
		cout<<"        Memory usage: ";
		cout<<system_util::memUsedPeak()<<" MB\n";
		cout<<"        CPU time: "<<((float)clk)/CLOCKS_PER_SEC<<" sec\n";
		cout<<"        Learnt clauses: "<<solver->nLearnts()<<"\n";
		cout<<"        #Restarts: "<<solver->nRestart()<<"\n";
		cout<<"        Decisions: "<<solver->nDecision()<<"\n";
		cout<<"        Propagations: "<<solver->nPropagation()<<"\n";
		cout<<"        Deleted clauses: "<<solver->getDeletedClause()<<"\n";
	};

	void generateGV(Solver* solver){
		clk=clock()-clk;
		double mem_used=system_util::memUsedPeak();
		cout<<"|                     UNSAT                     |\n";
		cout<<"|-----------------------------------------------|\n";
#if PROVE
		cout<<"|------------< Generating prove... >------------|\n";
		ofstream fout;fout.open("graphics/prove.gv");
		fout<<"digraph G {\n";
		std::vector<std::vector<Clause> >prove = solver->getProve();
		for(int i=0; i<prove.size();++i){
			for(int j=0;j<prove[i].size()-2;++j){
				if(prove[i].size()<2){
					fout<<"\""<<prove[i][j]<<"\";\n";
					break;
				}
				fout<<"\""<<prove[i][j]<<"\" -> \""<<prove[i][j+2]<<"\";\n\""<<prove[i][j+1]<<"\" -> \""<<prove[i][j+2]<<"\";\n";
				j++;
			}
		}
		fout<<"}\n";
		fout.close();
#endif
		cout<<"|===============================================|\n\n";
		cout<<"        Memory usage: ";
		cout<<system_util::memUsedPeak()<<" MB\n";
		cout<<"        CPU time: "<<((float)clk)/CLOCKS_PER_SEC<<" sec\n";
		cout<<"        Learnt clauses: "<<solver->nLearnts()<<"\n";
		cout<<"        Conflict clauses: "<<solver->nConflict()<<"\n";
		cout<<"        #Restarts: "<<solver->nRestart()<<"\n";
		cout<<"        Decisions: "<<solver->nDecision()<<"\n";
		cout<<"        Propagations: "<<solver->nPropagation()<<"\n";
		cout<<"        Deleted clauses: "<<solver->getDeletedClause()<<"\n";

	};

	void printEnd(){
		cout<<"\n";
		cout<<"|><>><>><>><>><>><>><>><>><>><>><>><>><>><>><>>>|\n";
		cout<<"\n\n";
	};

	void generateCSV(Solver* solver, bool sat){
		clk=clock()-clk;
		ofstream fout;
		fout.open ("test.csv", std::ofstream::out | std::ofstream::app);
		fout<<solver->pName()<<","<<solver->nL()<<","<<solver->nC()<<",=("<<clk<<")/"<<CLOCKS_PER_SEC<<","<<solver->nConflict()<<","<<solver->nLearnts()<<","<<solver->nDecision()<<","<<solver->nPropagation()<<","<<((sat)?"SAT":"UNSAT");
		fout<<",";
		fout<<system_util::memUsedPeak()<<"\n";
		fout.close();
	};
};

#endif
