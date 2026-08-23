// ------------------------------------------------
// DetailSimulator.hppの実装
// ------------------------------------------------

#include "DetailSimulator.hpp"

#include "core/Simulation.hpp"
#include "result/ResultSaver.hpp"

bool DetailSimulator::simulate() {
    m_result = Simulation::Run(m_input);
    return true;
}

void DetailSimulator::saveResult() {
    const std::string dir = "result/" + m_outputDirName + "/";
    ResultSaver::SaveDetail(dir, m_result, m_mapData, m_applicationSettings.result.precision);
}
