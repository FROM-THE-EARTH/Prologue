// ------------------------------------------------
// Gnuplot.hppの実装
// ------------------------------------------------

#include "Gnuplot.hpp"

#include <app/AppSetting.hpp>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "GnuplotController.hpp"
#include "app/CommandLine.hpp"
#include "env/Map.hpp"
#include "misc/Platform.hpp"
#include "solver/Solver.hpp"

#if PLATFORM_WINDOWS
#define GNUPLOT_TERMINAL "windows"
#define PAUSE_COMMAND "pause"
#elif PLATFORM_MACOS
#define GNUPLOT_TERMINAL "qt"
#define PAUSE_COMMAND "read -n1 -r -p \"Press any key to continue...\" key"
#else
#define GNUPLOT_TERMINAL "x11"
#define PAUSE_COMMAND "read -rsp $'Press any key to continue...\n'"
#endif

enum class PlotDimension { Dimension2D, Dimension3D };

struct GnuplotRange {
    double xMin, xMax;
    double yMin, yMax;
};

namespace Gnuplot {
    GnuplotController gnuplot;

    std::string dirname;
    std::string command;
    MapData mapData;
    PlotDimension dimension;
    GnuplotRange range;
    size_t bodyCount = 0;
    size_t plotCount = 0;  // plot group
    double windSpeed = 0.0, windDirection = 0.0;

    namespace Internal {
        void Initialize() {
            // reset
            gnuplot.send("reset");

            // Ticslevel
            gnuplot.send("set ticslevel 0");

            // grid
            gnuplot.send("set grid");

            // legends box
            gnuplot.send("set key opaque box");

            // Label
            gnuplot.send("set xlabel \"Distance[m](+: East, -: West)\"");
            gnuplot.send("set ylabel \"Distance[m](+: North, -: South)\"");
            gnuplot.send("set zlabel \"Height[m]\"");

            // title, xyz ratio, range
            switch (dimension) {
            case PlotDimension::Dimension2D:
                gnuplot.send("set size ratio -1");
                gnuplot.send("set xrange[%f:%f]", range.xMin, range.xMax);
                gnuplot.send("set yrange[%f:%f]", range.yMin, range.yMax);
                command = "plot ";
                break;

            case PlotDimension::Dimension3D:
                gnuplot.send("set title \"Wind: %.2f[m/s] %.2f[deg]\"", windSpeed, windDirection);
                gnuplot.send("set view equal xyz");
                command = "splot ";
                break;
            }
            gnuplot.send("set title font \"MS Gothic, 20\"");
        }

        void PlotLaunchPoint() {
            std::ofstream file("result/" + dirname + "/data/launch_point.txt");
            file << 0 << " " << 0 << " " << 0 << std::endl;
            file.close();
        }

        void Show2D() {
            command += "\"data/launch_point.txt\" ";
            command += "title \"LaunchPoint\" ";
            command += "with ";
            command += "points ";
            command += "pt 7 ps 2";
            command += "lc rgb \"yellow\"";
            command += ", ";

            const auto windCount = plotCount / bodyCount;
            for (size_t i = 0; i < windCount; i++) {
                for (size_t j = 0; j < bodyCount; j++) {
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

                    if (i != plotCount - 1) {
                        command += ", ";
                    }
                }
            }

            gnuplot.send(command);
        }

        void Show3D() {
            command += "\"data/launch_point.txt\" ";
            command += "title \"LaunchPoint\" ";
            command += "with ";
            command += "points ";
            command += "pt 7 ps 2";
            command += "lc rgb \"yellow\"";
            command += ", ";

            for (size_t i = 0; i < plotCount; i++) {
                const std::string filename = "body" + std::to_string(i) + "_trajectory";
                command += "\"data/" + filename + ".txt\" ";
                command += "title \"Body No." + std::to_string(i + 1) + "\" ";
                command += "with ";
                command += "lines ";
                command += "lw 2";

                if (i != plotCount - 1) {
                    command += ", ";
                }
            }

            gnuplot.send(command);
        }

        void CalcRange(const SimuResultSummary& result, bool init, bool end) {
            if (init) {
                range = {result.bodyResults[0].steps[0].rocket_pos.x,
                         result.bodyResults[0].steps[0].rocket_pos.x,
                         result.bodyResults[0].steps[0].rocket_pos.y,
                         result.bodyResults[0].steps[0].rocket_pos.y};
            }

            for (const auto& bodyResult : result.bodyResults) {
                for (const auto& step : bodyResult.steps) {
                    if (range.xMin > step.rocket_pos.x) {
                        range.xMin = step.rocket_pos.x;
                    }
                    if (range.xMax < step.rocket_pos.x) {
                        range.xMax = step.rocket_pos.x;
                    }
                    if (range.yMin > step.rocket_pos.y) {
                        range.yMin = step.rocket_pos.y;
                    }
                    if (range.yMax < step.rocket_pos.y) {
                        range.yMax = step.rocket_pos.y;
                    }
                }
            }

            if (end) {
                const double deltaX = (range.xMax - range.xMin) * 0.2;
                const double deltaY = (range.yMax - range.yMin) * 0.2;

                range.xMin -= deltaX;
                range.xMax += deltaX;
                range.yMin -= deltaY;
                range.yMax += deltaY;
            }
        }

        void SetMap() {
            if (dimension == PlotDimension::Dimension2D) {
                command += "\"../../input/map/";
                command += mapData.imageFileName + "\" ";
                command += "binary filetype=png ";
                command += "dx=" + std::to_string(mapData.gnuplot_dx) + " ";
                command += "dy=" + std::to_string(mapData.gnuplot_dy) + " ";
                command += "origin=(" + std::to_string(mapData.gnuplot_origin_x) + ", "
                           + std::to_string(mapData.gnuplot_origin_y) + ") ";
                command += "with rgbimage notitle,";
            }
        }

        void SaveAsPNG() {
            gnuplot.send("set terminal pngcairo size 1920, 1440");

            gnuplot.send("set output \"result.png\"");

            gnuplot.send("load \"result.plt\"");

            gnuplot.send("set terminal " GNUPLOT_TERMINAL);

            gnuplot.send("set output");
        }
    }

    void Initialize(const char* _dirname, MapData _mapData) {
        mapData = _mapData;
        dirname = _dirname;

        const std::filesystem::path dir = "result/" + dirname + "/data";
        std::filesystem::remove_all(dir);
        std::filesystem::create_directory(dir);
    }

    void Plot(const SimuResultSummary& result) {
        Internal::PlotLaunchPoint();

        plotCount = result.bodyResults.size();

        for (size_t i = 0; i < plotCount; i++) {
            const std::string filename = "body" + std::to_string(i) + "_trajectory";
            std::ofstream file("result/" + dirname + "/data/" + filename + ".txt");
            for (const auto& step : result.bodyResults[i].steps) {
                file << step.rocket_pos.x << " " << step.rocket_pos.y << " " << step.rocket_pos.z << std::endl;
            }
            file.close();
        }

        windSpeed     = result.windSpeed;
        windDirection = result.windDirection;

        dimension = PlotDimension::Dimension3D;
    }

    void Plot(const std::vector<SimuResultSummary>& results) {
        Internal::PlotLaunchPoint();

        bodyCount = results[0].bodyResults.size();

        for (size_t i = 0; i < results.size(); i++) {
            Internal::CalcRange(results[i], i == 0, i == results.size() - 1);
        }

        const size_t directions = static_cast<size_t>(std::ceil(360 / AppSetting::Simulation::windDirInterval));
        const size_t winds =
            static_cast<size_t>(AppSetting::Simulation::windSpeedMax - AppSetting::Simulation::windSpeedMin + 1);

        plotCount = winds * bodyCount;

        for (size_t i = 0; i < winds; i++) {          // winds
            for (size_t j = 0; j < bodyCount; j++) {  // bodies
                const std::string filename = "body" + std::to_string(j) + "_landings" + std::to_string(i);
                const std::string fname    = "result/" + dirname + "/data/" + filename + ".txt ";
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

        dimension = PlotDimension::Dimension2D;
    }

    void Show() {
        if (std::filesystem::exists("result.plt")) {
            gnuplot.open();
            gnuplot.send("load \"result.plt\"");
        } else {
            if (!gnuplot.open()) {
                std::cout << "Could not open gnuplot" << std::endl;
                return;
            }

            Internal::Initialize();

            gnuplot.send("cd \"result/%s\"", dirname.c_str());

            switch (dimension) {
            case PlotDimension::Dimension2D:
                Internal::SetMap();
                Internal::Show2D();
                break;

            case PlotDimension::Dimension3D:
                Internal::Show3D();
                break;
            }
        }

        gnuplot.flush();

        system(PAUSE_COMMAND);

        gnuplot.send(" exit");

        gnuplot.close();
    }

    void Save() {
        if (!gnuplot.open()) {
            std::cout << "Could not open gnuplot" << std::endl;
            return;
        }

        Internal::Initialize();

        gnuplot.send("cd \"result/%s\"", dirname.c_str());

        switch (dimension) {
        case PlotDimension::Dimension2D:
            Internal::SetMap();
            Internal::Show2D();
            break;

        case PlotDimension::Dimension3D:
            Internal::Show3D();
            break;
        }

        gnuplot.flush();

        gnuplot.send("save \"result.plt\"");

        gnuplot.send("replot");

        gnuplot.send("set terminal " GNUPLOT_TERMINAL);

        gnuplot.flush();

        if (dimension == PlotDimension::Dimension2D) {
            Internal::SaveAsPNG();
        }

        gnuplot.send(" exit");

        gnuplot.close();
    }
}
