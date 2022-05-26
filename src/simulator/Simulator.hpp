#pragma once
#include <string>
#include <vector>

#include "env/Environment.hpp"
#include "gnuplot/Gnuplot.hpp"
#include "rocket/RocketSpec.hpp"
#include "solver/Solver.hpp"

enum class SimulationMode : int { Scatter = 1, Detail };

class Simulator {
protected:
    // settings
    const std::string m_jsonFile;
    const SimulationMode m_simulationMode;
    RocketType m_rocketType;
    TrajectoryMode m_trajectoryMode;
    DetachType m_detachType;
    double m_detachTime;

    // simulate
    const double m_dt;
    double m_windSpeed     = 0.0;
    double m_windDirection = 0.0;  // direction is clockwise from the north

    // from json
    Environment m_environment;
    RocketSpec m_rocketSpec;
    MapData m_mapData;

    // result
    std::string m_outputDirName;

public:
    virtual ~Simulator() {}

    virtual void plotToGnuplot() = 0;

    static Simulator* New(double dt);

    bool run();

protected:
    Simulator(const std::string& jsonFile, SimulationMode simulationMode, double dt) :
        m_jsonFile(jsonFile), m_simulationMode(simulationMode), m_dt(dt) {}

    virtual bool simulate() = 0;

    virtual void saveResult() = 0;

private:
    bool initialize();

    static std::string SetJSONFile();

    static SimulationMode SetSimulationMode();

    void setTrajectoryMode();

    void setWindCondition();

    void setDetachType();

    void setDetachTime();

    void createResultDirectory();
};
