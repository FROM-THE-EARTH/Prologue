#pragma once

#include <vector>
#include <string>

struct SolvedResult;

namespace ResultSaver {
	void SaveScatter(const std::string& dir, const std::vector<SolvedResult>& result);

	void SaveScatterAll(const std::string& dir, const std::vector<SolvedResult>& result);

	void SaveDetail(const std::string& dir, const SolvedResult& result);

	void SaveDetailAll(const std::string& dir, const SolvedResult& result);
}