#pragma once

#include "Simulator.hpp"
#include "gnuplot/Gnuplot.hpp"

class ScatterSimulator : public Simulator {
private:
    std::vector<ResultRocket> m_result;

public:
    ScatterSimulator(const std::string& jsonFile, double dt) : Simulator(jsonFile, SimulationMode::Scatter, dt) {}

    bool simulate() override;

    void saveResult() override;

    void plotToGnuplot() override;

private:
    void solve(double windSpeed, double windDir, ResultRocket* result, bool* finish, bool* error);

    bool singleThreadSimulation();

    bool multiThreadSimulation();

    ResultRocket formatResultForScatter(const ResultRocket& result);

    void eraseNotLandingPoint(ResultRocket* result);

    bool updateWindCondition();
};
