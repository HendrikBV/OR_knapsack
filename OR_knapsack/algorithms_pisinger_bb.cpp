#include "algorithms.h"
#include <chrono>
#include <stdexcept>


namespace KP
{

	bool PisingerBB::branching(int64_t P, int64_t W, int64_t s, int64_t t)
	{
		++_nodes_explored;
		bool improved = false;

		_output << "\n\nP = " << P << ", W = " << W << ", s = " << s + 1 << ", t = " << t + 1;

		if (W <= _capacity) // add some item j >= t
		{
			if (P > _best_value)
			{
				improved = true;
				_best_value = P;
				_exceptions.clear();

				_output << "\nNew best solution found!";
			}

			while (true)
			{
				// limits
				if (t >= _items.size())
				{
					_output << "\nCannot add any more items: Backtrack ...\n";
					return improved;
				}

				// upper bound test
				double UB = P + (_capacity - W) * (double)_items[t].value / (double)_items[t].weight;
				_output << "\nDo upper bound test: P + (C-W) ps/ws = " << P << " + (" << _capacity << "-"
					<< W << ")*" << _items[t].value << "/" << _items[t].weight << " = " << UB;

				if (det(P - _best_value - 1, W - _capacity, _items[t].value, _items[t].weight) < 0)
				{
					_output << "\nUB <= current best solution. Backtrack ...\n";
					return improved;
				}

				// branch one level further
				_output << "\nTry to add item " << t + 1;
				if (branching(P + _items[t].value, W + _items[t].weight, s, t + 1))
				{
					improved = true;
					_exceptions.push_back(t);
				}

				// go to next item
				++t;
			}
		}
		else // remove some item j <= s
		{
			while (true)
			{
				// limits
				if (s < 0)
				{
					_output << "\nCannot remove any more items: Backtrack ...\n";
					return improved;
				}

				// upper bound test
				double UB = P + (_capacity - W) * (double)_items[s].value / (double)_items[s].weight;
				_output << "\nDo upper bound test: P + (C-W) ps/ws = " << P << " + (" << _capacity << "-"
					<< W << ")*" << _items[s].value << "/" << _items[s].weight << " = " << UB;

				if (det(P - _best_value - 1, W - _capacity, _items[s].value, _items[s].weight) < 0)
				{
					_output << "\nUB <= current best solution. Backtrack ...\n";
					return improved;
				}

				// branch one level further
				_output << "\nTry to remove item " << s + 1;
				if (branching(P - _items[s].value, W - _items[s].weight, s - 1, t))
				{
					improved = true;
					_exceptions.push_back(s);
				}

				// go to previous item
				--s;
			}
		}
	}

	void PisingerBB::run(bool verbose)
	{
		_output.set_on(true);
		_output << "\n\n\nStarting the branch-and-bound method of Pisinger ...";
		_output.set_on(verbose);

		auto start_time = std::chrono::system_clock::now();
		std::chrono::duration<double, std::ratio<1, 1>> elapsed_time;



		// Initialize
		_best_solution.reserve(_items.size());
		for (size_t i = 0; i < _items.size(); ++i)
			_best_solution.push_back(0);
		_best_value = 0;
		_exceptions.clear();
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
		_output << "\n\nFinding the break item ...";
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
		_output << "\nBreakitem = " << breakitem_index + 1 << ", Pb = " << valuesum << ", Ws = " << weightsum;



		// 3. find a heuristic solution
		_output << "\n\nFinding a heuristic solution";

		// forward greedy
		for (size_t i = breakitem_index; i < _items.size(); ++i)
		{
			if (weightsum + _items[i].weight <= _capacity)
			{
				if (valuesum + _items[i].value > _best_value)
				{
					_best_value = valuesum + _items[i].value;

					_exceptions.clear();
					_exceptions.push_back(i);

					_output << "\nSolution found with z = " << _best_value << ", by adding item " << i + 1;
				}
			}
		}
		// backward greedy
		for (size_t i = 0; i < breakitem_index; ++i)
		{
			if (weightsum + _items[breakitem_index].weight - _items[i].weight <= _capacity)
			{
				if (valuesum + _items[breakitem_index].value - _items[i].value > _best_value)
				{
					_best_value = valuesum + _items[breakitem_index].value - _items[i].value;

					_exceptions.clear();
					_exceptions.push_back(i);
					_exceptions.push_back(breakitem_index);

					_output << "\nSolution found with z = " << _best_value << ", by adding the break item and removing item " << i + 1;
				}
			}
		}


		// 4. branching
		_output << "\n\nStart branching procedure ...";
		branching(valuesum, weightsum, breakitem_index - 1, breakitem_index);


		// 5. store optimal solution
		for (auto&& e : _exceptions)
			_best_solution[e] = (1 - _best_solution[e]);



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
				_output << "\nError in Pisinger: Weight " << Ws << " exceeds capacity " << _capacity;
			else if (Ps != _best_value)
				_output << "\nError in Pisinger: Value " << Ps << " not equal to best lower bound " << _best_value;
			else
				_output << "\nCheck OK";
		}
	}

}