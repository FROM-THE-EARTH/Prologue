#pragma once
#include <string>
#include <vector>

#include "Solver.hpp"
#include "gnuplot/Gnuplot.hpp"
#include "rocket/RocketSpec.hpp"

enum class SimulationMode : int { Scatter = 1, Detail };

class Simulator {
    // settings
    std::string m_jsonFilename;
    SimulationMode m_simulationMode;
    RocketType m_rocketType;
    TrajectoryMode m_trajectoryMode;
    DetachType m_detachType;
    double m_detachTime;

    // simulate
    const double m_dt;
    double m_windSpeed     = 0.0;
    double m_windDirection = 0.0;  // direction is clockwise from the north
    bool m_solved          = false;

    // from json
    RocketSpec m_rocketSpec;
    MapData m_mapData;

    // result
    std::string m_outputDirName;
    SolvedResult m_detailResult;
    std::vector<SolvedResult> m_scatterResult;

public:
    Simulator(double dt) : m_dt(dt) {}

    bool run();

    void plotToGnuplot() {
        switch (m_simulationMode) {
        case SimulationMode::Scatter:
            Gnuplot::Plot(m_scatterResult);
            break;

        case SimulationMode::Detail:
            Gnuplot::Plot(m_detailResult);
            break;
        }
    }

private:
    bool initialize();

    void setJSONFile();
    void setSimulationMode();
    void setTrajectoryMode();
    void setWindCondition();
    void setDetachType();
    void setDetachTime();

    void scatterSimulation();
    void detailSimulation();

    void singleThreadSimulation();
    void multiThreadSimulation();

    void solve(double windSpeed, double windDir, SolvedResult* result, bool* finish, bool* error);

    SolvedResult formatResultForScatter(const SolvedResult& result);

    void eraseNotLandingPoint(SolvedResult* result);

    void createResultDirectory();

    void saveResult();

    bool updateWindCondition();
};
