#ifndef READER_HH
#define READER_HH
#include "Solver.hh"
#include "Datastructure.hh"
#include "Config.h"
#include <stdio.h>
#include <vector>
#include "omp.h"
#define EOL '\n'

namespace Dimacs{
	static void fillFromFile(char* file, Solver& solver){
		
		char c;
		int nL, nC;
		FILE *fp=fopen(file, "r");

		solver.setName(file);
		
		if(fp==nullptr){
			std::cerr<<"ERROR reading file.\n\tSystem aborting!!\n";
			exit(1);
		}
		bool intro=true;
		while(intro){
			int ret = fscanf(fp, "%c", &c);
			if(c=='c'){while(c!=EOL) ret=fscanf(fp, "%c", &c);}
			else if(c=='p'){
				while(c!='f') ret=fscanf(fp, "%c", &c);
				ret= fscanf(fp, "%i %i", &nL, &nC);
				intro=false;
			}
		}
		solver.init(nL, nC);
		int index=1;
		int val;
		int i=0;
		std::vector<Literal> lits_val;
#if VERBOSE
		float percentage=0;
#endif

		while(true){
#if VERBOSE
			percentage=(float)i/(float)nC;
			printf("[ Reading... ");
			printf("%3d %% ]\r",(int)((percentage)*100));
#endif

			int ret=fscanf(fp, "%i", &val);
			if(ret==EOF) break;
			if(val!=0){
				lits_val.push_back(Literal(val));
			}else if(lits_val.size()>0){
				solver.newClause(lits_val, false);
				index++;
				i++;
				lits_val.clear();
			}else break;
		}
		fclose(fp);
	};

};

#endif
