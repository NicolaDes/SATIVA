#ifndef SOLVER_HH
#define SOLVER_HH
#include "Config.h"
#include "Datastructure.hh"
#include <queue>
#include <list>
#include "omp.h"
#include <cassert>

class Solver{
	public:
		friend class Clause;
		Solver();
		~Solver();

		/**
		 * Init the solver allocating the necessary memory on the heap
		 * @warning To use the solver, this method is foundamental
		 */
		void init(int nL, int nC);

		/**
		 * Create a new clause in the heap of the program!
		 */
		void newClause(std::vector<Literal>& lits_val, bool learnt);
		/**
		 * Check if SAT is SAT under this assignment
		 */
		bool isSAT();
		/**
		 * Compute the CDCL procedure
		 */
		bool CDCL();
		
		/**
		 * Return the model in wich SAT is satisfied or not
		 */
		std::vector<lbool> getModel(){return assignments;};

		/**
		 * Return the prove of unsat
		 */
		inline std::vector<std::vector<Clause> >& getProve(){return resolvents;};

		/**
		 * Return the number of conflicts
		 */
		inline int nConflict(){return nConflicts;};
		/**
		 * Return the number of decisions
		 */
		inline int nDecision(){return nDecisions;};
		/**
		 * Return the number of propagations
		 */
		inline int nPropagation(){return nPropagations;};

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
		
	private:
		//!< Constants values
		const int root_level=0;
		int max_conflict;
		int nConflicts=0;
		int nPropagations=0;
		int nDecisions=0;

		//!< Structure to get the counterexample
		std::vector<std::vector<Clause> > resolvents;

		bool initialized=false;
		int nClauses;
		int nLiterals;
		//!< Default constraints
		Clause* clauses; //!< List of clauses.
		std::vector<Clause*> learnts; //!< List of learnt clauses.

		//!< Propagation constraints
		std::vector<Clause*>*  watches; //!< For each literal with a positive phase a list of clause to be watched if p changes value.
		Clause* reason;
		std::queue<Literal> propQ;
		std::vector<Clause*>* undos;

		//!< Assignment values
		std::vector<lbool> assignments; //!< Assignment indexed by variable. Size of this is the number of vars.
		std::vector<Literal> trail; //!< List of assignment in chronological order.
		std::vector<int> trail_lim;
		std::vector<int> levels; //!< For each variable the level it belongs to.

		template<class T>
		inline void clear(std::vector<T>& origin){
			std::vector<T> empty;
			std::swap(origin, empty);
		};
		template<class T>
		inline void clear(std::queue<T>& origin){
			std::queue<T> empty;
			std::swap(origin, empty);
		};
		
		/**
		 * Check if is possible to be SAT
		 */
		bool canBeSAT();

		/**
		 * Select new variable
		 */
		Literal select();

		/**
		 * Simplify the original set of variables
		 */
		void simplify(); //TODO

		/**
		 * Reduce the learnts clauses
		 */
		void reduceLearnts(); //TODO

		/**
		 * Return the current assignment for variable x
		 */
		inline lbool value(var x){return assignments[x];};
		/**
		 * Return the sobstitution of assignment to literal l
		 */
		inline lbool value(Literal l){
			if(l.val()>0) return assignments[l.index()];
			else{
				if(assignments[l.index()]!=U) return (assignments[l.index()]==T)?F:T;
				else return U;
			}
		};	

		/**
		 * Return the current decision level
		 */
		inline int decisionLevel(){return trail_lim.size();};
		/**
		 * Undo last propagation or decision in trail
		 */
		void undoOne();

		/**
		 * Bump activity
		 */
		void reward(Clause* c);

		/**
		 * Delete an entire level of decision with its propagations
		 */
		void cancel();

		/**
		 * Propagate operation
		 * This method support pragma openmp operation to parallelize into nThread the propagation for each clause. The number of thread depends on CPU cores.
		 */
		Clause* propagate();
		/**
		 * Pick a variable
		 */
		bool assume(Literal& p);

		/**
		 * Return the number of assigned variable
		 */
		int nAssigns();

		/**
		 * Method used to analyze the conflict
		 */
		int analyze(Clause* conflict, std::vector<Literal>& to_learn);

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
		bool enqueue(Literal p, Clause* c = nullptr);

		/**
		 * Method to decay all the activities
		 */
		void decayActivity();

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

		/**
		 * Record a learnt clause
		 */
		void record(std::vector<Literal>& clause);
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
		if(literals[0]!=~*p&&literals[1]!=~*p) return true;
#if ASSERT
		assert(size()>0);
		assert(literals[0]==~*p||literals[1]==~*p);
		assert(solver->value(~*p)==F);
#endif
		if(literals[0]==~*p){
			literals[0]=literals[1];literals[1]=~*p;
		}
#if ASSERT
		assert(literals[1]==~*p); //!< l2 = F
#endif
		if(solver->value(literals[0])==T){
			//Not necessary to search an other watched literal, waste of time
	/*		for(int i=2;i<size();++i){
				if(solver->value(literals[i])!=F){
					Literal tmp=literals[1];
					literals[1]=literals[i];literals[i]=tmp;
					solver->watched_list[-literals[1].val()].push_back(this);
					std::cout<<"swapped "<<literals[i]<<", with "<<literals[1]<<", deleted: "<<*this<<" and added to "<<-literals[1].val()<<" "<<*solver->watched_list[-literals[1].val()].back()<<"\n";
					break;
				}
			}*/
			solver->watches[p->val()].push_back(this);
			return true;
		}
		for(int i=2; i<size();++i){
			if(solver->value(literals[i])!=F){
				literals[1]=literals[i];literals[i]=~*p;
				solver->watches[-literals[1].val()].push_back(this);
				return true;
			}
		}
		solver->watches[p->val()].push_back(this);
		return solver->enqueue(literals[0], this);
	};

	inline void Clause::calcReason(Solver* solver, Literal& p, std::vector<Literal>& p_reason){
#if ASSERT
		assert(solver->value(p)==U||literals[0]==p);
#endif
		for(int i=((solver->value(p)==U)?0:1);i<size();++i) p_reason.push_back(~literals[i]);		
	};

	inline void Clause::undo(Solver* solver, Literal& p){
		solver->watches[-p.val()].push_back(this);
	};
#endif
