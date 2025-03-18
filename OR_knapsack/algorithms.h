#pragma once
#ifndef ALGORITHMS_MS_H



#include <exception>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include "ortools/linear_solver/linear_solver.h"



namespace KP // knapsack
{
	class Output
	{
		bool _on = true;

	public:
		void set_on(bool on) { _on = on; }
		friend Output& operator<<(Output& output, const std::string& msg);
		friend Output& operator<<(Output& output, const char* msg);
		friend Output& operator<<(Output& output, long unsigned int value);
		friend Output& operator<<(Output& output, size_t value);
		friend Output& operator<<(Output& output, unsigned int value);
		friend Output& operator<<(Output& output, int value);
		friend Output& operator<<(Output& output, int64_t value);
		friend Output& operator<<(Output& output, float value);
		friend Output& operator<<(Output& output, double value);
	};

	///////////////////////////////////////////////////////////////////////////

	struct Item
	{
		int number;
		int value;
		int weight;
	};

	///////////////////////////////////////////////////////////////////////////

	// Base class
	class Algorithm
	{
	protected:
		Output _output;

		std::vector<Item> _items;
		int64_t _capacity;

		int64_t _best_value = std::numeric_limits<int64_t>::max();
		std::vector<int> _best_solution; // [i] == 1 if item i added to knapsack, 0 otherwise

	public:
		virtual ~Algorithm() {}

		void read_data(const std::string& filename);
		void generate_dataset(size_t nb_items, int type, int range, const std::string& name);

		virtual void run(bool verbose) = 0;
	};

	///////////////////////////////////////////////////////////////////////////

	class CompleteEnumeration : public Algorithm
	{
		void complete_enumeration_imp(std::vector<int>& sequence);

	public:
		void run(bool verbose) override;
	};

	///////////////////////////////////////////////////////////////////////////

	class BranchAndBound : public Algorithm
	{
	public:
		void run(bool verbose) override;
	};

	///////////////////////////////////////////////////////////////////////////

	class PisingerBB : public Algorithm
	{
		int64_t det(size_t a, size_t b, size_t c, size_t d) { return a * d - b * c; }

		size_t _nodes_explored;
		std::list<int> _exceptions;

		bool branching(int64_t P, int64_t W, int64_t s, int64_t t);

	public:
		void run(bool verbose) override;
	};

	///////////////////////////////////////////////////////////////////////////

	class ORToolsIP : public Algorithm
	{
		std::unique_ptr<operations_research::MPSolver> _solver; // OR Tools solver

		/*!
		 *	@brief Which solver is used?
		 *
		 *	solver_id is case insensitive, and the following names are supported:
		 *  - SCIP_MIXED_INTEGER_PROGRAMMING or SCIP
		 *  - CBC_MIXED_INTEGER_PROGRAMMING or CBC
		 *  - CPLEX_MIXED_INTEGER_PROGRAMMING or CPLEX or CPLEX_MIP			(license needed)
		 *  - GUROBI_MIXED_INTEGER_PROGRAMMING or GUROBI or GUROBI_MIP	    (license needed)
		 *  - XPRESS_MIXED_INTEGER_PROGRAMMING or XPRESS or XPRESS_MIP		(license needed)
		 */
		std::string _solver_type = "SCIP";

		void build_problem();
		void solve_problem();

		bool _output_screen = false;
		double _max_computation_time = 600; // seconds

	public:
		void run(bool verbose) override;
		void set_max_time(double time) { _max_computation_time = time; }
	};

	///////////////////////////////////////////////////////////////////////////

	class DPItems : public Algorithm
	{
	public:
		void run(bool verbose) override;
	};

	///////////////////////////////////////////////////////////////////////////

	class DPCapacity : public Algorithm
	{
	public:
		void run(bool verbose) override;
	};

	///////////////////////////////////////////////////////////////////////////

	class AlgorithmFactory
	{
	public:
		static std::unique_ptr<Algorithm> create(std::string& algorithm);
	};


} // namespace MS


#endif // !ALGORITHMS_MS_H