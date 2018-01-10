#include "Config.h"
#include "Datastructure.hh"
#include "Solver.hh"
#include "Reader.hh"
#include <iostream>
#include <time.h>

static inline double cpuTime(void){return (double)clock();}
typedef std::vector<Literal> LiteralList;
typedef std::vector<Clause> ClauseList;

int main(int argc, char** argv){
	

	Solver solver;

	if(argc>1) Dimacs::fillFromFile(argv[1], solver);
	std::cout<<"Ended to read!\n\n";

	double clk=cpuTime();
	if(solver.CDCL()){
#if ASSERT
		assert(solver.isSAT());
#endif
		std::cout<<"Ho trovato un assegnamento con la propagazione!!\n";
		std::vector<lbool> model=solver.getModel();
		for(int i=0; i<model.size();++i){if(model[i]!=U) std::cout<<"x_"<<i<<" <- "<<model[i]<<"; ";};
		std::cout<<"\n";
	}else{
		std::cout<<"La formula non è soddisfacibile!!\n";
		std::vector<lbool>model = solver.getModel();
		for(int i=0; i<model.size();++i){if(model[i]!=U) std::cout<<"x_"<<i<<" <- "<<model[i]<<"; ";};
		std::cout<<"\n";
	}
	printf("CPU time: %12.2f\n", clk);


	return 0;
}
