#ifndef PIGEONHOLE_HH
#define PIGEONHOLE_HH
#include "Datastructure.hh"
#include <vector>

namespace pigeonhole{

void fillWithPigeonhole(Solver* solver, int holes){
#if VERBOSE
	float percentage=0;
#endif
	int n=holes;  // n+1 pigeons in n holes
	
	solver->init((n+1)*n, (n+1)+n*(n*(n+1)/2));
	std::vector<Literal> tmp;

	int i, k;  // pigeons i, k
	int j;     // hole j

  // n+1 clauses which say that a pigeon has to be placed in some hole
	for (i=1; i <= n+1; i++) {
#if VERBOSE
		percentage=(float)i/(float)n+1;
		printf("[ generating... ");
		printf("%3d %% ]\r",(int)((percentage)*100));
#endif

		for (j=1; j <= n; j++)
			tmp.push_back(Literal(n*(i-1)+j));
		solver->newClause(tmp);
		tmp.clear();
	}

  // for each hole we have a set of clauses ensuring that only one single
  // pigeon is placed into that hole
	for (j=1; j <= n; j++){
#if VERBOSE
		percentage=(float)j/(float)n;
		printf("[ generating... ");
		printf("%3d %% ]\r",(int)((percentage)*100));
#endif

		for (i=1; i <= n; i++){
			for (k=i+1; k <= n+1; k++){
				tmp.push_back(Literal(-(n*(i-1)+j)));tmp.push_back(-(n*(k-1)+j));
				solver->newClause(tmp, false);
				tmp.clear();
			}
		}
	}

};
}

#endif
