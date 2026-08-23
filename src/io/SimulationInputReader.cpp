// ------------------------------------------------
// SimulationInputReader.hppの実装
// ------------------------------------------------

#include "SimulationInputReader.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cmath>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// disable all warnings of fast-cpp-csv-parser
#pragma warning(push, 0)
#include <fast-cpp-csv-parser/csv.h>
#pragma warning(pop)

#include "misc/Constant.hpp"
#include "utils/JsonUtils.hpp"

namespace SimulationInputReader {
    struct Document::Impl {
        std::filesystem::path specificationFile;
        boost::property_tree::ptree specJson;

        explicit Impl(const std::filesystem::path& file) : specificationFile(file) {
            boost::property_tree::read_json(specificationFile.string(), specJson);
        }
    };

    namespace {
        constexpr size_t AvailableBodyCount                = 3;
        constexpr const char* BodyList[AvailableBodyCount] = {"rocket1", "rocket2", "rocket3"};

        std::vector<ThrustData> ReadThrustData(const std::string& filename) {
            if (filename.empty()) {
                return {};
            }

            const std::filesystem::path thrustFile = std::filesystem::path("input/thrust") / filename;
            std::ifstream stream(thrustFile);
            if (!stream.is_open()) {
                throw std::runtime_error{"Failed to open thrust data file: " + thrustFile.string()};
            }

            while (true) {
                const auto next = stream.peek();
                if (next == std::char_traits<char>::eof()) {
                    throw std::runtime_error{"No thrust data found in: " + thrustFile.string()};
                }
                if ('0' <= next && next <= '9') break;

                std::string headerLine;
                std::getline(stream, headerLine);
            }

            std::vector<ThrustData> thrustData;
            while (true) {
                ThrustData data;
                if (!(stream >> data.time >> data.thrust)) break;
                thrustData.emplace_back(data);
            }

            if (!stream.eof()) {
                throw std::runtime_error{"Invalid thrust data in: " + thrustFile.string()};
            }
            if (thrustData.empty()) {
                throw std::runtime_error{"No thrust data found in: " + thrustFile.string()};
            }
            if (thrustData.front().time != 0.0) {
                thrustData.insert(thrustData.begin(), ThrustData{0.0, 0.0});
            }
            if (thrustData.size() < 2) {
                throw std::runtime_error{"Thrust data must contain at least two points: " + thrustFile.string()};
            }
            for (size_t i = 0; i < thrustData.size(); i++) {
                if (!std::isfinite(thrustData[i].time) || !std::isfinite(thrustData[i].thrust)) {
                    throw std::runtime_error{"Thrust data must contain only finite values: " + thrustFile.string()};
                }
                if (i > 0 && thrustData[i - 1].time >= thrustData[i].time) {
                    throw std::runtime_error{"Thrust data time must be strictly increasing: " + thrustFile.string()};
                }
            }

            return thrustData;
        }

        std::optional<std::vector<AeroCoefficientData>> ReadAeroCoefficientData(
            const std::string& filename,
            std::vector<InputIO::Diagnostic>& diagnostics) {
            if (filename.empty()) {
                return std::nullopt;
            }

            try {
                io::CSVReader<7> csv("input/aero_coef/" + filename);
                csv.read_header(io::ignore_extra_column,
                                "air_speed[m/s]",
                                "Cp_from_nose[m]",
                                "Cp_a[m/rad]",
                                "Cd_i",
                                "Cd_f",
                                "Cd_a2[/rad^2]",
                                "Cna");

                std::vector<AeroCoefficientData> data;
                AeroCoefficientData point;
                while (csv.read_row(
                    point.airspeed, point.Cp, point.Cp_a, point.Cd_i, point.Cd_f, point.Cd_a2, point.Cna)) {
                    data.emplace_back(point);
                }
                return data;
            } catch (const std::exception& e) {
                diagnostics.push_back(
                    {.level = InputIO::DiagnosticLevel::Error,
                     .lines = {"While reading aero coefficient file.", e.what()}});
                return std::nullopt;
            }
        }

        Environment ReadEnvironment(const boost::property_tree::ptree& specJson) {
            return Environment(JsonUtils::GetValue<std::string>(specJson, "environment.place"),
                               JsonUtils::GetValueExc<double>(specJson, "environment.rail_len"),
                               JsonUtils::GetValueExc<double>(specJson, "environment.rail_azi"),
                               JsonUtils::GetValueExc<double>(specJson, "environment.rail_elev"));
        }

        BodySpecification ReadBodySpecification(const boost::property_tree::ptree& pt,
                                                 size_t index,
                                                 std::vector<InputIO::Diagnostic>& diagnostics) {
            const std::string key = BodyList[index];
            BodySpecification spec;

            spec.length     = JsonUtils::GetValueExc<double>(pt, key + ".ref_len");
            spec.diameter   = JsonUtils::GetValueExc<double>(pt, key + ".diam");
            spec.bottomArea = spec.diameter * spec.diameter * 0.25 * Constant::PI;

            spec.CGLengthInitial = JsonUtils::GetValueExc<double>(pt, key + ".CGlen_i");
            spec.CGLengthFinal   = JsonUtils::GetValueExc<double>(pt, key + ".CGlen_f");

            spec.massInitial = JsonUtils::GetValueExc<double>(pt, key + ".mass_i");
            spec.massFinal   = JsonUtils::GetValueExc<double>(pt, key + ".mass_f");

            spec.rollingMomentInertiaInitial = JsonUtils::GetValueExc<double>(pt, key + ".Iyz_i");
            spec.rollingMomentInertiaFinal   = JsonUtils::GetValueExc<double>(pt, key + ".Iyz_f");

            spec.Cmq = JsonUtils::GetValueExc<double>(pt, key + ".Cmq");

            if (pt.get_child_optional(key + ".parachutes")) {
                for (const auto& child : pt.get_child(key + ".parachutes")) {
                    const double openingTime =
                        JsonUtils::GetValueWithDefault<double>(child.second, "op_time_from_launch", -1.0);
                    const double delayTime =
                        JsonUtils::GetValueWithDefault<double>(child.second, "op_time_from_peak", -1.0);
                    const double openingHeight =
                        JsonUtils::GetValueWithDefault<double>(child.second, "op_height", -1.0);

                    const unsigned char openingType =
                        (openingTime >= 0.0 ? PARACHUTE_OPENING_TYPE_FIXED_TIME : 0x00)
                        | (delayTime >= 0.0 ? PARACHUTE_OPENING_TYPE_TIME_FROM_DETECT_PEAK : 0x00)
                        | (openingHeight >= 0.0 ? PARACHUTE_OPENING_TYPE_DETECT_PEAK : 0x00);

                    const double CdS = JsonUtils::GetValueExc<double>(child.second, "CdS");

                    spec.parachutes.emplace_back(Parachute{.openingType   = openingType,
                                                           .openingTime   = openingTime,
                                                           .delayTime     = delayTime,
                                                           .openingHeight = openingHeight,
                                                           .CdS           = CdS});
                }
            }

            spec.engine = Engine(ReadThrustData(JsonUtils::GetValue<std::string>(pt, key + ".motor_file")));
            if (const auto pressure = JsonUtils::GetValueOpt<double>(pt, key + ".thrust_measured_pressure");
                pressure.has_value()) {
                spec.engine.setThrustMeasuredPressure(pressure.value());
            }
            if (const auto diameter = JsonUtils::GetValueOpt<double>(pt, key + ".engine_nozzle_diameter");
                diameter.has_value()) {
                spec.engine.setNozzleDiameter(diameter.value());
            }

            const auto aeroCoefficientData =
                ReadAeroCoefficientData(JsonUtils::GetValue<std::string>(pt, key + ".aero_coef_file"), diagnostics);
            if (aeroCoefficientData.has_value()) {
                spec.aeroCoefStorage = AeroCoefficientStorage(aeroCoefficientData.value(), true);
                diagnostics.push_back(
                    {.level = InputIO::DiagnosticLevel::Information,
                     .lines = {"Rocket: " + key, "Aero coefficients are set from CSV"}});
            } else {
                diagnostics.push_back(
                    {.level = InputIO::DiagnosticLevel::Information,
                     .lines = {"Rocket: " + key, "Aero coefficients are set from JSON"}});
                spec.aeroCoefStorage.init(JsonUtils::GetValueExc<double>(pt, key + ".CPlen"),
                                          JsonUtils::GetValue<double>(pt, key + ".CP_alpha"),
                                          JsonUtils::GetValueExc<double>(pt, key + ".Cd_i"),
                                          JsonUtils::GetValueExc<double>(pt, key + ".Cd_f"),
                                          JsonUtils::GetValue<double>(pt, key + ".Cd_alpha2"),
                                          JsonUtils::GetValueExc<double>(pt, key + ".Cna"));
            }

            try {
                for (const auto& child : pt.get_child(key + ".transitions")) {
                    spec.transitions.emplace_back(
                        Transition{.time = JsonUtils::GetValueExc<double>(child.second, "time"),
                                   .mass = JsonUtils::GetValueExc<double>(child.second, "mass"),
                                   .Cd   = JsonUtils::GetValueExc<double>(child.second, "Cd")});
                }
            } catch (...) {
            }

            return spec;
        }

        RocketSpecification ReadRocketSpecification(const boost::property_tree::ptree& specJson,
                                                     std::vector<InputIO::Diagnostic>& diagnostics) {
            const bool isMultipleRocket = JsonUtils::Exist(specJson, BodyList[1]);
            std::vector<BodySpecification> bodySpecs;

            size_t index = 0;
            do {
                bodySpecs.emplace_back(ReadBodySpecification(specJson, index, diagnostics));
                index++;
            } while (isMultipleRocket && index < AvailableBodyCount);

            std::vector<SeparationSpecification> separations;
            if (isMultipleRocket) {
                separations.emplace_back(
                    SeparationSpecification{.sourceBodyIndex = 0, .productBodyIndices = {1, 2}});
            }

            return RocketSpecification(std::move(bodySpecs), std::move(separations));
        }
    }

    Document::Document(const std::filesystem::path& specificationFile) :
        m_impl(std::make_unique<Impl>(specificationFile)) {}

    Document::~Document() = default;

    Document::Document(Document&&) noexcept = default;

    Document& Document::operator=(Document&&) noexcept = default;

    std::string Document::specificationName() const {
        return m_impl->specificationFile.stem().string();
    }

    bool Document::isMultipleRocket() const {
        return JsonUtils::Exist(m_impl->specJson, BodyList[1]);
    }

    InputIO::ReadResult<SimulationInput> Document::toSimulationInput() const {
        std::vector<InputIO::Diagnostic> diagnostics;
        SimulationInput input{
            .environment = ReadEnvironment(m_impl->specJson),
            .rocket = ReadRocketSpecification(m_impl->specJson, diagnostics),
        };
        return {.value = std::move(input), .diagnostics = std::move(diagnostics)};
    }
}
