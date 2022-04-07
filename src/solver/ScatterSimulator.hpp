#pragma once

#include "Simulator.hpp"
#include "gnuplot/Gnuplot.hpp"

class ScatterSimulator : public Simulator {
private:
    std::vector<std::shared_ptr<SimuResult>> m_result;

public:
    ScatterSimulator(const std::string& jsonFile, double dt) : Simulator(jsonFile, SimulationMode::Scatter, dt) {}

    bool simulate() override;

    void saveResult() override;

    void plotToGnuplot() override;

private:
    void solve(double windSpeed, double windDir, std::shared_ptr<SimuResult>& result, bool* finish, bool* error);

    bool singleThreadSimulation();

    bool multiThreadSimulation();

    void formatResultForScatter(std::shared_ptr<SimuResult>& result);

    void eraseNotLandingPoint(std::shared_ptr<SimuResult>& result);

    bool updateWindCondition();
};
