#include "Config.h"
#include "Datastructure.hh"
#include "Solver.hh"
#include "Reader.hh"
#include "CommandLineInterface.hh"
#include "Pigeonhole.hh"
#include <iostream>
#include <time.h>
//#include "System.hh"

struct PARAMS{
	char* filename=nullptr;
	int pigeonhole=-1;
}options;

void printUsage(char** argv);
void launchOnFile(Solver* solver);
void launchOnPig(Solver* solver);


int main(int argc, char** argv){
	

#if VERBOSE
	system("setterm -cursor off");
#endif
	for(int i=1; i<argc;++i){
		if(argv[i][0]=='-'){
			if(argv[i][1]=='f') options.filename=argv[i+1];
			else if(argv[i][1]=='p') options.pigeonhole=atoi(argv[i+1]);
		}
	}
	
	if(options.filename!=nullptr){
		Solver solver;
		launchOnFile(&solver);
	}else if(options.pigeonhole>0){
		Solver solver;
		launchOnPig(&solver);
	}else
		printUsage(argv);
#if VERBOSE
	system("setterm -cursor on");
#endif
	return 0;
}

void launchOnFile(Solver* solver){
	tabular::fillFromFile(options.filename, solver);
//	Dimacs::fillFromFile(options.filename, *solver);
	tabular::printInit(solver);
	tabular::printLaunch(solver);
	bool res=solver->CDCL();
	if(res) tabular::printModel(solver);
	else{
		tabular::generateGV(solver);
//		tabular::printProve(solver);       
	}
	tabular::printEnd();
};

void launchOnPig(Solver* solver){
	std::string name="pigeonhole ";
	name+=std::to_string(options.pigeonhole);
	solver->setName(name);

	pigeonhole::fillWithPigeonhole(solver, options.pigeonhole);

	tabular::printInit(solver);
	tabular::printLaunch(solver);
	bool res = solver->CDCL();
	if(res) tabular::printModel(solver);
	else{
		tabular::generateGV(solver);
//		tabular::printProve(solver);
	}
	tabular::printEnd();

};


void printUsage(char** argv){
	std::cout<<"\nUsage :\n";
	std::cout<<"\t"<<argv[0]<<" -f [filename] or -file [filename]\twill launch CDCL procedure on an input file.\n";
	std::cout<<"\t"<<argv[0]<<" -p [number] or -pigeonhole [number]\twill launch CDCL proceure on a pigeonhole instance with size [number].\n";
	std::cout<<"\n\n";
};
