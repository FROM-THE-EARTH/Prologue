// ------------------------------------------------
// Detailモードに対するSimulatorBaseの定義
// ------------------------------------------------

#pragma once

#include "SimulatorBase.hpp"

class DetailSimulator : public SimulatorBase {
private:
    SimuResultSummary m_result;

public:
    // 継承コンストラクタ
    using SimulatorBase::SimulatorBase;

    bool simulate() override;

    void saveResult() override;

    void plotToGnuplot() override;
};
