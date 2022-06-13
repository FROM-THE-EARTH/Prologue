#include "Gnuplot.hpp"

#include <app/AppSetting.hpp>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

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
    FILE* p;

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
            fprintf(p, "reset\n");

            // Ticslevel
            fprintf(p, "set ticslevel 0\n");

            // grid
            fprintf(p, "set grid\n");

            // legends box
            fprintf(p, "set key opaque box\n");

            // Label
            fprintf(p, "set xlabel \"Distance[m](+: East, -: West)\"\n");
            fprintf(p, "set ylabel \"Distance[m](+: North, -: South)\"\n");
            fprintf(p, "set zlabel \"Height[m]\"\n");

            // title, xyz ratio, range
            switch (dimension) {
            case PlotDimension::Dimension2D:
                fprintf(p, "set size ratio -1\n");
                fprintf(p, "set xrange[%f:%f]\n", range.xMin, range.xMax);
                fprintf(p, "set yrange[%f:%f]\n", range.yMin, range.yMax);
                command = "plot ";
                break;

            case PlotDimension::Dimension3D:
                fprintf(p, "set title \"Wind: %.2f[m/s] %.2f[deg]\"\n", windSpeed, windDirection);
                fprintf(p, "set view equal xyz\n");
                command = "splot ";
                break;
            }
            fprintf(p, "set title font \"MS Gothic, 20\"\n");
        }

        void PlotLaunchPoint() {
            std::ofstream file("result/" + dirname + "/data/launch.txt");
            file << 0 << " " << 0 << " " << 0 << std::endl;
            file.close();
        }

        void Show2D() {
            command += "\"data/launch.txt\" ";
            command += "title \"LaunchPoint\" ";
            command += "with ";
            command += "points ";
            command += "pt 7 ps 2";
            command += "lc rgb \"yellow\"";
            command += ", ";

            for (size_t i = 0; i < plotCount; i++) {
                command += "\"data/result" + std::to_string(i) + ".txt\" ";
                size_t k = i / bodyCount;
                if (k == 0) {
                    command += "title \"Body No." + std::to_string(i % bodyCount + 1) + "\" ";
                } else {
                    command += "notitle ";
                }
                command += "with ";
                command += "lines ";
                command += "lw 2 ";

                if (i % bodyCount == 0) {
                    command += "lc rgb \"red\"";
                } else {
                    command += "lc rgb \"blue\"";
                }

                if (i != plotCount - 1)
                    command += ", ";
            }

            command += "\n";
            fprintf(p, "%s", command.c_str());
        }

        void Show3D() {
            command += "\"data/launch.txt\" ";
            command += "title \"LaunchPoint\" ";
            command += "with ";
            command += "points ";
            command += "pt 7 ps 2";
            command += "lc rgb \"yellow\"";
            command += ", ";

            for (size_t i = 0; i < plotCount; i++) {
                command += "\"data/result" + std::to_string(i) + ".txt\" ";
                command += "title \"Body No." + std::to_string(i + 1) + "\" ";
                command += "with ";
                command += "lines ";
                command += "lw 2";

                if (i != plotCount - 1)
                    command += ", ";
            }

            command += "\n";
            fprintf(p, "%s", command.c_str());
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
            fprintf(p, "set terminal pngcairo size 1920, 1440\n");

            fprintf(p, "set output \"result.png\"\n");

            fprintf(p, "load \"result.plt\"\n");

            fprintf(p, "set terminal " GNUPLOT_TERMINAL "\n");

            fprintf(p, "set output\n");
        }
    }

    void Initialize(const char* _dirname, MapData _mapData) {
        mapData = _mapData;
        dirname = _dirname;

        const std::filesystem::path dir = "result/" + dirname + "/data";
        if (!std::filesystem::exists(dir)) {
            std::filesystem::create_directory(dir);
        }
    }

    void Plot(const SimuResultSummary& result) {
        Internal::PlotLaunchPoint();

        plotCount = result.bodyResults.size();

        for (size_t i = 0; i < plotCount; i++) {
            std::ofstream file("result/" + dirname + "/data/result" + std::to_string(i) + ".txt");
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
                const std::string fname =
                    "result/" + dirname + "/data/result" + std::to_string(i * bodyCount + j) + ".txt";
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
            p = POPEN("gnuplot", "w");
            fprintf(p, "load \"result.plt\"\n");
        } else {
            p = POPEN("gnuplot", "w");
            if (p == nullptr) {
                std::cout << "Could not open gnuplot" << std::endl;
                return;
            }

            Internal::Initialize();

            fprintf(p, "cd \"result/%s\"\n", dirname.c_str());

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

        fflush(p);

        system(PAUSE_COMMAND);

        fprintf(p, " exit\n");

        PCLOSE(p);
    }

    void Save() {
        p = POPEN("gnuplot", "w");
        if (p == nullptr) {
            std::cout << "Could not open gnuplot" << std::endl;
            return;
        }

        Internal::Initialize();

        fprintf(p, "cd \"result/%s\"\n", dirname.c_str());

        switch (dimension) {
        case PlotDimension::Dimension2D:
            Internal::SetMap();
            Internal::Show2D();
            break;

        case PlotDimension::Dimension3D:
            Internal::Show3D();
            break;
        }

        fflush(p);

        fprintf(p, "save \"result.plt\"\n");

        fprintf(p, "replot\n");

        fprintf(p, "set terminal " GNUPLOT_TERMINAL "\n");

        fflush(p);

        if (dimension == PlotDimension::Dimension2D) {
            Internal::SaveAsPNG();
        }

        fprintf(p, "exit\n");

        PCLOSE(p);
    }
}
