// ------------------------------------------------
// Detailモードに対するSimulatorBaseの定義
// ------------------------------------------------

#pragma once

#include "SimulatorBase.hpp"
#include "core/SimulationResult.hpp"

class DetailSimulator : public SimulatorBase {
private:
    SimulationResult m_result;

public:
    // 継承コンストラクタ
    using SimulatorBase::SimulatorBase;

    bool simulate() override;

    void saveResult() override;

};
