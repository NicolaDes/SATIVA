#include "Config.h"
#include "Datastructure.hh"
#include "Solver.hh"
#include "Reader.hh"
#include <iostream>

typedef std::vector<Literal> LiteralList;
typedef std::vector<Clause> ClauseList;

int main(int argc, char** argv){
	

	Solver solver;

	if(argc>1) Dimacs::fillFromFile(argv[1], solver);
	std::cout<<"Ended to read!\n";

	if(solver.CDCL()){
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


	return 0;
}
