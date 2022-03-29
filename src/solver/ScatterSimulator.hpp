#pragma once

#include "Simulator.hpp"
#include "gnuplot/Gnuplot.hpp"

class ScatterSimulator : public Simulator {
private:
    std::vector<SolvedResult> m_result;

public:
    ScatterSimulator(const std::string& jsonFile, double dt) : Simulator(jsonFile, SimulationMode::Scatter, dt) {}

    bool simulate() override;

    void saveResult() override;

    void plotToGnuplot() override {
        Gnuplot::Plot(m_result);
    }

private:
    void solve(double windSpeed, double windDir, SolvedResult* result, bool* finish, bool* error);

    bool singleThreadSimulation();

    bool multiThreadSimulation();

    SolvedResult formatResultForScatter(const SolvedResult& result);

    void eraseNotLandingPoint(SolvedResult* result);

    bool updateWindCondition();
};
