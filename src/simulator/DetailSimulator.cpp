// ------------------------------------------------
// DetailSimulator.hppの実装
// ------------------------------------------------

#include "DetailSimulator.hpp"

#include "gnuplot/plotter/Plotter3D.hpp"
#include "result/ResultSaver.hpp"

bool DetailSimulator::simulate() {
    Solver solver(m_mapData,
                  m_rocketType,
                  m_setting.trajectoryMode,
                  m_setting.detachType,
                  m_setting.detachTime,
                  m_environment,
                  m_rocketSpec);

    if (auto resultLogger = solver.solve(m_setting.windSpeed, m_setting.windDirection); resultLogger) {
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
    auto plotter = Plotter3D(
        "result/" + m_outputDirName + "/", m_result.bodyResults.size(), m_setting.windSpeed, m_setting.windDirection);
    plotter.saveResult(m_result);
    plotter.savePlot();
}
