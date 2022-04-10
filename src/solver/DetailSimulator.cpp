#include "DetailSimulator.hpp"

#include "utils/ResultSaver.hpp"

bool DetailSimulator::simulate() {
    Solver solver(
        m_dt, m_mapData, m_rocketType, m_trajectoryMode, m_detachType, m_detachTime, m_environment, m_rocketSpec);

    if (m_result = solver.solve(m_windSpeed, m_windDirection); m_result) {
        m_result->organize();
        return true;
    } else {
        return false;
    }
}

void DetailSimulator::saveResult() {
    const std::string dir = "result/" + m_outputDirName + "/";
    ResultSaver::SaveDetail(dir, m_result);
}
