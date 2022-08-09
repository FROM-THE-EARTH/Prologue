// ------------------------------------------------
// Scatterモードに対するSimulatorBaseの定義
// ------------------------------------------------

#pragma once

#include <future>
#include <vector>

#include "SimulatorBase.hpp"

class ScatterSimulator : public SimulatorBase {
    // 型名が長いので別名を付けている
    using AsyncSolver = std::future<std::shared_ptr<SimuResultLogger>>;

private:
    double m_windSpeed     = 0.0;
    double m_windDirection = 0.0;

    std::vector<SimuResultSummary> m_result;

public:
    // 継承コンストラクタ
    using SimulatorBase::SimulatorBase;

    bool simulate() override;

    void saveResult() override;

    void plotToGnuplot() override;

private:
    std::shared_ptr<SimuResultLogger> solve(double windSpeed, double windDir);

    bool singleThreadSimulation();

    bool multiThreadSimulation();

    bool updateWindCondition();

    // Launch thread to solve and update wind condition
    // Return false if this is last solve
    bool launchNextAsyncSolve(AsyncSolver& solver);
};
