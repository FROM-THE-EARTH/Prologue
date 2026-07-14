// ------------------------------------------------
// ResultSaver.hppの実装
// ------------------------------------------------

#include "ResultSaver.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>

#include <boost/progress.hpp>

#include <kml/dom.h>
#include <kml/base/file.h>
#include <kml/engine.h>

#include "app/AppSetting.hpp"
#include "app/CommandLine.hpp"
#include "misc/Platform.hpp"
#include "solver/Solver.hpp"

#define WITH_COMMA(value) value << ','

std::string Bits2String(const std::vector<bool>& bits) {
	std::string result ;
	result.reserve(bits.size() + 2);
	result.push_back('"');
	for (auto it = bits.end(); it != bits.begin(); --it) {
		result += (*(it - 1) ? "1" : "0");
	}
	result.push_back('"');
	return result;
}

namespace ResultSaver {
	using kmldom::KmlFactory;
	using kmldom::CoordinatesPtr;
	using kmldom::LinearRingPtr;
	using kmldom::OuterBoundaryIsPtr;
	using kmldom::PointPtr;
	using kmldom::PolygonPtr;
	using kmldom::PlacemarkPtr;
	using kmldom::DocumentPtr;
	using kmldom::KmlPtr;
	using kmldom::StylePtr;
	using kmldom::LineStylePtr;
	using kmldom::PolyStylePtr;

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
        "attack angle[rad]",
        "altitude[m]",
        "velocity[m/s]",
        "airspeed[m/s]",
        "accel[m/s2]",
        "longitudinal accel[m/s2]",
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
                                                    "max_altitude_airspeed[m/s]",
                                                    "max_dynamic_pressure[N/m2]",
                                                    "max_dynamic_pressure_time[s]",
                                                    "max_dynamic_pressure_altitude[m]",
                                                    "max_airspeed[m/s]",
                                                    "max_airspeed_time[s]",
                                                    "max_longitudinal_accel[m/s2]",
                                                    "max_longitudinal_accel_time[s]",
                                                    "first_parachute_open_time[s]",
                                                    "first_parachute_open_altitude[m]",
                                                    "first_parachute_open_airspeed[m/s]"
                                                    };

    namespace Internal {
        std::ofstream OpenResultCSV(const std::string& path) {
            std::ofstream file(path);
            file << std::fixed << std::setprecision(AppSetting::Result::precision);
            return file;
        }

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
                file << WITH_COMMA(step.launchClear) << WITH_COMMA(step.combusting) << WITH_COMMA(Bits2String(step.parachuteOpened));

                // air
                file << WITH_COMMA(step.air_density) << WITH_COMMA(step.air_gravity) << WITH_COMMA(step.air_pressure)
                     << WITH_COMMA(step.air_temperature) << WITH_COMMA(step.air_wind.x) << WITH_COMMA(step.air_wind.y)
                     << WITH_COMMA(step.air_wind.z);

                // body
                file << WITH_COMMA(step.rocket_mass) << WITH_COMMA(step.rocket_cgLength) << WITH_COMMA(step.rocket_iyz)
                     << WITH_COMMA(step.rocket_ix) << WITH_COMMA(step.rocket_attackAngle)
                     << WITH_COMMA(step.rocket_pos.z) << WITH_COMMA(step.rocket_velocity.length())
                     << WITH_COMMA(step.rocket_airspeed_b.length())
                     << WITH_COMMA(step.rocket_force_b.length() / step.rocket_mass)
                     << WITH_COMMA(step.rocket_force_b.x / step.rocket_mass)
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

        void WriteSummaryHeader(std::ofstream& file, size_t bodyCount) {
            // write header
            for (const auto& head : headerSummary) {
                file << WITH_COMMA(head);
            }

            // additional header
            for (size_t i = 0; i < bodyCount; i++) {
                const std::string body = "body" + std::to_string(i + 1);
                file << WITH_COMMA(body + "_final_latitude") << WITH_COMMA(body + "_final_longitude");
            }

            file << "\n";
        }

        void WriteSummary(std::ofstream& file, const SimuResultSummary& result, size_t bodyCount) {
            file << WITH_COMMA(result.windSpeed) << WITH_COMMA(result.windDirection)
                 << WITH_COMMA(result.launchClearTime) << WITH_COMMA(result.launchClearVelocity.length())
                 << WITH_COMMA(result.maxAltitude) << WITH_COMMA(result.detectPeakTime) << WITH_COMMA(result.airspeedAtPeak)
                 << WITH_COMMA(result.maxDynamicPressureDuringRising) << WITH_COMMA(result.maxDynamicPressureTime) << WITH_COMMA(result.maxDynamicPressureAltitude)
                 << WITH_COMMA(result.maxAirspeed) << WITH_COMMA(result.maxAirspeedTime)
                 << WITH_COMMA(result.maxLongitudinalAccel) << WITH_COMMA(result.maxLongitudinalAccelTime)
                 << WITH_COMMA(result.firstParachuteOpenTime)
                 << WITH_COMMA(result.firstParachuteOpenAltitude) << WITH_COMMA(result.firstParachuteOpenAirspeed);

            for (size_t i = 0; i < bodyCount; i++) {
                if (i < result.bodyFinalPositions.size()) {
                    file << WITH_COMMA(result.bodyFinalPositions[i].latitude)
                         << WITH_COMMA(result.bodyFinalPositions[i].longitude);
                } else {
                    file << WITH_COMMA(0.0) << WITH_COMMA(0.0);
                }
            }

            file << "\n";
        }

		void WriteScatterKml(const std::string& dir, const std::vector<SimuResultSummary>& results) {
			if (results.empty()) {
				return;
			}

			std::map<double, std::list<SimuResultSummary>> windSpeedMap;
			size_t bodyCount = 0;
			for (const auto& result : results) {
				windSpeedMap[result.windSpeed].push_back(result);
				bodyCount = std::max(bodyCount, result.bodyFinalPositions.size());
			}
			for (auto& [windSpeed, resultList] : windSpeedMap) {
				resultList.sort([](const SimuResultSummary& a, const SimuResultSummary& b) {
					return a.windDirection < b.windDirection;
				});
			}

			KmlFactory* factory = KmlFactory::GetFactory();
			const size_t firstBodyIndex = bodyCount > 1 ? 1 : 0;
			for (size_t bodyIndex = firstBodyIndex; bodyIndex < bodyCount; bodyIndex++) {
				DocumentPtr doc = factory->CreateDocument();
				doc->set_name("Scatter Result");

				StylePtr style = factory->CreateStyle();
				style->set_id("scatter_style");
				LineStylePtr lineStyle = factory->CreateLineStyle();
				lineStyle->set_width(2.0);
				lineStyle->set_color(kmlbase::Color32(0xff000000)); // black
				style->set_linestyle(lineStyle);
				PolyStylePtr polyStyle = factory->CreatePolyStyle();
				polyStyle->set_fill(false);
				polyStyle->set_outline(true);
				style->set_polystyle(polyStyle);
				doc->add_styleselector(style);

				for (const auto& [windSpeed, resultList] : windSpeedMap) {
					std::vector<BodyFinalPosition> positions;
					for (const auto& result : resultList) {
						if (bodyIndex >= result.bodyFinalPositions.size()) {
							continue;
						}

						const auto& finalPos = result.bodyFinalPositions[bodyIndex];
						positions.push_back(finalPos);

						CoordinatesPtr pointCoordinates = factory->CreateCoordinates();
						pointCoordinates->add_latlngalt(finalPos.latitude, finalPos.longitude, 0.0);
						PointPtr point = factory->CreatePoint();
						point->set_coordinates(pointCoordinates);

						PlacemarkPtr pointPlacemark = factory->CreatePlacemark();
						pointPlacemark->set_name("Wind Speed " + std::to_string(windSpeed)
							+ " m/s, Wind Direction " + std::to_string(result.windDirection) + " deg");
						pointPlacemark->set_geometry(point);
						doc->add_feature(pointPlacemark);
					}

					std::vector<BodyFinalPosition> distinctPositions;
					for (const auto& position : positions) {
						const auto found = std::find_if(distinctPositions.begin(), distinctPositions.end(),
							[&position](const BodyFinalPosition& other) {
								return position.latitude == other.latitude && position.longitude == other.longitude;
							});
						if (found == distinctPositions.end()) {
							distinctPositions.push_back(position);
						}
					}
					if (distinctPositions.size() < 3) {
						continue;
					}

					CoordinatesPtr polygonCoordinates = factory->CreateCoordinates();
					for (const auto& position : positions) {
						polygonCoordinates->add_latlngalt(position.latitude, position.longitude, 0.0);
					}
					const auto& firstPosition = positions.front();
					polygonCoordinates->add_latlngalt(firstPosition.latitude, firstPosition.longitude, 0.0);

					LinearRingPtr linearRing = factory->CreateLinearRing();
					linearRing->set_tessellate(true);
					linearRing->set_coordinates(polygonCoordinates);
					OuterBoundaryIsPtr outerBoundary = factory->CreateOuterBoundaryIs();
					outerBoundary->set_linearring(linearRing);
					PolygonPtr polygon = factory->CreatePolygon();
					polygon->set_outerboundaryis(outerBoundary);

					PlacemarkPtr polygonPlacemark = factory->CreatePlacemark();
					polygonPlacemark->set_name("Wind Speed " + std::to_string(windSpeed) + " m/s");
					polygonPlacemark->set_geometry(polygon);
					polygonPlacemark->set_styleurl("#scatter_style");
					doc->add_feature(polygonPlacemark);
				}

				KmlPtr kml = factory->CreateKml();
				kml->set_feature(doc);
				const std::string kmlPath = dir + "scatter_body" + std::to_string(bodyIndex + 1) + ".kml";
				kmlbase::File::WriteStringToFile(kmldom::SerializePretty(kml), kmlPath);
			}
		}

        void WriteSummaryScatter(const std::string& dir, const std::vector<SimuResultSummary>& results) {
            std::ofstream file = Internal::OpenResultCSV(dir + "summary.csv");

            size_t bodyCount = 0;
            for (const auto& result : results) {
                bodyCount = bodyCount < result.bodyFinalPositions.size() ? result.bodyFinalPositions.size() : bodyCount;
            }

            WriteSummaryHeader(file, bodyCount);

            for (const auto& result : results) {
                WriteSummary(file, result, bodyCount);
            }

            file.close();

			// write kml
			Internal::WriteScatterKml(dir, results);
        }

        void WriteSummaryDetail(const std::string& dir, const SimuResultSummary& result) {
            std::ofstream file = Internal::OpenResultCSV(dir + "summary.csv");

            WriteSummaryHeader(file, result.bodyFinalPositions.size());

            WriteSummary(file, result, result.bodyFinalPositions.size());

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
                std::ofstream file         = Internal::OpenResultCSV(path);
                Internal::WriteBodyResult(file, result.bodyResults[i].steps);
                file.close();
            }
        }
    }
}
