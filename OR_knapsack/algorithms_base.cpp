#include "algorithms.h"
#include <fstream>
#include <random>
#include <stdexcept>
#include <iostream>



namespace KP
{

	Output& operator<<(Output& output, const std::string& msg)
	{
		if (output._on) {
			std::cout << msg;
		}
		// otherwise no output
		return output;
	}

	Output& operator<<(Output& output, const char* msg)
	{
		if (output._on) {
			std::cout << msg;
		}
		// otherwise no output
		return output;
	}

	Output& operator<<(Output& output, long unsigned int value)
	{
		if (output._on) {
			std::cout << value;
		}
		// otherwise no output
		return output;
	}

	Output& operator<<(Output& output, size_t value)
	{
		if (output._on) {
			std::cout << value;
		}
		// otherwise no output
		return output;
	}

	Output& operator<<(Output& output, unsigned int value)
	{
		if (output._on) {
			std::cout << value;
		}
		// otherwise no output
		return output;
	}

	Output& operator<<(Output& output, int value)
	{
		if (output._on) {
			std::cout << value;
		}
		// otherwise no output
		return output;
	}

	Output& operator<<(Output& output, int64_t value)
	{
		if (output._on) {
			std::cout << value;
		}
		// otherwise no output
		return output;
	}

	Output& operator<<(Output& output, float value)
	{
		if (output._on) {
			std::cout << value;
		}
		// otherwise no output
		return output;
	}

	Output& operator<<(Output& output, double value)
	{
		if (output._on) {
			std::cout << value;
		}
		// otherwise no output
		return output;
	}


	///////////////////////////////////////////////////////////////////////////


	void Algorithm::read_data(const std::string& filename)
	{
		_items.clear();

		std::ifstream file;
		file.open(filename);
		if (!file.is_open())
			throw std::invalid_argument("KP::Algorithm::import_dataset: Couldn't open file");

		std::string text, value;

		file >> text; // name of instance

		file >> text; // "capacity"
		file >> value; // capacity
		_capacity = std::stoull(value);

		file >> text; // "nb_items"
		file >> value;
		size_t nb_items = std::stoull(value);

		_items.reserve(nb_items);
		file >> text >> text >> text; // "item, value, weight"
		for (size_t i = 0; i < nb_items; ++i)
		{
			size_t number, value, weight;
			file >> number >> value >> weight;

			_items.push_back(Item());
			_items.back().number = number;
			_items.back().value = value;
			_items.back().weight = weight;
		}

	}


	void Algorithm::generate_dataset(size_t nb_items, int type, int range, const std::string& name)
	{
		if (type < 1 || type > 4)
			throw std::logic_error("Error in Knapsack::generate_data: Argument \"type\" should have a value of 1, 2, 3, or 4");

		std::random_device randdev;
		std::seed_seq seedseq{ randdev(),randdev(), randdev(), randdev(), randdev(), randdev(), randdev() };
		std::mt19937_64 generator(seedseq);

		std::uniform_int_distribution<> dist_weight(1, range);

		_items.clear();
		_items.reserve(nb_items);

		_capacity = 0;

		for (size_t i = 0; i < nb_items; ++i)
		{
			int weight = dist_weight(generator);
			int value = 0;

			if (type == 1) // uncorrelated
			{
				value = dist_weight(generator);
			}
			else if (type == 2) // weakly correlated
			{
				int LB = weight - 0.1 * range;
				if (LB < 1)
					LB = 1;
				int UB = weight + 0.1 * range;
				std::uniform_int_distribution<> dist_value(LB, UB);
				value = dist_value(generator);
			}
			else if (type == 3) // strongly correlated
			{
				value = weight + 10;
			}
			else if (type == 4) // subset sum
			{
				value = weight;
			}

			_capacity += weight;

			_items.push_back(Item());
			_items.back().number = i;
			_items.back().value = value;
			_items.back().weight = weight;
		}

		_capacity /= 2;


		std::ofstream file;
		file.open(name + ".txt");
		if (!file.is_open())
		{
			std::cout << "\Knapsack::generate_data: Couldn't open file";
			return;
		}
		file << name
			<< "\ncapacity\t" << _capacity
			<< "\nnb_items\t" << _items.size()
			<< "\nitem\tvalue\tweight";
		for (auto&& it : _items)
		{
			file << "\n" << it.number << "\t" << it.value << "\t" << it.weight;
		}
		file.flush();
		file.close();

	}


	///////////////////////////////////////////////////////////////////////////


	std::unique_ptr<Algorithm> AlgorithmFactory::create(std::string& algorithm)
	{
		std::transform(algorithm.begin(), algorithm.end(), algorithm.begin(),
			[](unsigned char c) { return std::tolower(c); });


		if (algorithm == "ce")
			return std::make_unique<CompleteEnumeration>();
		else if (algorithm == "bb")
			return std::make_unique<BranchAndBound>();
		else if (algorithm == "pi")
			return std::make_unique<PisingerBB>();
		else if (algorithm == "dp1")
			return std::make_unique<DPItems>();
		else if (algorithm == "dp2")
			return std::make_unique<DPCapacity>();
		else if (algorithm == "ip")
			return std::make_unique<ORToolsIP>();
		else
			throw std::invalid_argument("No algorithm " + algorithm + " exists");
	}
}