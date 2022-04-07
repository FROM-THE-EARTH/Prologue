#pragma once

#include <string>
#include <vector>

struct ResultRocket;

namespace ResultSaver {
    void SaveScatter(const std::string& dir, const std::vector<ResultRocket>& result);

    void SaveScatterAll(const std::string& dir, const std::vector<ResultRocket>& result);

    void SaveDetail(const std::string& dir, const ResultRocket& result);

    void SaveDetailAll(const std::string& dir, const ResultRocket& result);
}
