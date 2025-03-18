#include "algorithms.h"
#include <chrono>
#include <stdexcept>



namespace KP
{
	void CompleteEnumeration::complete_enumeration_imp(std::vector<int>& item_selected)
	{
		if (item_selected.size() == _items.size())
		{
			size_t value = 0, weight = 0;

			_output << "\nx = [ ";
			for (size_t i = 0; i < _items.size(); ++i)
			{
				_output <<  item_selected[i] << " ";

				value += item_selected[i] * _items[i].value;
				weight += item_selected[i] * _items[i].weight;
			}

			_output << "]\tz= " << value << "\tw = " << weight;
			if (weight > _capacity)
				_output << "\tinfeasible";

			else if (value > _best_value)
			{
				_best_value = value;
				_best_solution = item_selected;
			}

			item_selected.pop_back();
			return;
		}

		// left branch
		item_selected.push_back(0);
		complete_enumeration_imp(item_selected);

		// right branch
		item_selected.push_back(1);
		complete_enumeration_imp(item_selected);

		if (item_selected.size() > 0)
			item_selected.pop_back();
	}

	void CompleteEnumeration::run(bool verbose)
	{
		_output.set_on(true);
		_output << "\n\n\nComplete enumeration:\n";
		_output.set_on(verbose);


		auto start_time = std::chrono::system_clock::now();
		std::chrono::duration<double, std::ratio<1, 1>> elapsed_time;

		_best_solution.clear();
		_best_value = 0;

		std::vector<int> vec;
		vec.reserve(_items.size());

		complete_enumeration_imp(vec);

		_output.set_on(true);
		_output << "\n\nBest solution:";
		_output << "\nx = [ ";
		size_t value = 0, weight = 0;
		for (size_t i = 0; i < _items.size(); ++i)
		{
			_output << _best_solution[i] << " ";
			value += _best_solution[i] * _items[i].value;
			weight += _best_solution[i] * _items[i].weight;
		}
		_output << "]";
		_output << "\tz = " << value;
		_output << "\tw = " << weight;


		elapsed_time = std::chrono::system_clock::now() - start_time;
		_output << "\n\nComputation time (s): " << elapsed_time.count();
	}

}