#ifndef SOLVER_HH
#define SOLVER_HH
#include "Config.h"
#include "Datastructure.hh"
#include <queue>
#include "omp.h"
#include <cassert>

class Solver{
	public:
		friend class Clause;
		Solver();
		~Solver();

		/**
		 * Init the solver allocating the necessary memory on the heap
		 */
		void init(int nL, int nC);

		/**
		 * Create a new clause in the heap of the program!
		 */
		void newClause(std::vector<int>& lits_val);

		/**
		 * Compute the CDCL procedure
		 */
		bool CDCL();

		/**
		 * Return the number of clauses
		 */
		int nC(){return nClauses;};
		/**
		 * Return the number of literals
		 */
		int nL(){return nLiterals;};
		/**
		 * Return the number of learnts clauses
		 */
		int nLearnts(){return learnts.size();};
		/**
		 * Return the current assignment for variable x
		 */
		lbool value(var x){return assignments[x];};
		/**
		 * Return the sobstitution of assignment to literal l
		 */
		lbool value(Literal l){return (l.sign())?not(assignments[l.index()]):assignments[l.index()];};
		/**
		 * Return the current decision level
		 */
		int decisionLevel(){return curr_level;};
		/**
		 * Propagate operation
		 * This method support pragma openmp operation to parallelize into nThread the propagation for each clause. The number of thread depends on CPU cores.
		 */
		bool propagate();
		/**
		 * Return the model in wich SAT is satisfied or not
		 */
		std::vector<lbool> getModel(){return assignments;};

		/**
		 * Pick a variable
		 */
		void pickALiteral();

		/**
		 * Method used to analyze the conflict
		 */
		int analyze();

		/**
		 * Method used to backtrack
		 */
		void backtrack(int btLevel);
		/**
		 * Method to restart
		 */
		void restart();
		
		/**
		 * Enqueue method to check if exist a conflict and if not insert into propagation queue next propagate literal, agiving asignement etc..
		 */
		bool enqueue(Literal p, Clause* c);

		/**
		 * Method to decay all the activities
		 */
		void decayActivity();
	private:
		bool initialized=false;
		int nClauses;
		int nLiterals;
		int curr_level;
		//!< Default constraints
		Clause* clauses; //!< List of clauses.
		std::vector<Clause*> learnts; //!< List of learnt clauses.

		//!< Propagation constraints
		std::vector<Clause*>*  watched_list; //!< For each literal with a positive phase a list of clause to be watched if p changes value.
		Clause* antecedents;
		std::queue<Literal> propQ;

		//!< Conflict clause
		Clause conflict;

		//!< Assignment values
		std::vector<lbool> assignments; //!< Assignment indexed by variable. Size of this is the number of vars.
		std::vector<Literal> trace; //!< List of assignment in chronological order.
		std::vector<int> levels; //!< For each variable the level it belongs to.

		//!< Decay activities
		float* activity;

		//private methods
		/**
		 * Here is define when the resolution analysis of conflict must end and the procedure can go on learn conflict.
		 */
		bool stop_criterion();
		/**
		 * Method wich check if all variables are signed!
		 */
		bool isAllSigned();

		/**
		 * Method to find 1UIP in implication graph
		 * @warning This method modifies the conflict!!
		 */
		void firstUIP();
};

//functions prototypes
	/**
	 * This method is executed in parallel!! Stay tuned!<br>
	 * Watched literals:<br>
	 * C is a clause and (l1, l2) his watched literals.<br>
	 * <b>INVARIANT</b>: If C is neither confict neither satisfied by a propagated literal it has 2 wathced literals!<br>
	 * <i>If l1 -> FALSE:</i>
	 * <ul>
	 * <li>If l2 = TRUE, search another watched literal (l3) and (l2, l3) is the new couple watched literals, if l3 does not exist, do nothing</li>
	 * 	<li>If l2 = UNDEFINED, search another watched literal (l3) and (l2, l3) is the new couple watched litrals, if l3 does not exist, l2 is a propagated literal (so i have to add to propQ)</li>
	 * 	<li>If l2 = FALSE, search other 2 watched literals (l3, l4) and (l3, l4) are the new watched literals. If only one is found that is a propagation literal, if none is found C is a conflict clause.</li>
	 * </ul>
	 */
	inline bool Clause::propagate(Solver *solver, Literal* p){
		//!< Implementation design: work only on one watched literal, so it's necessary to swap the two literals if it is not the working literal decided. W1 is the working literal (literals[0])
		if(literals[0]==~*p){
			literals[0]=literals[1];
			literals[1]=~*p;
		}
		if(solver->value(literals[0])==T){
			for(int i=2;i<size();++i){
				if(solver->value(literals[i])==U){
					Literal tmp=literals[0];
					literals[0]==literals[i];
					literals[i]=tmp;
					break;
				}
			}
			solver->watched_list[-literals[0].val()].push_back(this);
			return true;
		}
		//!< here i have a unit propagation or a conflict. I search for an other literal and if found i give to w2 and the propagation or conflict is delegate to enqueue method.
		for(int i=2;i<size();++i){
			if(solver->value(literals[i])==U){
				literals[1]=literals[i];literals[i]=~*p;
				solver->watched_list[-literals[1].val()].push_back(this);
				return true;
			}
		}

		//!< Here i have either a propagation unit or a conflict
		solver->watched_list[-literals[1].val()].push_back(this);
		return solver->enqueue(literals[0], this);
	};

	inline Clause Clause::resolve(Clause c, Solver* solver){
		std::set<Literal> resolv_set(begin(), end());
		Clause resolv;
		for(int i=0; i<c.size();++i) resolv_set.insert(c[i]);
		for(std::set<Literal>::iterator it = resolv_set.begin(); it!=resolv_set.end();it++){
			if(resolv_set.find(~(*it))==resolv_set.end()){
				resolv.addLiteral(*it);
				solver->activity[it->val()]+=BONUS;
			}
		}
		return resolv;
	};
#endif
