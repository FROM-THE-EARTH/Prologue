// ------------------------------------------------
// SimulatorBaseクラスの生成を担う
// コマンドラインで設定を入力し、それを元に生成する
// ------------------------------------------------

#pragma once

#include <memory>

#include "simulator/SimulatorBase.hpp"

namespace SimulatorFactory {
    // DetailSimulatorまたはScatterSimulatorの生成
    std::unique_ptr<SimulatorBase> Create();
}
