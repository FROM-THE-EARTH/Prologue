// ------------------------------------------------
// Plotter2D.hppの実装
// ------------------------------------------------

#include "Plotter2D.hpp"

#include <filesystem>
#include <fstream>

#include "app/AppSetting.hpp"

Plotter2D::Plotter2D(std::string_view resultDirectory, size_t bodyCount, const MapData& mapData) :
    IPlotter(
        resultDirectory,
        bodyCount,
        bodyCount
            * static_cast<size_t>(AppSetting::Simulation::windSpeedMax - AppSetting::Simulation::windSpeedMin + 1)),
    m_mapData(mapData) {}

void Plotter2D::saveResult(const std::vector<SimuResultSummary>& results) {
    saveLaunchPoint();

    calcPlotRange(results);

    const size_t directions = static_cast<size_t>(std::ceil(360 / AppSetting::Simulation::windDirInterval));
    const size_t winds =
        static_cast<size_t>(AppSetting::Simulation::windSpeedMax - AppSetting::Simulation::windSpeedMin + 1);

    for (size_t i = 0; i < winds; i++) {            // winds
        for (size_t j = 0; j < m_bodyCount; j++) {  // bodies
            const std::string filename = "body" + std::to_string(j) + "_landings" + std::to_string(i);
            const std::string fname    = m_resultDirectory + "data/" + filename + ".txt ";
            std::ofstream file(fname.c_str());

            for (size_t k = 0; k < directions; k++) {  // directions
                const size_t n     = directions * i + k;
                const Vector3D pos = results[n].bodyResults[j].steps[0].rocket_pos;
                file << pos.x << " " << pos.y << std::endl;
            }

            // add initial point to be circle
            const size_t ini = directions * i;
            file << results[ini].bodyResults[j].steps[0].rocket_pos.x << " "
                 << results[ini].bodyResults[j].steps[0].rocket_pos.y << std::endl;

            file.close();
        }
    }
}

void Plotter2D::savePlotAsPng() {
    // Wait for saving the plot
    const auto plotFile = m_resultDirectory + "result.plt";
    while (!std::filesystem::exists(plotFile)) {
    }

    if (m_gnuplot.open()) {
        m_gnuplot.send("cd \"%s\"", m_resultDirectory.c_str());
        m_gnuplot.send("set terminal pngcairo size 1920, 1440");
        m_gnuplot.send("set output \"result.png\"");
        m_gnuplot.send("load \"result.plt\"");
        m_gnuplot.send("set terminal " GNUPLOT_TERMINAL);
        m_gnuplot.send("set output");
        m_gnuplot.close();
    }
}

void Plotter2D::initializePlot() const {
    IPlotter::initializePlot();

    m_gnuplot.send("set size ratio -1");
    m_gnuplot.send("set xrange[%f:%f]", m_range.xMin, m_range.xMax);
    m_gnuplot.send("set yrange[%f:%f]", m_range.yMin, m_range.yMax);
}

void Plotter2D::plot() const {
    // 2D plot
    std::string command = "plot ";

    // Map
    command += "\"../../input/map/";
    command += m_mapData.imageFileName + "\" ";
    command += "binary filetype=png ";
    command += "dx=" + std::to_string(m_mapData.gnuplot_dx) + " ";
    command += "dy=" + std::to_string(m_mapData.gnuplot_dy) + " ";
    command += "origin=(" + std::to_string(m_mapData.gnuplot_origin_x) + ", "
               + std::to_string(m_mapData.gnuplot_origin_y) + ") ";
    command += "with rgbimage notitle,";

    // Launch point
    command += "\"data/launch_point.txt\" ";
    command += "title \"LaunchPoint\" ";
    command += "with ";
    command += "points ";
    command += "pt 7 ps 2";
    command += "lc rgb \"yellow\"";
    command += ", ";

    // Landing point for each wind and body
    const auto windCount = m_plotCount / m_bodyCount;
    for (size_t i = 0; i < windCount; i++) {
        for (size_t j = 0; j < m_bodyCount; j++) {
            const std::string filename = "body" + std::to_string(j) + "_landings" + std::to_string(i);
            command += "\"data/" + filename + ".txt\" ";

            if (i == 0) {
                command += "title \"Body No." + std::to_string(j + 1) + "\" ";
            } else {
                command += "notitle ";
            }
            command += "with ";
            command += "lines ";
            command += "lw 2 ";

            if (j == 0) {
                command += "lc rgb \"red\"";
            } else {
                command += "lc rgb \"blue\"";
            }

            if (i != m_plotCount - 1) {
                command += ", ";
            }
        }
    }

    m_gnuplot.send(command);
}

void Plotter2D::calcPlotRange(const std::vector<SimuResultSummary>& results) {
    m_range = {0};

    for (size_t i = 0; i < results.size(); i++) {
        const auto& result = results[i];

        if (i == 0) {
            m_range = {
                result.bodyResults[0].steps[0].rocket_pos.x,
                result.bodyResults[0].steps[0].rocket_pos.x,
                result.bodyResults[0].steps[0].rocket_pos.y,
                result.bodyResults[0].steps[0].rocket_pos.y,
            };
        }

        for (const auto& bodyResult : result.bodyResults) {
            for (const auto& step : bodyResult.steps) {
                if (m_range.xMin > step.rocket_pos.x) {
                    m_range.xMin = step.rocket_pos.x;
                }
                if (m_range.xMax < step.rocket_pos.x) {
                    m_range.xMax = step.rocket_pos.x;
                }
                if (m_range.yMin > step.rocket_pos.y) {
                    m_range.yMin = step.rocket_pos.y;
                }
                if (m_range.yMax < step.rocket_pos.y) {
                    m_range.yMax = step.rocket_pos.y;
                }
            }
        }

        if (i == results.size() - 1) {
            const double deltaX = (m_range.xMax - m_range.xMin) * 0.2;
            const double deltaY = (m_range.yMax - m_range.yMin) * 0.2;

            m_range.xMin -= deltaX;
            m_range.xMax += deltaX;
            m_range.yMin -= deltaY;
            m_range.yMax += deltaY;
        }
    }
}
