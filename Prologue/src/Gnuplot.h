#pragma once
#include <vector>
#include <string>

struct SolvedResult;

namespace Gnuplot {

	void Initialize(const char* dirname);

	void Plot(const SolvedResult& result);

	void Plot(const std::vector<SolvedResult>& result);

	void Show();

}