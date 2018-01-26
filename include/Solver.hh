#ifndef SOLVER_HH
#define SOLVER_HH
#include "Config.h"
#include "Datastructure.hh"
#include <queue>
#include <list>
#include "omp.h"
#include <cassert>
#include <memory>
#include <math.h>

class Solver{
	public:
		friend class Clause; //!< Clause class can access member of Solver
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
		void newClause(std::vector<Literal>& lits_val, bool learnt=false);
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
		inline std::vector<struct btree<Clause>>& getProve(){return prove;};

		/**
		 * Return the number of deleted clauses
		 */
		inline int getDeletedClause(){return deletedClauses;};

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
		/**
		 * Return the number of restarts
		 */
		int nRestart(){return nRestarts;};

		/**
		 * Return the name of the current problem
		 */
		std::string pName(){return problemName;};
		/**
		 * Set the name of the problem
		 */
		void setName(std::string name){problemName=name;};

	
	private:
		//!< Constants values
		const int root_level=0; //!< root level definition. It must be 0.
		const int luby_base=100; //!< luby base factor. 
		std::string problemName; //!< problem name
		int max_conflict=1; //!< max conflict before restart
		int deletedClauses=0; //!< number of deleted clauses

		int nRestarts=0; //!< number of restarts
		int nConflicts=0; //!< number of conflicts
		int nPropagations=0; //!< number of propagations
		int nDecisions=0; //!< number of decisions

		std::vector<btree<Clause>> prove; //!< Structure to get the counterexample

		
		bool initialized=false; //!< boolean variable to check if solver was initialized correctly
		int nClauses; //!< number of clauses problem
		int nLiterals; //!< number of literals problem

		// Default constraints
		std::vector<Clause*> clauses; //!< List of clauses.
		std::vector<Clause*> learnts; //!< List of learnt clauses.

		// Propagation constraints
		std::vector<Watcher>*  watches; //!< For each literal with a positive phase a list of clause to be watched if p changes value.
		Clause* reason; //!< reason vector
		std::queue<Literal> propQ; //!< propagation queue

		// Assignment values
		std::vector<lbool> assignments; //!< Assignment indexed by variable. Size of this is the number of vars.
		std::vector<Literal> trail; //!< List of assignment in chronological order.
		std::vector<int> trail_lim; //!< Current level spartition. Based on trail
		std::vector<int> levels; //!< For each variable the level it belongs to.
		
		//!< Decay activities
		float* activity;


		/**
		 * Internal clear method for vector
		 */
		template<class T>
		inline void clear(std::vector<T>& origin){
			std::vector<T> empty;
			std::swap(origin, empty);
		};

		/**
		 * Internal clear method for queue
		 */
		template<class T>
		inline void clear(std::queue<T>& origin){
			std::queue<T> empty;
			std::swap(origin, empty);
		};

//Method to verify
#if ASSERT
/**
 * Check if watches at index i is consistent
 */
void assertWatches(int index);
		
/**
* Check if is possible to be SAT
*/
bool canBeSAT();

#endif

		/**
		 * Select new variable
		 */
		Literal select();

		/**
		 * Simplify the original set of variables
		 */
		void simplify();

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
		int analyze(Clause* conflict, std::vector<Literal>& to_learn, bool unsat=false);

		/**
		 * Method used to backtrack
		 */
		void backtrack(int btLevel);
		
		/**
		 * Enqueue method to check if exist a conflict and if not insert into propagation queue next propagate literal, agiving asignement etc..
		 */
		bool enqueue(Literal p, Clause* c = nullptr);

		/**
		 * Method to decay all the activities
		 */
		void decayActivity();

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
		/**
		 * Attach wathcer to watches
		 */
		void attachWatcher(Clause* clause);
#if PROVE
		void completeResolvs(Clause* conflict);
#endif
		/**
		 * Sussume a set of clauses
		 */
		void sussume(Clause& c);
		/**
		 * Luby activity
		 * It receive the increment value in y and the current number of conflict and stores the new luby sequence on max_conflict
		 */
		inline void lubyActivity(){
			int y=2;int x=nRestarts;
			int size, seq;
			for(size=1,seq=0;size<x+1;seq++,size=2*size+1);
			while(size-1!=x){
				size=(size-1)>>1;
				seq--;
				x=x%size;
			}
			max_conflict=(pow(y,seq))*luby_base;
		};
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
		if(deleted){
		       	return true;
		}
#if ASSERT
		assert(size()>0);
		assert(literals[0]==~*p||literals[1]==~*p);
		assert(solver->value(~*p)==F);
#endif
		
		if(literals[0]==~*p){
			literals[0]=literals[1];literals[1]=~*p;
		}
		assert(literals[1]==~*p); //!< l2 = F
		
		if(solver->value(literals[0])==T){
			solver->watches[p->val()].push_back(Watcher(this, *p));
#if ASSERT
solver->assertWatches(p->val());
#endif
			return true;
		}
		for(int i=2; i<size();++i){
			if(solver->value(literals[i])!=F){
				literals[1]=literals[i];literals[i]=~*p;
				solver->watches[-literals[1].val()].push_back(Watcher(this,literals[1]));
#if ASSERT
solver->assertWatches(-literals[1].val());
#endif
				return true;
			}
		}
		
		solver->watches[p->val()].push_back(Watcher(this, *p));
#if ASSERT
solver->assertWatches(p->val());
#endif
		return solver->enqueue(literals[0], this);
	};

	inline void Clause::calcReason(Solver* solver, Literal& p, std::vector<Literal>& p_reason){
		assert(solver->value(p)==U||literals[0]==p);
		for(int i=((solver->value(p)==U)?0:1);i<size();++i) p_reason.push_back(~literals[i]);		
	};


	inline bool Clause::satisfied(Solver* solver){
		for(auto x : literals){
			if(solver->value(x)==T) return true;
		}
		return false;
	};

	inline void Clause::detach(Solver* solver){
		for(auto x=solver->watches[-literals[0].val()].begin();x!=solver->watches[-literals[0].val()].end();++x){
			if(*(*x).cref==*this) {solver->watches[-literals[0].val()].erase(x);break;}
		}
		for(auto x=solver->watches[-literals[1].val()].begin();x!=solver->watches[-literals[1].val()].end();++x){
			if(*(*x).cref==*this) {solver->watches[-literals[1].val()].erase(x);break;}
		}

	};

#endif
