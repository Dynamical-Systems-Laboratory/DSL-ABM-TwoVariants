#include "../include/utils.h"

// Convert a string to all lowercase
std::string str_to_lower(std::string s) 
{
	std::transform(s.begin(), s.end(), s.begin(), 
                  [](unsigned char c){ return std::tolower(c); } );
    return s;
}

// Combine contributions from different lambdas
// Change things into references
std::vector<double> add_lambdas(std::vector<std::vector<double>>&& lambdas, 
						std::map<int, const std::vector<double>&>&& mult)
{
	std::vector<double> lambda_tot(lambdas.at(0).size(), 0.0);
	int i = 0;
	for (auto& lambda : lambdas)
	{
		auto iter = mult.find(i);
		if (iter != mult.end()) {
			// Need to multiply this lambda with factors corresponding to each strain
			const std::vector<double> factor = iter->second;
			// Multiplying lambda
			std::transform (lambda.begin(), lambda.end(), factor.cbegin(), lambda.begin(), std::multiplies<double>());
		}
		std::transform(lambda_tot.begin(), lambda_tot.end(), lambda.begin(), lambda_tot.begin(), std::plus<double>());
		++i;	
	}
	return lambda_tot;
}
