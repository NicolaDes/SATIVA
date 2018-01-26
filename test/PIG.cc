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

	std::string name="pigeonhole ";
	name+=argv[1];
	solver.setName(name);

	int holes = atoi(argv[1]);

	pigeonhole::fillWithPigeonhole(&solver, holes);

	tabular::printInit(&solver);
	tabular::printLaunch(&solver);
	bool res = solver.CDCL();
	if(res) tabular::printModel(&solver);
	else{
		tabular::generateGV(&solver);
	}
	tabular::printEnd();



	return 0;
}
