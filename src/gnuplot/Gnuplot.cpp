#include "Gnuplot.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "app/CommandLine.hpp"
#include "env/Map.hpp"
#include "solver/Solver.hpp"

#if defined(_WIN32) || defined(WIN32)
#define POPEN _popen
#define PCLOSE _pclose
#define GNUPLOT_TERMINAL "windows"
#define PAUSE_COMMAND "pause"
#elif __APPLE__
#define POPEN popen
#define PCLOSE pclose
#define GNUPLOT_TERMINAL "qt"
#define PAUSE_COMMAND "read -n1 -r -p \"Press any key to continue...\" key"
#else
#define POPEN popen
#define PCLOSE pclose
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

        void CalcRange(const ResultRocket& result, bool init, bool end) {
            if (init) {
                range = {result.bodies[0].timeSeriesBodies[0].pos.x,
                         result.bodies[0].timeSeriesBodies[0].pos.x,
                         result.bodies[0].timeSeriesBodies[0].pos.y,
                         result.bodies[0].timeSeriesBodies[0].pos.y};
            }

            for (const auto& body : result.bodies) {
                for (const auto& b : body.timeSeriesBodies) {
                    if (range.xMin > b.pos.x) {
                        range.xMin = b.pos.x;
                    }
                    if (range.xMax < b.pos.x) {
                        range.xMax = b.pos.x;
                    }
                    if (range.yMin > b.pos.y) {
                        range.yMin = b.pos.y;
                    }
                    if (range.yMax < b.pos.y) {
                        range.yMax = b.pos.y;
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

    void Plot(const ResultRocket& result) {
        Internal::PlotLaunchPoint();

        plotCount = result.bodies.size();

        for (size_t i = 0; i < plotCount; i++) {
            const std::string fname = "result/" + dirname + "/data/result" + std::to_string(i) + ".txt";
            std::ofstream file(fname.c_str());
            for (const auto& body : result.bodies[i].timeSeriesBodies) {
                file << body.pos.x << " " << body.pos.y << " " << body.pos.z << std::endl;
            }
            file.close();
        }

        windSpeed     = result.windSpeed;
        windDirection = result.windDirection;

        dimension = PlotDimension::Dimension3D;
    }

    void Plot(const std::vector<ResultRocket>& result) {
        Internal::PlotLaunchPoint();

        bodyCount = result[0].bodies.size();

        for (size_t i = 0; i < result.size(); i++) {
            if (result[i].windDirection == 0.0) {
                plotCount++;
            }
            Internal::CalcRange(result[i], i == 0, i == result.size() - 1);
        }

        const size_t directions = result.size() / plotCount;
        const size_t winds      = plotCount;
        plotCount *= bodyCount;

        for (size_t i = 0; i < winds; i++) {          // winds
            for (size_t j = 0; j < bodyCount; j++) {  // bodies
                const std::string fname =
                    "result/" + dirname + "/data/result" + std::to_string(i * bodyCount + j) + ".txt";
                std::ofstream file(fname.c_str());

                for (size_t k = 0; k < directions; k++) {  // directions
                    const size_t n     = directions * i + k;
                    const Vector3D pos = result[n].bodies[j].timeSeriesBodies[0].pos;
                    file << pos.x << " " << pos.y << std::endl;
                }

                // add initial point to be circle
                const size_t ini = directions * i;
                file << result[ini].bodies[j].timeSeriesBodies[0].pos.x << " "
                     << result[ini].bodies[j].timeSeriesBodies[0].pos.y << std::endl;

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
