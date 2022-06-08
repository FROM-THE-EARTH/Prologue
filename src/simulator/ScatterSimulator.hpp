#pragma once

#include <future>

#include "Simulator.hpp"
#include "gnuplot/Gnuplot.hpp"

class ScatterSimulator : public Simulator {
    using AsyncSolver = std::future<std::shared_ptr<SimuResultLogger>>;

private:
    std::vector<SimuResultSummary> m_result;

public:
    ScatterSimulator(const std::string& jsonFile, double dt) : Simulator(jsonFile, SimulationMode::Scatter, dt) {}

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
