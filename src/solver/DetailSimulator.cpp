#include "DetailSimulator.hpp"

#include "utils/ResultSaver.hpp"

bool DetailSimulator::simulate() {
    Solver solver(
        m_dt, m_mapData, m_rocketType, m_trajectoryMode, m_detachType, m_detachTime, m_environment, m_rocketSpec);

    if (!solver.run(m_windSpeed, m_windDirection)) {
        return false;
    }

    m_result = solver.getResult();
    m_result.organize(m_mapData);

    return true;
}

void DetailSimulator::saveResult() {
    const std::string dir = "result/" + m_outputDirName + "/";
    ResultSaver::SaveDetail(dir, m_result);
    ResultSaver::SaveDetailAll(dir, m_result);
}
