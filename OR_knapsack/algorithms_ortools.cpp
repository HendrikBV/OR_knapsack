#include "algorithms.h"
#include <stdexcept>
#include <memory>
#include <iostream>
#include <fstream>
#include <chrono>
#include <queue>


namespace KP
{
	void ORToolsIP::build_problem()
	{
		// create the solver (_solver_type == SCIP or CPLEX or ...)
		_solver.reset(operations_research::MPSolver::CreateSolver(_solver_type));


		// add variables
		const size_t nbitems = _items.size();
		const double infinity = _solver->infinity();


		// variables x_j
		for (size_t j = 0; j < nbitems; ++j)
		{
			std::string varname = "x_" + std::to_string(j + 1);
			operations_research::MPVariable* var = _solver->MakeBoolVar(varname);
		}


		// set objective function
		operations_research::MPObjective* objective = _solver->MutableObjective();
		objective->SetMaximization();
		for (auto j = 0; j < nbitems; ++j)
		{
			operations_research::MPVariable* var = _solver->variable(j);
			objective->SetCoefficient(var, _items[j].value);
		}


		// capacity constraint
		{
			std::string conname = "cap";
			operations_research::MPConstraint* constraint = _solver->MakeRowConstraint(-infinity, _capacity, conname);

			// x_j
			for (auto j = 0; j < nbitems; ++j)
			{
				operations_research::MPVariable* var = _solver->variable(j);
				constraint->SetCoefficient(var, _items[j].weight);
			}
		}

		// write to file
		//_solver->Write("ORToolsIP.lp"); // NOT YET SUPPORTED

		{
			std::ofstream mfile("ORTools_IP.lp");
			mfile << "Obj\t";
			for (auto& var : _solver->variables())
			{
				if (_solver->Objective().GetCoefficient(var) > 0.00001 || _solver->Objective().GetCoefficient(var) < -0.00001)
					mfile << _solver->Objective().GetCoefficient(var) << " " << var->name() << " + ";
			}

			for (auto& con : _solver->constraints())
			{
				mfile << "\n\n" << con->name() << "\t" << con->lb() << " <=  ";
				for (auto& var : _solver->variables())
				{
					if (con->GetCoefficient(var) > 0.00001 || con->GetCoefficient(var) < -0.00001)
						mfile << con->GetCoefficient(var) << " " << var->name() << " + ";
				}
				mfile << "  <= " << con->ub();
			}
		}
	}


	void ORToolsIP::solve_problem()
	{
		std::cout << "\nUsing an IP model implemented in ORTools with SCIP to solve the problem ...\n\n";

		// Output to screen
		if (_output_screen)
			_solver->EnableOutput();
		else
			_solver->SuppressOutput();

		// Set time limit (milliseconds) 
		int64_t time_limit = static_cast<int64_t>(_max_computation_time * 1000);
		_solver->set_time_limit(time_limit);

		// Solve the problem
		auto start_time = std::chrono::system_clock::now();
		const operations_research::MPSolver::ResultStatus result_status = _solver->Solve();
		std::chrono::duration<double, std::ratio<1, 1>> elapsed_time_IP = std::chrono::system_clock::now() - start_time;

		std::cout << "\nResult solve = " << result_status;

		// If optimal or feasible (e.g. time limit reached)
		if (result_status == operations_research::MPSolver::OPTIMAL || result_status == operations_research::MPSolver::FEASIBLE)
		{
			double objval = _solver->Objective().Value();
			_best_value = objval + 0.0001;

			_best_solution.clear();
			_best_solution.reserve(_items.size());
			for (auto j = 0; j < _items.size(); ++j) 
			{
				operations_research::MPVariable* var = _solver->variable(j);
				double solvalue = var->solution_value();

				if (solvalue > 0.99)
					_best_solution.push_back(1);
				else
					_best_solution.push_back(0);
			}

			std::cout << "\nElapsed time (s): " << elapsed_time_IP.count();

			std::cout << "\nObjective value = " << _best_value;
			std::cout << "\nItems selected: ";
			for (auto j = 0; j < _items.size(); ++j)
				if (_best_solution[j] > 0)
					std::cout << j + 1 << " ";
		}
	}

	void ORToolsIP::run(bool verbose)
	{
		_output_screen = verbose;

		build_problem();
		solve_problem();
	}
}