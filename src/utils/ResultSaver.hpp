#pragma once

#include <memory>
#include <string>
#include <vector>

struct SimuResult;

namespace ResultSaver {
    void SaveScatter(const std::string& dir, const std::vector<std::shared_ptr<SimuResult>>& result);

    void SaveDetail(const std::string& dir, const std::shared_ptr<SimuResult>& result);
}
