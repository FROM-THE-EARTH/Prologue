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

        void WriteTimeSeries(std::vector<std::ofstream>& files, const SimuResult& result) {
            const size_t bodyCount                 = files.size();
            const std::vector<std::string> headers = {"time_from_launch[s]",
                                                      "elapsed_time[s]",
                                                      "launch_clear?",
                                                      "detect_peak?",
                                                      "parachute_open?",
                                                      "height[m]",
                                                      "velocity[m/s]",
                                                      "airspeed[m/s]",
                                                      "attack_angle[deg]",
                                                      "distance_from_launch_point[m]",
                                                      "latitude",
                                                      "longitude",
                                                      "mass[kg]",
                                                      "body length[m]",
                                                      "Cp_from_nose[m]",
                                                      "Cd",
                                                      "Cna"};

            for (auto& file : files) {
                for (const auto& head : headers) {
                    file << Internal::WithComma(head);
                }
                file << "\n";
            }

            for (const auto& rocket : result.timeSeriesRockets) {
                for (size_t i = 0; i < bodyCount; i++) {
                    auto& body = rocket.bodies[i];
                    files[i] << Internal::WithComma(rocket.timeFromLaunch) << Internal::WithComma(body.elapsedTime);

                    // status
                    files[i] << Internal::WithComma(Internal::BoolStr(rocket.launchClear))
                             << Internal::WithComma(Internal::BoolStr(body.detectPeak))
                             << Internal::WithComma(Internal::BoolStr(body.parachuteOpened));

                    // important values
                    files[i] << Internal::WithComma(body.pos.z) << Internal::WithComma(body.velocity.length())
                             << Internal::WithComma(body.airSpeed_b.length()) << Internal::WithComma(body.attackAngle)
                             << Internal::WithComma(body.lenFromLaunchPoint) << Internal::WithComma(body.latitude)
                             << Internal::WithComma(body.longitude);

                    // param
                    files[i] << Internal::WithComma(body.mass) << Internal::WithComma(body.reflLength);

                    // aero
                    files[i] << Internal::WithComma(body.aeroCoef.Cp) << Internal::WithComma(body.aeroCoef.Cd)
                             << Internal::WithComma(body.aeroCoef.Cna);

                    files[i] << "\n";
                }
            }
        }
    }

    void SaveScatter(const std::string& dir, const std::vector<std::shared_ptr<SimuResult>>& result) {
        const std::vector<std::string> headers = {
            "wind_speed[m/s]",
            "wind_dir[deg]",
            "launch_vel[m/s]",
            "max_height[m]",
            "max_height_time[s]",
            "max_vel[m/s]",
            "paraopen_time[s]",
            "paraopen_height[m]",
            "paraopen_airspeed[m/s]",
            "terminal_vel[m/s]",
            "terminal_time[s]",
            "max_attack_angle[rad]",
            "max_normal_force[N]",
        };

        std::ofstream file(dir + "summary.csv");
        for (const auto& head : headers) {
            file << Internal::WithComma(head);
        }
        file << "\n";

        for (const auto& r : result) {
            file << Internal::WithComma(r->windSpeed) << Internal::WithComma(r->windDirection)
                 << Internal::WithComma(r->launchClearVelocity) << Internal::WithComma(r->maxHeight)
                 << Internal::WithComma(r->detectPeakTime) << Internal::WithComma(r->maxVelocity)
                 << Internal::WithComma(r->timeAtParaOpened) << Internal::WithComma(r->heightAtParaOpened)
                 << Internal::WithComma(r->airVelAtParaOpened) << Internal::WithComma(r->terminalVelocity)
                 << Internal::WithComma(r->maxAttackAngle) << Internal::WithComma(r->maxNormalForce);
            file << "\n";
        }

        file.close();
    }

    void SaveDetail(const std::string& dir, const std::shared_ptr<SimuResult>& result) {
        // write rocket spec and contidions(wind speed, wind direction)
        {
            std::ofstream file(dir + "contidion.csv");

            file << Internal::WithComma("wind_speed[m/s]") << Internal::DoubleToString(result->windSpeed, 2) << "\n";
            file << Internal::WithComma("wind_direction[deg]") << Internal::DoubleToString(result->windDirection, 2)
                 << "\n";

            file.close();
        }

        // write time-series data of all bodies
        {
            const size_t bodyCount = result->timeSeriesRockets[result->timeSeriesRockets.size() - 1].bodies.size();

            std::vector<std::ofstream> files(bodyCount);
            for (size_t i = 0; i < bodyCount; i++) {
                const std::string fileName = "detail_body" + std::to_string(i + 1);
                const std::string path     = dir + fileName + ".csv";
                files[i].open(path);
            }

            Internal::WriteTimeSeries(files, *result);

            for (auto& file : files) {
                file.close();
            }
        }
    }
}
