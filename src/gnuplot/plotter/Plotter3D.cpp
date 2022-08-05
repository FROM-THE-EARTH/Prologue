// ------------------------------------------------
// Plotter3D.hppの実装
// ------------------------------------------------

#include "Plotter3D.hpp"

#include <filesystem>
#include <fstream>

void Plotter3D::saveResult(const SimuResultSummary& result) {
    saveLaunchPoint();

    for (size_t i = 0; i < m_plotCount; i++) {
        const std::string filename = "body" + std::to_string(i) + "_trajectory";
        std::ofstream file(m_resultDirectory + "data/" + filename + ".txt");
        for (const auto& step : result.bodyResults[i].steps) {
            file << step.rocket_pos.x << " " << step.rocket_pos.y << " " << step.rocket_pos.z << std::endl;
        }
        file.close();
    }
}

void Plotter3D::initializePlot() const {
    IPlotter::initializePlot();

    m_gnuplot.send("set title \"Wind: %.2f[m/s] %.2f[deg]\"", m_windSpeed, m_windDirection);
    m_gnuplot.send("set view equal xyz");
}

void Plotter3D::plot() const {
    // 3D plot
    std::string command = "splot ";

    // Launch point
    command += "\"data/launch_point.txt\" ";
    command += "title \"LaunchPoint\" ";
    command += "with ";
    command += "points ";
    command += "pt 7 ps 2";
    command += "lc rgb \"yellow\"";
    command += ", ";

    // Trajectory
    for (size_t i = 0; i < m_plotCount; i++) {
        const std::string filename = "body" + std::to_string(i) + "_trajectory";
        command += "\"data/" + filename + ".txt\" ";
        command += "title \"Body No." + std::to_string(i + 1) + "\" ";
        command += "with ";
        command += "lines ";
        command += "lw 2";

        if (i != m_plotCount - 1) {
            command += ", ";
        }
    }

    m_gnuplot.send(command);
}
