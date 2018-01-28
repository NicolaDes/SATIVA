#include "Config.h"
#include "Datastructure.hh"
#include "Solver.hh"
#include "Reader.hh"
#include "Pigeonhole.hh"
#include "CommandLineInterface.hh"
#include <iostream>
#include <time.h>

typedef std::vector<Literal> LiteralList;
typedef std::vector<Clause> ClauseList;

int main(int argc, char** argv){
	

	Solver solver;


	std::string name="PIG ";
	name+=argv[1];
	solver.setName(name);	
	int holes=atoi(argv[1]);
//	pigeonhole::fillWithPigeonhole(solver, holes);
	if(argc>1) tabular::fillFromFile(argv[1], &solver);

	tabular::printInit(&solver);
	tabular::printLaunch(&solver);
	bool res = solver.CDCL();
	tabular::generateCSV(&solver,res);

	return 0;
}
