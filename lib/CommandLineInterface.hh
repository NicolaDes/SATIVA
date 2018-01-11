#ifndef COMMANDLINEINTERFACE_HH
#define COMMANDLINEINTERFACE_HH
#include <iostream>
#include "Config.h"
#include "System.hh"

namespace tabular{
	using namespace std;
	void printInit(Solver* solver){
	//	double cpu_time=system_util::cpuTime();
	//	double mem_used=system_util::memUsedPeak();
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
		cout<<"| #Clauses ";
		i=9;
		int _size=solver->nC();
		while((_size/=10)>0)i++;
		cout<<solver->nC();
		for(;i>0;i--)cout<<" ";
		cout<<"|\n";
		cout<<"|";
	};

	void printLaunch(Solver* solver){
	//	double cpu_time=system_util::cpuTime();
	//	double mem_used=system_util::memUsedPeak();
		int i=0;
		cout<<"|";
		for(;i<15;++i) cout<<"=";
		cout<<"< SAT-IVA STATS >";
		i=0;
		for(;i<15;++i) cout<<"=";
		cout<<"|\n";
		i=0;
		cout<<"|";
		for(;i<13;++i) cout<<"-";
		cout<<" Launching CDCL procedure ";
		for(;i<21;++i) cout<<"-";
		cout<<"|\n";
		//print clauses
		cout<<"| clauses ";
		i=9;
		int _size=solver->nC();
		while((_size/=10)>0)i++;
		cout<<solver->nC();
		for(;i>0;i--)cout<<" ";
		cout<<"|\n";
		cout<<"|";
	};
	void clear(){
		for(int i=0;i<100;++i)cout<<"\n";
	};
};

#endif
