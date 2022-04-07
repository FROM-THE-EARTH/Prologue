#pragma once

#include "Simulator.hpp"
#include "gnuplot/Gnuplot.hpp"

class DetailSimulator : public Simulator {
private:
    ResultRocket m_result;

public:
    DetailSimulator(const std::string& jsonFile, double dt) : Simulator(jsonFile, SimulationMode::Detail, dt) {}

    bool simulate() override;

    void saveResult() override;

    void plotToGnuplot() override {
        Gnuplot::Plot(m_result);
    }
};
