// ------------------------------------------------
// DetailSimulator.hppの実装
// ------------------------------------------------

#include "DetailSimulator.hpp"

#include "gnuplot/plotter/Plotter3D.hpp"
#include "result/ResultSaver.hpp"

bool DetailSimulator::simulate() {
    Solver solver(
        m_dt, m_mapData, m_rocketType, m_trajectoryMode, m_detachType, m_detachTime, m_environment, *m_rocketSpec);

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

void DetailSimulator::plotToGnuplot() {
    auto plotter =
        Plotter3D("result/" + m_outputDirName + "/", m_result.bodyResults.size(), m_windSpeed, m_windDirection);
    plotter.saveResult(m_result);
    plotter.savePlot();
}
