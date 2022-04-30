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
    const char comma = ',';

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

        std::string BoolStr(bool b) {
            return b ? "true" : "false";
        }

        void WriteBodyResult(std::ofstream& file, const std::vector<SimuResultStep>& stepResult) {
            const std::vector<std::string> headers = {"time_from_launch[s]",
                                                      "elapsed_time[s]",
                                                      "height[m]",
                                                      "velocity[m/s]",
                                                      "airspeed[m/s]",
                                                      "attack_angle[deg]",
                                                      "distance_from_launch_point[m]",
                                                      "latitude",
                                                      "longitude",
                                                      "mass[kg]",
                                                      "Cg_from_nose[m]",
                                                      "Cp_from_nose[m]",
                                                      "Cd",
                                                      "Cna"};

            for (const auto& head : headers) {
                file << Internal::WithComma(head);
            }
            file << "\n";

            for (const auto& step : stepResult) {
                file << Internal::WithComma(step.gen_timeFromLaunch) << Internal::WithComma(step.gen_elapsedTime);

                // important values
                file << Internal::WithComma(step.rocket_pos.z) << Internal::WithComma(step.rocket_velocity.length())
                     << Internal::WithComma(step.rocket_airspeed_b.length())
                     << Internal::WithComma(step.rocket_attackAngle) << Internal::WithComma(step.lenFromLaunchPoint)
                     << Internal::WithComma(step.latitude) << Internal::WithComma(step.longitude);

                // param
                file << Internal::WithComma(step.rocket_mass) << Internal::WithComma(step.rocket_cgLength);

                // aero
                file << Internal::WithComma(step.Cp) << Internal::WithComma(step.Cd) << Internal::WithComma(step.Cna);

                file << "\n";
            }
        }

        void WriteSummaryHeader(std::ofstream& file) {
            const std::vector<std::string> headers = {
                "wind_speed[m/s]", "wind_dir[deg]", "launch_clear_time[s]", "launch_clear_vel[m/s]",
                /*"max_height_time[s]",
                "max_height[m]",
                "max_vel[m/s]",
                "paraopen_time[s]",
                "paraopen_height[m]",
                "paraopen_airspeed[m/s]",
                "terminal_vel[m/s]",
                "terminal_time[s]",
                "max_attack_angle[rad]",
                "max_normal_force[N]",*/
            };

            // write header
            for (const auto& head : headers) {
                file << Internal::WithComma(head);
            }
            file << "\n";
        }

        void WriteSummaryScatter(const std::string& dir, const std::vector<SimuResultSummary>& results) {
            std::ofstream file(dir + "summary.csv");

            WriteSummaryHeader(file);

            for (const auto& result : results) {
                file << Internal::WithComma(result.windSpeed) << Internal::WithComma(result.windDirection)
                     << Internal::WithComma(result.launchClearTime)
                     << Internal::WithComma(result.launchClearVelocity.length());
                file << "\n";
            }

            file.close();
        }

        void WriteSummaryDetail(const std::string& dir, const SimuResultSummary& result) {
            std::ofstream file(dir + "summary.csv");

            WriteSummaryHeader(file);

            file << Internal::WithComma(result.windSpeed) << Internal::WithComma(result.windDirection)
                 << Internal::WithComma(result.launchClearTime)
                 << Internal::WithComma(result.launchClearVelocity.length());
            file << "\n";

            file.close();
        }
    }

    void SaveScatter(const std::string& dir, const std::vector<SimuResultSummary>& result) {
        // Save summary
        Internal::WriteSummaryScatter(dir, result);

        // Save detail
    }

    void SaveDetail(const std::string& dir, const SimuResultSummary& result) {
        // Save summary
        Internal::WriteSummaryDetail(dir, result);

        // Save detail

        // write rocket spec and contidions(wind speed, wind direction)
        /*{
            std::ofstream file(dir + "contidion.csv");

            file << Internal::WithComma("wind_speed[m/s]") << Internal::DoubleToString(result->windSpeed, 2) << "\n";
            file << Internal::WithComma("wind_direction[deg]") << Internal::DoubleToString(result->windDirection, 2)
                 << "\n";

            file.close();
        }*/

        // write time-series data of all bodies
        {
            const auto bodyCount = result.bodyResults.size();
            for (size_t i = 0; i < bodyCount; i++) {
                const std::string fileName = "detail_body" + std::to_string(i + 1);
                const std::string path     = dir + fileName + ".csv";
                std::ofstream file(path);
                Internal::WriteBodyResult(file, result.bodyResults[i].steps);
                file.close();
            }
        }
    }
}
