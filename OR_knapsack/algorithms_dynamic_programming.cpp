#include "algorithms.h"
#include <chrono>
#include <stdexcept>
#include <algorithm>



namespace KP
{
	void DPCapacity::run(bool verbose)
	{
		_output.set_on(true);
		_output << "\n\n\nDynamic programming with a stage for every residual capacity ...";
		_output.set_on(verbose);

		auto start_time = std::chrono::system_clock::now();
		std::chrono::duration<double, std::ratio<1, 1>> elapsed_time;

		// initialize
		_best_solution.reserve(_items.size());
		for (size_t i = 0; i < _items.size(); ++i)
			_best_solution.push_back(0);

		// forward arcs
		struct Stage
		{
			int64_t best_profit;
			int64_t best_item = -1;
			std::vector<int> items_already_added;
		};
		std::vector<Stage> stages;

		for (size_t available = 0; available <= _capacity; ++available)
		{
			_output << "\n\nWe go to the next stage with an availability of " << available;

			int best_profit = 0;
			int best_item = -1;
			int best_residualcap = 0;
			for (size_t j = 0; j < _items.size(); ++j)
			{
				int residualcap = available - _items[j].weight;

				bool item_not_yet_added = true;
				if (residualcap >= 0) {
					for (auto&& k : stages[residualcap].items_already_added) {
						if (k == j) {
							item_not_yet_added = false;
							break;
						}
					}
				}

				if (residualcap >= 0
					&& _items[j].value + stages[residualcap].best_profit > best_profit
					&& item_not_yet_added)
				{
					best_profit = _items[j].value + stages[residualcap].best_profit;
					best_item = j;
					best_residualcap = residualcap;
				}

				if (residualcap < 0)
					_output << "\nWe cannot add item " << j + 1 << " because there is not sufficient capacity left";
				else if (!item_not_yet_added)
					_output << "\nWe cannot add item " << j + 1 << " because it was already added in a previous stage";
				else
					_output << "\nIf we add item " << j + 1 << " the profit is p[" << j + 1 <<
					"] + g(" << residualcap << ") = " << _items[j].value << " + "
					<< stages[residualcap].best_profit << " = " << _items[j].value + stages[residualcap].best_profit;
			}
			stages.push_back(Stage());
			stages.back().best_item = best_item;
			stages.back().best_profit = best_profit;
			stages.back().items_already_added = stages[best_residualcap].items_already_added;
			stages.back().items_already_added.push_back(best_item);

			_output << "\nThe best choice is thus to add item " << best_item + 1 <<
				" for a profit of g(" << available << ") = " << best_profit << "\nItems added: {";
			for (auto&& mm : stages.back().items_already_added)
				if (mm >= 0)
					_output << " " << mm + 1;
			_output << " }";
		}

		// reconstruct solution
		_output.set_on(true);
		_output << "\n\nThe optimal solution is:";
		_output << "\n\tz = " << stages[_capacity].best_profit << "\n";

		if (_items.size() <= 20)
		{
			int64_t residualcap = _capacity;

			int minweight = std::numeric_limits<int>::max();
			for (auto&& i : _items)
				if (i.weight < minweight)
					minweight = i.weight;

			while (residualcap >= minweight)
			{
				if (stages[residualcap].best_item >= 0)
					_best_solution[stages[residualcap].best_item] = 1;
				residualcap -= _items[stages[residualcap].best_item].weight;
			}
			if (_items.size() <= 10)
				for (size_t j = 0; j < _items.size(); ++j)
					_output << "\tx[" << j + 1 << "] = " << _best_solution[j];
		}

		elapsed_time = std::chrono::system_clock::now() - start_time;
		_output << "\n\nComputation time (s): " << elapsed_time.count();
	}

	///////////////////////////////////////////////////////////////////////////

	void DPItems::run(bool verbose)
	{
		_output.set_on(true);
		_output << "\n\n\nDynamic programming with a stage for every item and states for the residual capacity ...";
		_output.set_on(verbose);

		auto start_time = std::chrono::system_clock::now();
		std::chrono::duration<double, std::ratio<1, 1>> elapsed_time;


		// Nodes to save states
		struct Node
		{
			int stage = -1;
			int available = -1;
			int bestprofit = -1;
			int amount = 0;
		};
		std::vector<std::vector<Node>> states;
		states.push_back(std::vector<Node>());
		for (int stage = 1; stage < _items.size(); ++stage) {
			states.push_back(std::vector<Node>());
			for (size_t available = 0; available <= _capacity; ++available) {
				states.back().push_back(Node());
			}
		}


		// stages from last until second
		for (int stage = _items.size() - 1; stage > 0; --stage)
		{
			_output << "\n\n\nWe go to stage " << stage + 1;
			if (stage == _items.size() - 1)
				_output << "\nThis is the final stage, so we don't have to take other stage into account.";

			for (int available = 0; available <= _capacity; ++available)
			{
				// last stage
				if (stage == _items.size() - 1)
				{
					// maximum 1 item
					int amountcurrent = std::min(available / _items[stage].weight, 1);
					int profit = amountcurrent * _items[stage].value;

					states[stage][available].stage = stage;
					states[stage][available].available = available;
					states[stage][available].amount = amountcurrent;
					states[stage][available].bestprofit = profit;

					_output << "\n\nFor d = " << available << ", we can add item " << stage + 1 << " at most " << amountcurrent
						<< " times, for a profit of " << profit;
					_output << "\nThen f_" << stage + 1 << "(" << available << ") = " << states[stage][available].bestprofit;
				}
				else
				{
					// max 1 item
					int maxcurrent = std::min(available / _items[stage].weight, 1);
					_output << "\n\nFor d = " << available << ", we can add item " << stage + 1 << " at most " << maxcurrent << " times.";

					for (int amountcurrent = 0; amountcurrent <= maxcurrent; ++amountcurrent)
					{
						int residualcap = available - amountcurrent * _items[stage].weight;
						int profit = amountcurrent * _items[stage].value + states[stage + 1][residualcap].bestprofit;

						_output << "\nIf we set x[" << stage + 1 << "] = " << amountcurrent << ", then the residual capacity is " << residualcap;
						_output << "\nThe profit is then " << amountcurrent << " * " << _items[stage].value << " + f_" << stage + 2 << "(" << residualcap << ") = " << profit;

						if (profit > states[stage][available].bestprofit)
						{
							states[stage][available].stage = stage;
							states[stage][available].available = available;
							states[stage][available].amount = amountcurrent;
							states[stage][available].bestprofit = profit;
						}
					}

					_output << "\nThe best choice at this stage when d = " << available << " is thus x[" << stage+1 <<
						"] = " << states[stage][available].amount << " for a profit of " << states[stage][available].bestprofit;
					_output << "\nWe save f_" << stage+1 << "(" << available << ") = " << states[stage][available].bestprofit;

				}
			}
		}

		// first stage
		int bestprofit = 0;
		int amount_item1 = 0;
		{
			int stage = 0;
			int available = _capacity;

			_output << "\n\n\nWe are now at stage 1, so we only need to look at an availability of " << _capacity;

			int maxcurrent = std::min(available / _items[stage].weight, 1);
			for (int amountcurrent = 0; amountcurrent <= maxcurrent; ++amountcurrent)
			{
				int residualcap = available - amountcurrent * _items[stage].weight;
				int profit = amountcurrent * _items[stage].value + states[stage + 1][residualcap].bestprofit;

				_output << "\nIf we set x[" << stage + 1 << "] = " << amountcurrent << ", then the residual capacity is " << residualcap;
				_output << "\nProfit is then " << amountcurrent << " * " << _items[stage].value 
					<< " + f_" << stage + 2 << "(" << residualcap << ") = " << profit;

				if (profit > bestprofit)
				{
					amount_item1 = amountcurrent;
					bestprofit = profit;
				}
			}

			_output << "\nThe best choice is thus x[" << stage + 1 << "] = " << amount_item1 << " for a profit of " << bestprofit;
		}

		// reconstruct solution
		_output.set_on(true);
		_output << "\n\n\nThe optimal solution has been identified.\n\tz = " << bestprofit;

		if (_items.size() <= 20)
		{
			_output << "\n\tx[1] = " << amount_item1;
			int available = _capacity - amount_item1 * _items[0].weight;
			for (int stage = 1; stage < _items.size(); ++stage)
			{
				_output << "\tx[" << stage + 1 << "] = " << states[stage][available].amount;
				available -= states[stage][available].amount * _items[stage].weight;
			}
		}

		elapsed_time = std::chrono::system_clock::now() - start_time;
		_output << "\n\nComputation time (s): " << elapsed_time.count();
	}
}