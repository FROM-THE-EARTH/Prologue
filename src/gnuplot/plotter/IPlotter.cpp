// ------------------------------------------------
// IPlotter.hppの実装
// ------------------------------------------------

#include "IPlotter.hpp"

#include <filesystem>
#include <fstream>

#include "app/CommandLine.hpp"

IPlotter::IPlotter(std::string_view resultDirectory, size_t bodyCount, size_t plotCount) :
    m_resultDirectory(resultDirectory), m_bodyCount(bodyCount), m_plotCount(plotCount) {
    // Remove previous plot
    std::filesystem::remove(m_resultDirectory + "result.plt");

    // Remove data directory and create empty
    const std::filesystem::path dir = m_resultDirectory + "data";
    std::filesystem::remove_all(dir);
    std::filesystem::create_directory(dir);
}

void IPlotter::savePlot() {
    if (!m_gnuplot.open()) {
        CommandLine::PrintInfo(PrintInfoType::Error, "Could not open gnuplot.");
        return;
    }

    initializePlot();

    m_gnuplot.send("cd \"%s\"", m_resultDirectory.c_str());

    plot();

    m_gnuplot.flush();

    m_gnuplot.send("save \"result.plt\"");

    m_gnuplot.send("replot");

    m_gnuplot.send("set terminal " GNUPLOT_TERMINAL);

    m_gnuplot.flush();

    m_gnuplot.close();
}

void IPlotter::initializePlot() const {
    // Reset gnuplot
    m_gnuplot.send("reset");

    // Ticslevel
    m_gnuplot.send("set ticslevel 0");

    // Grid
    m_gnuplot.send("set grid");

    // Legends box
    m_gnuplot.send("set key opaque box");

    // Label
    m_gnuplot.send("set xlabel \"Distance[m](+: East, -: West)\"");
    m_gnuplot.send("set ylabel \"Distance[m](+: North, -: South)\"");
    m_gnuplot.send("set zlabel \"Height[m]\"");

    // Font
    m_gnuplot.send("set title font \"MS Gothic, 20\"");
}

void IPlotter::saveLaunchPoint() const {
    std::ofstream file(m_resultDirectory + "data/launch_point.txt");
    file << 0 << " " << 0 << " " << 0 << std::endl;
    file.close();
}
