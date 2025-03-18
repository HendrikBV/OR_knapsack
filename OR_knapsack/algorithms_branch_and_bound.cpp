#include "algorithms.h"
#include <chrono>
#include <stdexcept>



namespace KP
{
	void BranchAndBound::run(bool verbose)
	{
		_output.set_on(true);
		_output << "\n\n\nStarting the branch-and-bound method of Winston ...";
		_output.set_on(verbose);

		auto start_time = std::chrono::system_clock::now();
		std::chrono::duration<double, std::ratio<1, 1>> elapsed_time;

		// Initialize
		size_t _nodes_explored = 0;
		std::vector<int> _items_x; // current solution
		std::vector<bool> _items_fixed; // current branching restrictions

		_best_solution.reserve(_items.size());
		for (size_t i = 0; i < _items.size(); ++i)
		{
			_items_x.push_back(0);
			_items_fixed.push_back(false); // all items free
			_best_solution.push_back(0); // all x = 0
		}
		_best_value = 0;
		_nodes_explored = 0;



		// 1. sort items based on efficiency (highest to lowest)
		_output << "\n\nSorting the items from most efficient to least efficient";
		std::sort(_items.begin(), _items.end(), [](const Item& a, const Item& b)
			{
				if (a.value * b.weight > a.weight * b.value)
					return true;
				return false;
			}
		);



		// 2. find the break item and store x vector
		size_t breakitem_index = 0;
		size_t weightsum = 0, valuesum = 0;
		for (size_t i = 0; i < _items.size(); ++i)
		{
			weightsum += _items[i].weight;
			if (weightsum > _capacity)
			{
				breakitem_index = i;
				weightsum -= _items[i].weight;
				break;
			}
			valuesum += _items[i].value;
			_best_solution[i] = 1;
		}
		_best_value = valuesum;




		// 3. branching
		struct Node
		{
			std::vector<std::pair<size_t, bool>> fixed_path; // Sequence of (variable, value) fixed to reach this node
		};
		std::vector<Node> nodes;
		nodes.push_back({ { { breakitem_index, false } } });
		nodes.push_back({ { { breakitem_index, true } } });

		while (!nodes.empty())
		{
			Node current = nodes.back();
			nodes.pop_back();
			++_nodes_explored;

			// reset state
			for (size_t i = 0; i < _items.size(); ++i)
			{
				_items_fixed[i] = false;
				_items_x[i] = 0;
			}

			// apply the fixed path
			for (const auto& it : current.fixed_path)
			{
				_items_fixed[it.first] = true;
				_items_x[it.first] = it.second ? 1 : 0;
			}

			size_t branching_variable = current.fixed_path.back().first;
			bool x = current.fixed_path.back().second;
			_output << "\n\nNew node: x[" << branching_variable + 1 << "] = " << static_cast<int>(x);


			// 2. calculate solution LP relaxation
			size_t new_branching_var = 0;
			bool integer = true;
			bool feasible = true;
			double z_LP = 0;
			size_t upperbound = 0;
			{
				size_t Ps = 0, Ws = 0;
				double fraction = 0;

				for (size_t i = 0; i < _items.size(); ++i) // first fixed variables
				{
					if (_items_fixed[i] && _items_x[i] == 1)
					{
						Ps += _items[i].value;
						Ws += _items[i].weight;
					}
					else if (!_items_fixed[i])
						_items_x[i] = 0;
				}
				if (Ws > _capacity)
				{
					feasible = false;
				}
				else if (Ws == _capacity)
				{
					upperbound = Ps;
				}
				else
				{
					for (size_t i = 0; i < _items.size(); ++i) // then free variables
					{
						if (!_items_fixed[i])
						{
							if (Ws + _items[i].weight <= _capacity)
							{
								Ps += _items[i].value;
								Ws += _items[i].weight;
								upperbound = Ps;
								z_LP = Ps;

								_items_x[i] = 1;

								if (Ws == _capacity)
									break;
							}
							else
							{
								integer = false;
								fraction = (static_cast<double>(_capacity) - static_cast<double>(Ws)) / static_cast<double>(_items[i].weight);
								z_LP = Ps + fraction * _items[i].value;
								upperbound = static_cast<size_t>(z_LP + 0.02);
								new_branching_var = i;

								break;
							}
						}
					}
				}
			}


			// 3. branch further or backtrack
			if (!feasible)
			{
				_output << "\nInfeasible. Backtrack ...";
			}
			else if (integer)
			{
				_output << "\nInteger solution with z = " << upperbound;

				if (upperbound > _best_value)
				{

					_best_value = upperbound;
					for (size_t i = 0; i < _items.size(); ++i)
						_best_solution[i] = _items_x[i];

					_output << "\nNew best solution found! Update LB = " << upperbound;
				}

				_output << "\nBacktrack ...";
			}
			else if (upperbound <= _best_value) // fathom due to upper bound test
			{
				_output << "\nupperbound = " << upperbound << " <= LB = " << _best_value << ". Backtrack ...";
			}
			else // branch further
			{
				_output << "\nz = " << z_LP;

				// Create new nodes by extending the fixed path
				std::vector<std::pair<size_t, bool>> path_true = current.fixed_path;
				path_true.push_back({ new_branching_var, true });
				nodes.push_back({ path_true });

				std::vector<std::pair<size_t, bool>> path_false = current.fixed_path;
				path_false.push_back({ new_branching_var, false });
				nodes.push_back({ path_false });
			}
		}



		// Output
		elapsed_time = std::chrono::system_clock::now() - start_time;
		_output.set_on(true);
		_output << "\n\n\nComputation time (s): " << elapsed_time.count()
			<< "\nNodes explored: " << _nodes_explored
			<< "\nOptimal solution: z = " << _best_value << "\n";
		if (_items.size() <= 10) {
			for (size_t i = 0; i < _items.size(); ++i) {
				if (_best_solution[i] == 1) {
					_output << "Item " << _items[i].number + 1 << "  ";
				}
			}
		}


		// Check
		{
			size_t Ps = 0, Ws = 0;
			for (size_t i = 0; i < _items.size(); ++i)
			{
				if (_best_solution[i] == 1)
				{
					Ps += _items[i].value;
					Ws += _items[i].weight;
				}
			}
			if (Ws > _capacity)
				_output << "\nError in BB Winston: Weight " << Ws << " exceeds capacity " << _capacity;
			else if (Ps != _best_value)
				_output << "\nError in BB Winston: Value " << Ps << " not equal to best lower bound " << _best_value;
			else
				_output << "\nCheck OK";
		}
	}

}