#include "ResultSaver.hpp"

#include <boost/progress.hpp>
#include <fstream>

#include "app/CommandLine.hpp"
#include "solver/Solver.hpp"

#if defined(_WIN32) || defined(WIN32)
#define SPRINTF sprintf_s
#else
#define SPRINTF snprintf
#endif

#define WITH_COMMA(value) value << ','

namespace ResultSaver {
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
        "wind_x[m/s]",
        "wind_y[m/s]",
        "wind_z[m/s]",
        // body
        "mass[kg]",
        "Cg_from_nose[m]",
        "inertia moment pitch & yaw[kg*m2]",
        "inertia moment roll[kg*m2]",
        "attack angle[deg]",
        "altitude[m]",
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
        "downrange[m]",
        // calculated
        "Fst[%]",
        "dynamic_pressure[Pa]"};

    const std::vector<std::string> headerSummary = {"wind_speed[m/s]",
                                                    "wind_dir[deg]",
                                                    "launch_clear_time[s]",
                                                    "launch_clear_vel[m/s]",
                                                    "max_altitude[m]",
                                                    "max_altitude_time[s]",
                                                    "max_velocity[m/s]",
                                                    "max_airspeed[m/s]",
                                                    "max_normal_force_rising[N]"};

    namespace Internal {
        void WriteBodyResult(std::ofstream& file, const std::vector<SimuResultStep>& stepResult) {
            for (const auto& head : headerDetail) {
                file << WITH_COMMA(head);
            }
            file << "\n";

            boost::progress_display progress(static_cast<uint32_t>(stepResult.size()));
            for (const auto& step : stepResult) {
                // general
                file << WITH_COMMA(step.gen_timeFromLaunch) << WITH_COMMA(step.gen_elapsedTime);

                // boolean
                file << WITH_COMMA(step.launchClear) << WITH_COMMA(step.combusting) << WITH_COMMA(step.parachuteOpened);

                // air
                file << WITH_COMMA(step.air_density) << WITH_COMMA(step.air_gravity) << WITH_COMMA(step.air_pressure)
                     << WITH_COMMA(step.air_temperature) << WITH_COMMA(step.air_wind.x) << WITH_COMMA(step.air_wind.y)
                     << WITH_COMMA(step.air_wind.z);

                // body
                file << WITH_COMMA(step.rocket_mass) << WITH_COMMA(step.rocket_cgLength) << WITH_COMMA(step.rocket_iyz)
                     << WITH_COMMA(step.rocket_ix) << WITH_COMMA(step.rocket_attackAngle)
                     << WITH_COMMA(step.rocket_pos.z) << WITH_COMMA(step.rocket_velocity.length())
                     << WITH_COMMA(step.rocket_airspeed_b.length())
                     << WITH_COMMA(sqrt(step.rocket_force_b.y * step.rocket_force_b.y
                                        + step.rocket_force_b.z * step.rocket_force_b.z))
                     << WITH_COMMA(step.Cnp) << WITH_COMMA(step.Cny) << WITH_COMMA(step.Cmqp) << WITH_COMMA(step.Cmqy)
                     << WITH_COMMA(step.Cp) << WITH_COMMA(step.Cd) << WITH_COMMA(step.Cna);

                // position
                file << WITH_COMMA(step.latitude) << WITH_COMMA(step.longitude) << WITH_COMMA(step.downrange);

                // calculated
                file << WITH_COMMA(step.Fst) << WITH_COMMA(step.dynamicPressure);

                file << "\n";

                ++progress;
            }
        }

        void WriteSummaryHeader(std::ofstream& file) {
            // write header
            for (const auto& head : headerSummary) {
                file << WITH_COMMA(head);
            }
            file << "\n";
        }

        void WriteSummary(std::ofstream& file, const SimuResultSummary& result) {
            file << WITH_COMMA(result.windSpeed) << WITH_COMMA(result.windDirection)
                 << WITH_COMMA(result.launchClearTime) << WITH_COMMA(result.launchClearVelocity.length())
                 << WITH_COMMA(result.maxAltitude) << WITH_COMMA(result.detectPeakTime)
                 << WITH_COMMA(result.maxVelocity) << WITH_COMMA(result.maxAirspeed)
                 << WITH_COMMA(result.maxNormalForceDuringRising);
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
