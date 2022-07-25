#pragma once

#include "Simulator.hpp"

class DetailSimulator : public Simulator {
private:
    SimuResultSummary m_result;

public:
    DetailSimulator(const std::string& jsonFile, double dt) : Simulator(jsonFile, SimulationMode::Detail, dt) {}

    bool simulate() override;

    void saveResult() override;

    void plotToGnuplot() override;
};
