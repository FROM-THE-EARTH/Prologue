#include "ResultSaver.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>

#include "app/CommandLine.hpp"
#include "solver/Solver.hpp"

#if defined(_WIN32) || defined(WIN32)
#define SPRINTF sprintf_s
#else
#define SPRINTF snprintf
#endif

namespace ResultSaver {

    std::ofstream f;
    const char comma            = ',';
    const std::string headers[] = {"wind_speed",
                                   "wind_dir",
                                   "launch_vel",
                                   "max_height",
                                   "max_height_time",
                                   "max_vel",
                                   "time_paraopen",
                                   "height_paraopen",
                                   "airspeed_paraopen",
                                   "terminal_vel",
                                   "terminal_time",
                                   "max_attack_angle",
                                   "max_normal_force",
                                   "distance_from_launch",
                                   "latitude longitude"};
    const int headerNum         = sizeof(headers) / sizeof(*headers);
    const int dataStartingRow   = 6;

    namespace Internal {

        std::string DoubleToString(double d, int precision) {
            std::stringstream stream;
            stream << std::fixed << std::setprecision(precision) << d;
            return stream.str();
        }

        std::string WithComma(int i) {
            return std::to_string(i) + comma;
        }

        std::string WithComma(double d, int precision = 2) {
            return DoubleToString(d, precision) + comma;
        }

        std::string WithComma(const std::string str) {
            return str + comma;
        }

        void OpenFile(const std::string& filepath, size_t rows) {
            f.open(filepath);

            if (!f) {
                const std::string s = "Failed to open : " + std::string(filepath);
                CommandLine::PrintInfo(PrintInfoType::Error, s.c_str());
                return;
            }

            // unit description
            f << "length=[m] | velocity=[m/s] | angle=[deg]\n";

            // header
            f << comma;
            for (auto& h : headers) {
                f << h << comma;
            }
            f << '\n';

            // min
            f << "min" << comma;
            char column = 'B';
            for (size_t i = 0; i < headerNum; i++) {
                char range[16];
                SPRINTF(range,
                        16,
                        "%c%d:%c%d",
                        column,
                        dataStartingRow,
                        column,
                        static_cast<int>(rows + dataStartingRow - 1));
                char min[32] = "";
                SPRINTF(min, 32, "=MIN(%s)", range);

                column++;
                f << min << comma;
            }
            f << '\n';

            // max
            f << "max" << comma;
            column = 'B';
            for (size_t i = 0; i < headerNum; i++) {
                char range[16];
                SPRINTF(range,
                        16,
                        "%c%d:%c%d",
                        column,
                        dataStartingRow,
                        column,
                        static_cast<int>(rows + dataStartingRow - 1));
                char max[32] = "";
                SPRINTF(max, 32, "=MAX(%s)", range);

                column++;
                f << max << comma;
            }
            f << '\n';

            // empty row
            f << '\n';
        }

        void WriteLine(const SolvedResult& result, size_t rocketIndex) {
            const std::string latitude_longitude =
                Internal::DoubleToString(result.rocket[rocketIndex].latitude, 8) + "N "
                + Internal::DoubleToString(result.rocket[rocketIndex].longitude, 8) + "E";

            f << comma << Internal::WithComma(result.windSpeed) << Internal::WithComma(result.windDirection)
              << Internal::WithComma(result.launchClearVelocity_)
              << Internal::WithComma(result.rocket[rocketIndex].maxHeight)
              << Internal::WithComma(result.rocket[rocketIndex].detectPeakTime)
              << Internal::WithComma(result.rocket[rocketIndex].maxVelocity)
              << Internal::WithComma(result.rocket[rocketIndex].timeAtParaOpened)
              << Internal::WithComma(result.rocket[rocketIndex].heightAtParaOpened)
              << Internal::WithComma(result.rocket[rocketIndex].airVelAtParaOpened)
              << Internal::WithComma(result.rocket[rocketIndex].terminalVelocity)
              << Internal::WithComma(result.rocket[rocketIndex].terminalTime)
              << Internal::WithComma(result.rocket[rocketIndex].maxAttackAngle)
              << Internal::WithComma(result.rocket[rocketIndex].maxNormalForce)
              << Internal::WithComma(result.rocket[rocketIndex].lenFromLaunchPoint) << latitude_longitude;
            f << '\n';
        }

        void Close() {
            f.close();
        }

        void WriteAll(const std::string& filepath, const std::vector<Rocket>& flightData) {
            std::ofstream file(filepath);

            file << Internal::WithComma("Time[s]") << Internal::WithComma("Height[m]")
                 << Internal::WithComma("Velocity[m/s]") << Internal::WithComma("AirSpeed[m/s]")
                 << Internal::WithComma("AttackAngle[deg]");
            file << '\n';

            for (const auto& data : flightData) {
                file << Internal::WithComma(data.elapsedTime) << Internal::WithComma(data.pos.z)
                     << Internal::WithComma(data.velocity.length()) << Internal::WithComma(data.airSpeed_b.length())
                     << Internal::WithComma(data.attackAngle * 180. / Constant::PI);
                file << '\n';
            }

            file.close();
        }
    }

    void SaveScatter(const std::string& dir, const std::vector<SolvedResult>& result) {
        // Write special values
        for (size_t i = 0; i < result[0].rocket.size(); i++) {
            const std::string fileName = "summary_rocket" + std::to_string(i + 1);
            const std::string path     = dir + fileName + ".csv";

            Internal::OpenFile(path, result.size());

            for (auto& res : result) {
                Internal::WriteLine(res, i);
            }

            Internal::Close();
        }
    }

    void SaveDetail(const std::string& dir, const SolvedResult& result) {
        // Write special values
        for (size_t i = 0; i < result.rocket.size(); i++) {
            const std::string fileName = "summary_rocket" + std::to_string(i + 1);
            const std::string path     = dir + fileName + ".csv";

            Internal::OpenFile(path, 1);

            Internal::WriteLine(result, i);

            Internal::Close();
        }
    }

    void SaveScatterAll(const std::string& dir, const std::vector<SolvedResult>& result) {
        for (const auto& r : result) {
            const std::string windCondition =
                "_" + Internal::DoubleToString(r.windSpeed, 1) + "-" + Internal::DoubleToString(r.windDirection, 1);

            for (size_t i = 0; i < r.rocket.size(); i++) {
                const std::string fileName = "detail_rocket" + std::to_string(i + 1) + windCondition;
                const std::string path     = dir + fileName + ".csv";

                Internal::WriteAll(path, r.rocket[i].flightData);
            }
        }
    }

    void SaveDetailAll(const std::string& dir, const SolvedResult& result) {
        const std::string windCondition = "[" + Internal::DoubleToString(result.windSpeed, 1) + "-"
                                          + Internal::DoubleToString(result.windDirection, 1) + "]";

        for (size_t i = 0; i < result.rocket.size(); i++) {
            const std::string fileName = "detail_rocket" + std::to_string(i + 1) + windCondition;
            const std::string path     = dir + fileName + ".csv";

            Internal::WriteAll(path, result.rocket[i].flightData);
        }
    }
}
