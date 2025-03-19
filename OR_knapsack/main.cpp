#include "algorithms.h"
#include "cxxopts.h"
#include <iostream>
#include <memory>



int main(int argc, char* argv[])
{
	try
	{
		cxxopts::Options options("OR -- Knapsack",
			"This program implements several algorithms for the 0-1 knapsack problem.");

		options.add_options()
			("algorithm", "The choice of algorithm. Possibilities:"
				"\n\t* \"CE\": complete enumeration"
				"\n\t* \"IP\": an integer programming model solved with SCIP"
				"\n\t* \"BB\": the standard branch-and-bound approach of Winston"
				"\n\t* \"PI\": the branch-and-bound approach of Pisinger"
				"\n\t* \"DP1\": dynamic programming with a stage for every item"
				"\n\t* \"DP2\": dynamic programming with a stage for every possible residual capacity"
				, cxxopts::value<std::string>())
			("data", "Name of the file containing the problem data", cxxopts::value<std::string>())
			("verbose", "Explain the various steps of the algorithm", cxxopts::value<bool>())
			("help", "Help on how to use the application");



		auto result = options.parse(argc, argv);

		if (argc <= 1 || result.count("help")) 
		{
			std::cout << options.help() << "\n\n\n\n\n";
			return EXIT_SUCCESS;
		}

		std::string algorithm;
		if (result.count("algorithm"))
			algorithm = result["algorithm"].as<std::string>();

		std::string datafile;
		if (result.count("data"))
			datafile = result["data"].as<std::string>();

		bool verbose = false;
		if (result.count("verbose"))
			verbose = result["verbose"].as<bool>();




		// create the algorithm and run it
		std::unique_ptr<KP::Algorithm> problem = KP::AlgorithmFactory::create(algorithm);
		problem->read_data(datafile);
		problem->run(verbose);



		std::cout << "\n\n\n\n\n";
		return EXIT_SUCCESS;
	}
	catch (const std::exception& e)
	{
		std::cout << "\n\n" << e.what() << "\n\n\n\n\n";
		return EXIT_FAILURE;
	}
}