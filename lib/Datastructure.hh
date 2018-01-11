#ifndef DATASTRUCTURE_HH
#define DATASTRUCTURE_HH
#include "Config.h"
#include <cmath>
#include <vector>
#include <set>
#include <iostream>
#if ASSERT
	#include <cassert>
#endif
#include <stdarg.h>


	typedef int var;
	typedef enum{F, T, U} mvlogic;

	class lbool{
	private:
		mvlogic x;
	public:
		//!< Constructors
		lbool();
		lbool(mvlogic x){this->x=x;};
		//!< Operators
		mvlogic operator !(){if(x==T) return F; else if(x==F) return T; else return U;};
		bool operator ==(mvlogic logic){return (logic==x);};
		bool operator !=(mvlogic logic){return (logic!=x);};
		friend std::ostream & operator <<(std::ostream &os, const lbool &l){if(l.x==T)os<<"T";else if(l.x==F)os<<"F";else os<<"U";return os;};
	};

	class Literal{
	private:
		var x;
	public:
		//!< Constructors
		Literal(){};
		Literal(var x){this->x=x;};

		//!< Operators
		Literal operator ~() const {Literal l(x*-1);return l;};
		friend bool operator ==(const Literal& l1, const Literal &l2){return l1.x==l2.x;};
		friend bool operator !=(const Literal& l1, const Literal &l2){return !(l1==l2);};

		friend std::ostream & operator <<(std::ostream &os, const Literal &l){if(l.x>0)os<<"x_"<<l.x;else os<<"-x_"<<std::abs(l.x);return os;};
		bool operator <(const Literal& l) const{return x<l.x;};

		/**
		 * Return true if is negate, false otherwise
		 */
		bool sign(){return (x>0)?false:true;};
		/**
		 * Return the value of Literal
		 */
		int val() const {return x;};
		/**
		 * Return an indexable value for array of Literal
		 */
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

		/**
		 * Add a literal to the clause.
		 */
		void addLiteral(Literal l){
			literals.push_back(l);
		};
		

		/**
		 * @warning Prototype function
		 * Propagate the clause.
		 */
		bool propagate(Solver *solver, Literal* p);
		/**
		 * Calculate the reason of the conflict clause
		 * @warning Prototype function
		 */
		void calcReason(Solver* solver, Literal& p, std::vector<Literal>& p_reason);	
		/**
		 * Undo last assignment restoring original status of the clause
		 * @warning Prototype function
		 */
		void undo(Solver* solver, Literal& p);

		/**
		 * Return the object literal at position :x
		 */
		Literal& at(int x){return literals[x];};

		//!< Operators
		/**
		 * Return the ogject literal at position :x
		 */
		Literal& operator [](int x){return literals[x];};
		friend std::ostream & operator <<(std::ostream &os, const Clause &c){os<<("( ");for(auto x=c.literals.begin();x!=c.literals.end();){os<<*x;if(++x!=c.literals.end())os<<" v ";};os<<" )";return os;};
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
		int size() const {return literals.size();};
			
	};


#endif
