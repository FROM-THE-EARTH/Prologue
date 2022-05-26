#include "DetailSimulator.hpp"

#include "result/ResultSaver.hpp"

bool DetailSimulator::simulate() {
    Solver solver(
        m_dt, m_mapData, m_rocketType, m_trajectoryMode, m_detachType, m_detachTime, m_environment, m_rocketSpec);

    if (auto resultLogger = solver.solve(m_windSpeed, m_windDirection); resultLogger) {
        resultLogger->organize();
        m_result = resultLogger->getResult();
        return true;
    } else {
        return false;
    }
}

void DetailSimulator::saveResult() {
    const std::string dir = "result/" + m_outputDirName + "/";
    ResultSaver::SaveDetail(dir, m_result);
}
