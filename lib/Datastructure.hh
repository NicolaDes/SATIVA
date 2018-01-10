#ifndef DATASTRUCTURE_HH
#define DATASTRUCTURE_HH
#include "Config.h"
#include <cmath>
#include <vector>
#include <set>
#include <iostream>
#include <cassert>
#include <stdarg.h>


	typedef int var;
	typedef enum{F, T, U} mvlogic;

	class lbool{
	private:
		mvlogic x;
	public:
		lbool();
		lbool(mvlogic x){this->x=x;};
		mvlogic operator !(){if(x==T) return F; else if(x==F) return T; else return U;};
		bool operator ==(mvlogic logic){return (logic==x);};
		bool operator !=(mvlogic logic){return (logic!=x);};
		friend std::ostream & operator <<(std::ostream &os, const lbool &l){if(l.x==T)os<<"T";else if(l.x==F)os<<"F";else os<<"U";return os;};
	};

	class Literal{
	private:
		var x;
	public:
		Literal(){};
		Literal(var x){this->x=x;};

		//!< self operators		
		Literal operator ~(){Literal l(x*-1);return l;};
		Literal operator ~() const {Literal l(x*-1);return l;};

		friend bool operator ==(const Literal& l1, const Literal &l2){return l1.x==l2.x;};
		friend bool operator !=(const Literal& l1, const Literal &l2){return !(l1==l2);};

		friend std::ostream & operator <<(std::ostream &os, const Literal &l){if(l.x>0)os<<"x_"<<l.x;else os<<"-x_"<<std::abs(l.x);return os;};
		//!< Accessing operators
		bool operator <(const Literal& l) const{return x<l.x;};

		bool sign(){return (x>0)?false:true;};
		int val(){return x;};
		int val() const {return x;};
		int index(){return std::abs(x);};
		int index() const {return std::abs(x);};
	};	
	typedef std::vector<Literal> LiteralList;

	class Solver;
	/**
	 * Class wich represent a clause. It is implemented following the Chaff SAT solver data structure for watched literals. It uses 2 pointers in the clause wich are called 'watched literals'. Here comes the definition:
	 * C is a clause and (l1, l2) his watched literals.<br>
	 * <b>INVARIANT</b>: If C is neither confict neither satisfied by a propagated literal it has 2 wathced literals!<br>
	 * <i>If l1 -> FALSE:</i>
	 * <ul>
	 * <li>If l2 = TRUE, search another watched literal (l3) and (l2, l3) is the new couple watched literals, if l3 does not exist, do nothing</li>
	 * 	<li>If l2 = UNDEFINED, search another watched literal (l3) and (l2, l3) is the new couple watched litrals, if l3 does not exist, l2 is a propagated literal (so i have to add to propQ)</li>
	 * 	<li>If l2 = FALSE, search other 2 watched literals (l3, l4) and (l3, l4) are the new watched literals. If only one is found that is a propagation literal, if none is found C is a conflict clause.</li>
	 * </ul>
	 * First 2 places are for watched literals
	 */
	class Clause{
	friend class Solver;
	private:
		LiteralList literals;

	public:
		Clause(){};

		void addLiteral(Literal l){
			literals.push_back(l);
		};
		
		//!< self operators
		friend std::ostream & operator <<(std::ostream &os, const Clause &c){os<<("( ");for(auto x=c.literals.begin();x!=c.literals.end();){os<<*x;if(++x!=c.literals.end())os<<" v ";};os<<" )";return os;};

		/**
		 * Prototype function!!
		 */
		bool propagate(Solver *solver, Literal* p);
		Clause resolve(Clause c, Solver* solver);

		typedef LiteralList::iterator iterator;
//		typedef LiteralList::const_iterator const_iterator;
		iterator begin(){return literals.begin();};
		bool atLeastOne(const Clause c){

			for(int i=0;i<size();++i){
				for(int j=0;j<c.size();++j){
					if(literals[i]==c.literals[j]||literals[i]==~c.literals[j]) return true;
				}
			} 

			return false;
		};
		iterator end(){return literals.end();};
		Literal& at(int x){return literals[x];};
		Literal& operator [](int x){return literals[x];};
		bool operator ==(Clause c){
			if(c.size()!=size()) return false;
			int found=0;
			for(int i=0; i<size();++i){
				for(int j=0; j<c.size();++j){
					if(c[i]==this->at(j)) found++;
				}
			}
			return (found==c.size())?true:false;
		}
		bool operator !=(Clause c){return !(*this==c);};
		//TODO: da ottimizzare la ricerca e quindi la risoluzione.
		Clause operator &(Clause c){
			std::set<Literal> resolv_set(begin(), end());
			Clause resolv;
			for(int i=0; i<c.size();++i) resolv_set.insert(c[i]);
			for(std::set<Literal>::iterator it = resolv_set.begin(); it!=resolv_set.end();it++){
				if(resolv_set.find(~(*it))==resolv_set.end()){
					resolv.addLiteral(*it);
				}
			}
			return resolv;
		};
		int size() const {return literals.size();};
			
	};


#endif
