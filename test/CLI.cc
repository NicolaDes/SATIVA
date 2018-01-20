#include "Config.h"
#include "Datastructure.hh"
#include "Solver.hh"
#include "Reader.hh"
#include "CommandLineInterface.hh"
#include <iostream>
#include <time.h>

typedef std::vector<Literal> LiteralList;
typedef std::vector<Clause> ClauseList;

int main(int argc, char** argv){
	

	Solver solver;

	if(argc>1) Dimacs::fillFromFile(argv[1], solver);

	tabular::printInit(&solver);
	tabular::printLaunch(&solver);
	bool res = solver.CDCL();
//	tabular::generateCSV(&solver,res);

	if(res) tabular::printModel(&solver);
	else{
		tabular::generateGV(&solver);
	}
	tabular::printEnd();


	return 0;
}
