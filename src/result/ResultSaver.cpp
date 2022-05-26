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
    constexpr char comma = ',';

    const std::vector<std::string> headerDetail = {
        // general
        "time_from_launch[s]",
        "elapsed_time[s]",
        // boolean
        "launch_clear?",
        "combusting?",
        "para_opened?",
        // air
        "air_density[kg/m3]",
        "gravity[m/s2]",
        "pressure[Pa]",
        "temperature[C]",
        "wind[m/s]",
        // body
        "mass[kg]",
        "Cg_from_nose[m]",
        "inertia moment pitch & yaw[kg*m2]",
        "inertia moment roll[kg*m2]",
        "attack angle[deg]",
        "height[m]",
        "velocity[m/s]",
        "airspeed[m/s]",
        "normal_force[N]",
        "Cnp",
        "Cny",
        "Cmqp",
        "Cmqy",
        "Cp_from_nose[m]",
        "Cd",
        "Cna",
        // position
        "latitude",
        "longitude",
        "length_from_launch_point[m]",
        // calculated
        "Fst[%]",
        "dynamic_pressure[Pa]"};

    const std::vector<std::string> headerSummary = {"wind_speed[m/s]",
                                                    "wind_dir[deg]",
                                                    "launch_clear_time[s]",
                                                    "launch_clear_vel[m/s]",
                                                    "max_height[m]",
                                                    "max_height_time[s]",
                                                    "max_velocity[m/s]",
                                                    "max_airspeed[m/s]",
                                                    "max_normal_force_rising[N]"};

    namespace Internal {
        std::string DoubleToString(double d, int precision) {
            std::stringstream stream;
            stream << std::fixed << std::setprecision(precision) << d;
            return stream.str();
        }

        std::string BoolToString(bool b) {
            return b ? "true" : "false";
        }

        std::string WithComma(bool b) {
            return BoolToString(b) + comma;
        }

        std::string WithComma(int i) {
            return std::to_string(i) + comma;
        }

        std::string WithComma(double d, int precision = 2) {
            return DoubleToString(d, precision) + comma;
        }

        std::string WithComma(const std::string& str) {
            return str + comma;
        }

        std::string WithComma(const Vector3D& v) {
            return "\"(" + DoubleToString(v.x, 2) + ',' + DoubleToString(v.y, 2) + ',' + DoubleToString(v.z, 2) + ")\""
                   + comma;
        }

        void WriteBodyResult(std::ofstream& file, const std::vector<SimuResultStep>& stepResult) {
            for (const auto& head : headerDetail) {
                file << Internal::WithComma(head);
            }
            file << "\n";

            for (const auto& step : stepResult) {
                // general
                file << Internal::WithComma(step.gen_timeFromLaunch) << Internal::WithComma(step.gen_elapsedTime);

                // boolean
                file << Internal::WithComma(step.launchClear) << Internal::WithComma(step.combusting)
                     << Internal::WithComma(step.parachuteOpened);

                // air
                file << Internal::WithComma(step.air_density) << Internal::WithComma(step.air_gravity)
                     << Internal::WithComma(step.air_pressure) << Internal::WithComma(step.air_temperature)
                     << Internal::WithComma(step.air_wind);

                // body
                file << Internal::WithComma(step.rocket_mass) << Internal::WithComma(step.rocket_cgLength)
                     << Internal::WithComma(step.rocket_iyz) << Internal::WithComma(step.rocket_ix)
                     << Internal::WithComma(step.rocket_attackAngle) << Internal::WithComma(step.rocket_pos.z)
                     << Internal::WithComma(step.rocket_velocity.length())
                     << Internal::WithComma(step.rocket_airspeed_b.length())
                     << Internal::WithComma(sqrt(step.rocket_force_b.y * step.rocket_force_b.y
                                                 + step.rocket_force_b.z * step.rocket_force_b.z))
                     << Internal::WithComma(step.Cnp) << Internal::WithComma(step.Cny) << Internal::WithComma(step.Cmqp)
                     << Internal::WithComma(step.Cmqy) << Internal::WithComma(step.Cp) << Internal::WithComma(step.Cd)
                     << Internal::WithComma(step.Cna);

                // position
                file << Internal::WithComma(step.latitude) << Internal::WithComma(step.longitude)
                     << Internal::WithComma(step.lenFromLaunchPoint);

                // calculated
                file << Internal::WithComma(step.Fst) << Internal::WithComma(step.dynamicPressure);

                file << "\n";
            }
        }

        void WriteSummaryHeader(std::ofstream& file) {
            // write header
            for (const auto& head : headerSummary) {
                file << Internal::WithComma(head);
            }
            file << "\n";
        }

        void WriteSummary(std::ofstream& file, const SimuResultSummary& result) {
            file << Internal::WithComma(result.windSpeed) << Internal::WithComma(result.windDirection)
                 << Internal::WithComma(result.launchClearTime)
                 << Internal::WithComma(result.launchClearVelocity.length()) << Internal::WithComma(result.maxHeight)
                 << Internal::WithComma(result.detectPeakTime) << Internal::WithComma(result.maxVelocity)
                 << Internal::WithComma(result.maxAirspeed) << Internal::WithComma(result.maxNormalForceDuringRising);
            file << "\n";
        }

        void WriteSummaryScatter(const std::string& dir, const std::vector<SimuResultSummary>& results) {
            std::ofstream file(dir + "summary.csv");

            WriteSummaryHeader(file);

            for (const auto& result : results) {
                WriteSummary(file, result);
            }

            file.close();
        }

        void WriteSummaryDetail(const std::string& dir, const SimuResultSummary& result) {
            std::ofstream file(dir + "summary.csv");

            WriteSummaryHeader(file);

            WriteSummary(file, result);

            file.close();
        }
    }

    void SaveScatter(const std::string& dir, const std::vector<SimuResultSummary>& result) {
        // Save summary
        Internal::WriteSummaryScatter(dir, result);
    }

    void SaveDetail(const std::string& dir, const SimuResultSummary& result) {
        // Save summary
        Internal::WriteSummaryDetail(dir, result);

        // Save detail
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
