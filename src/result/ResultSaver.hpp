// ------------------------------------------------
// 結果保存用関数
// ------------------------------------------------

#pragma once

#include <memory>
#include <string>
#include <vector>

struct SimuResultSummary;

namespace ResultSaver {
    void SaveScatter(const std::string& dir, const std::vector<SimuResultSummary>& result);

    void SaveDetail(const std::string& dir, const SimuResultSummary& result);
}
