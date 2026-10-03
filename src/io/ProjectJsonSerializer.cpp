// ------------------------------------------------
// ProjectJsonSerializer.hppの実装
// ------------------------------------------------

#include "ProjectJsonSerializer.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

#include "utils/JsonUtils.hpp"

namespace ProjectJsonSerializer {
    namespace {
        using boost::property_tree::ptree;

        std::filesystem::path PathFromJson(const std::string& value) {
            return std::filesystem::path(std::u8string(value.begin(), value.end()));
        }

        std::string PathToJson(const std::filesystem::path& value) {
            const auto utf8 = value.generic_u8string();
            return {reinterpret_cast<const char*>(utf8.data()), utf8.size()};
        }

        std::optional<std::filesystem::path> ReadOptionalPath(const ptree& source,
                                                              const std::string& key) {
            const auto value = source.get_optional<std::string>(key);
            if (!value.has_value() || value->empty()) return std::nullopt;
            return PathFromJson(*value);
        }

        project::Parachute ReadParachute(const ptree& source) {
            return {
                .openingTimeFromLaunch = JsonUtils::GetOptional<double>(source, "op_time_from_launch"),
                .openingTimeFromPeak = JsonUtils::GetOptional<double>(source, "op_time_from_peak"),
                .openingHeight = JsonUtils::GetOptional<double>(source, "op_height"),
                .CdS = JsonUtils::GetValueExc<double>(source, "CdS"),
            };
        }

        project::Transition ReadTransition(const ptree& source) {
            return {
                .time = JsonUtils::GetValueExc<double>(source, "time"),
                .mass = JsonUtils::GetValueExc<double>(source, "mass"),
                .Cd = JsonUtils::GetValueExc<double>(source, "Cd"),
            };
        }

        project::Body ReadBody(const ptree& source) {
            project::Body result{
                .length = JsonUtils::GetValueExc<double>(source, "ref_len"),
                .diameter = JsonUtils::GetValueExc<double>(source, "diam"),
                .massInitial = JsonUtils::GetValueExc<double>(source, "mass_i"),
                .massFinal = JsonUtils::GetValueExc<double>(source, "mass_f"),
                .centerOfGravityInitial = JsonUtils::GetValueExc<double>(source, "CGlen_i"),
                .centerOfGravityFinal = JsonUtils::GetValueExc<double>(source, "CGlen_f"),
                .pitchYawMomentOfInertiaInitial = JsonUtils::GetValueExc<double>(source, "Iyz_i"),
                .pitchYawMomentOfInertiaFinal = JsonUtils::GetValueExc<double>(source, "Iyz_f"),
                .pitchDampingMomentCoefficient = JsonUtils::GetValueExc<double>(source, "Cmq"),
            };

            if (const auto parachutes = source.get_child_optional("parachutes")) {
                for (const auto& child : *parachutes) {
                    result.parachutes.emplace_back(ReadParachute(child.second));
                }
            }

            result.engine.thrustFile = ReadOptionalPath(source, "motor_file");
            result.engine.thrustMeasuredPressure =
                JsonUtils::GetValueWithDefault<double>(source, "thrust_measured_pressure", 101325.0);
            result.engine.nozzleDiameter =
                JsonUtils::GetValueWithDefault<double>(source, "engine_nozzle_diameter", 0.0);

            result.aerodynamics.coefficientFile = ReadOptionalPath(source, "aero_coef_file");
            result.aerodynamics.centerOfPressure = JsonUtils::GetValue<double>(source, "CPlen");
            result.aerodynamics.centerOfPressureAlpha = JsonUtils::GetValue<double>(source, "CP_alpha");
            result.aerodynamics.dragCoefficientInitial = JsonUtils::GetValue<double>(source, "Cd_i");
            result.aerodynamics.dragCoefficientFinal = JsonUtils::GetValue<double>(source, "Cd_f");
            result.aerodynamics.dragCoefficientAlphaSquared =
                JsonUtils::GetValue<double>(source, "Cd_alpha2");
            result.aerodynamics.normalForceCoefficient = JsonUtils::GetValue<double>(source, "Cna");

            if (!result.aerodynamics.coefficientFile.has_value()) {
                result.aerodynamics.centerOfPressure = JsonUtils::GetValueExc<double>(source, "CPlen");
                result.aerodynamics.dragCoefficientInitial = JsonUtils::GetValueExc<double>(source, "Cd_i");
                result.aerodynamics.dragCoefficientFinal = JsonUtils::GetValueExc<double>(source, "Cd_f");
                result.aerodynamics.normalForceCoefficient = JsonUtils::GetValueExc<double>(source, "Cna");
            }

            if (const auto transitions = source.get_child_optional("transitions")) {
                for (const auto& child : *transitions) {
                    result.transitions.emplace_back(ReadTransition(child.second));
                }
            }
            return result;
        }

        project::Separation ReadSeparation(const ptree& source) {
            project::Separation result{
                .sourceBodyIndex = JsonUtils::GetValueExc<size_t>(source, "source_body"),
            };
            for (const auto& child : source.get_child("product_bodies")) {
                result.productBodyIndices.emplace_back(child.second.get_value<size_t>());
            }
            return result;
        }

        void Indent(std::ostream& output, int depth) {
            for (int index = 0; index < depth; index++) output << "  ";
        }

        void WriteString(std::ostream& output, const std::string& value) {
            output << '"';
            for (const unsigned char character : value) {
                switch (character) {
                    case '"': output << "\\\""; break;
                    case '\\': output << "\\\\"; break;
                    case '\b': output << "\\b"; break;
                    case '\f': output << "\\f"; break;
                    case '\n': output << "\\n"; break;
                    case '\r': output << "\\r"; break;
                    case '\t': output << "\\t"; break;
                    default:
                        if (character < 0x20) {
                            output << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                                   << static_cast<int>(character) << std::dec << std::setfill(' ');
                        } else {
                            output << static_cast<char>(character);
                        }
                }
            }
            output << '"';
        }

        void WriteDouble(std::ostream& output, double value) {
            if (!std::isfinite(value)) {
                throw std::runtime_error{"Project JSON cannot contain a non-finite number."};
            }
            output << std::setprecision(std::numeric_limits<double>::max_digits10) << value;
        }

        void WriteNamedDouble(std::ostream& output,
                              int depth,
                              const char* name,
                              double value,
                              bool trailingComma = true) {
            Indent(output, depth);
            WriteString(output, name);
            output << ": ";
            WriteDouble(output, value);
            if (trailingComma) output << ',';
            output << '\n';
        }

        void WriteParachutes(std::ostream& output,
                             const std::vector<project::Parachute>& parachutes,
                             int depth) {
            Indent(output, depth);
            output << "\"parachutes\": [";
            if (!parachutes.empty()) output << '\n';
            for (size_t index = 0; index < parachutes.size(); index++) {
                const auto& parachute = parachutes[index];
                Indent(output, depth + 1);
                output << "{\n";
                if (parachute.openingTimeFromLaunch.has_value()) {
                    WriteNamedDouble(output,
                                     depth + 2,
                                     "op_time_from_launch",
                                     *parachute.openingTimeFromLaunch);
                }
                if (parachute.openingTimeFromPeak.has_value()) {
                    WriteNamedDouble(output,
                                     depth + 2,
                                     "op_time_from_peak",
                                     *parachute.openingTimeFromPeak);
                }
                if (parachute.openingHeight.has_value()) {
                    WriteNamedDouble(output, depth + 2, "op_height", *parachute.openingHeight);
                }
                WriteNamedDouble(output, depth + 2, "CdS", parachute.CdS, false);
                Indent(output, depth + 1);
                output << '}';
                if (index + 1 < parachutes.size()) output << ',';
                output << '\n';
            }
            if (!parachutes.empty()) Indent(output, depth);
            output << "],\n";
        }

        void WriteTransitions(std::ostream& output,
                              const std::vector<project::Transition>& transitions,
                              int depth) {
            Indent(output, depth);
            output << "\"transitions\": [";
            if (!transitions.empty()) output << '\n';
            for (size_t index = 0; index < transitions.size(); index++) {
                const auto& transition = transitions[index];
                Indent(output, depth + 1);
                output << "{\n";
                WriteNamedDouble(output, depth + 2, "time", transition.time);
                WriteNamedDouble(output, depth + 2, "mass", transition.mass);
                WriteNamedDouble(output, depth + 2, "Cd", transition.Cd, false);
                Indent(output, depth + 1);
                output << '}';
                if (index + 1 < transitions.size()) output << ',';
                output << '\n';
            }
            if (!transitions.empty()) Indent(output, depth);
            output << "]\n";
        }

        void WriteBody(std::ostream& output, const project::Body& body, int depth) {
            Indent(output, depth);
            output << "{\n";
            WriteNamedDouble(output, depth + 1, "ref_len", body.length);
            WriteNamedDouble(output, depth + 1, "diam", body.diameter);
            WriteNamedDouble(output, depth + 1, "mass_i", body.massInitial);
            WriteNamedDouble(output, depth + 1, "mass_f", body.massFinal);
            WriteNamedDouble(output, depth + 1, "CGlen_i", body.centerOfGravityInitial);
            WriteNamedDouble(output, depth + 1, "CGlen_f", body.centerOfGravityFinal);
            WriteNamedDouble(output, depth + 1, "Iyz_i", body.pitchYawMomentOfInertiaInitial);
            WriteNamedDouble(output, depth + 1, "Iyz_f", body.pitchYawMomentOfInertiaFinal);
            WriteNamedDouble(output, depth + 1, "Cmq", body.pitchDampingMomentCoefficient);
            WriteParachutes(output, body.parachutes, depth + 1);

            if (body.engine.thrustFile.has_value()) {
                Indent(output, depth + 1);
                output << "\"motor_file\": ";
                WriteString(output, PathToJson(*body.engine.thrustFile));
                output << ",\n";
            }
            WriteNamedDouble(output,
                             depth + 1,
                             "thrust_measured_pressure",
                             body.engine.thrustMeasuredPressure);
            WriteNamedDouble(output, depth + 1, "engine_nozzle_diameter", body.engine.nozzleDiameter);

            if (body.aerodynamics.coefficientFile.has_value()) {
                Indent(output, depth + 1);
                output << "\"aero_coef_file\": ";
                WriteString(output, PathToJson(*body.aerodynamics.coefficientFile));
                output << ",\n";
            }
            WriteNamedDouble(output, depth + 1, "CPlen", body.aerodynamics.centerOfPressure);
            WriteNamedDouble(output, depth + 1, "CP_alpha", body.aerodynamics.centerOfPressureAlpha);
            WriteNamedDouble(output, depth + 1, "Cd_i", body.aerodynamics.dragCoefficientInitial);
            WriteNamedDouble(output, depth + 1, "Cd_f", body.aerodynamics.dragCoefficientFinal);
            WriteNamedDouble(output,
                             depth + 1,
                             "Cd_alpha2",
                             body.aerodynamics.dragCoefficientAlphaSquared);
            WriteNamedDouble(output, depth + 1, "Cna", body.aerodynamics.normalForceCoefficient);
            WriteTransitions(output, body.transitions, depth + 1);
            Indent(output, depth);
            output << '}';
        }
    }

    project::Document Load(const std::filesystem::path& file) {
        ptree root;
        std::ifstream input(file, std::ios::binary);
        if (!input.is_open()) {
            throw std::runtime_error{"Failed to open project file: " + file.string()};
        }
        boost::property_tree::read_json(input, root);

        project::Document result;
        result.formatVersion = JsonUtils::GetValueExc<int>(root, "format_version");
        if (result.formatVersion != project::CurrentFormatVersion) {
            throw std::runtime_error{"Unsupported project format version: "
                                     + std::to_string(result.formatVersion)};
        }

        const auto& environment = root.get_child("environment");
        result.environment = {
            .place = JsonUtils::GetValueExc<std::string>(environment, "place"),
            .latitude = JsonUtils::GetValueExc<double>(environment, "latitude"),
            .longitude = JsonUtils::GetValueExc<double>(environment, "longitude"),
            .coordinateZone = JsonUtils::GetValueExc<int>(environment, "zone"),
            .magneticDeclination =
                JsonUtils::GetValueExc<double>(environment, "magnetic_declination"),
            .railLength = JsonUtils::GetValueExc<double>(environment, "rail_len"),
            .railAzimuth = JsonUtils::GetValueExc<double>(environment, "rail_azi"),
            .railElevation = JsonUtils::GetValueExc<double>(environment, "rail_elev"),
        };

        for (const auto& child : root.get_child("bodies")) {
            result.bodies.emplace_back(ReadBody(child.second));
        }
        if (result.bodies.empty()) {
            throw std::runtime_error{"Project must contain at least one body."};
        }

        for (const auto& child : root.get_child("separations")) {
            result.separations.emplace_back(ReadSeparation(child.second));
        }
        return result;
    }

    void Save(const std::filesystem::path& file, const project::Document& document) {
        if (document.formatVersion != project::CurrentFormatVersion) {
            throw std::runtime_error{"Only the current project format can be saved."};
        }
        if (document.bodies.empty()) {
            throw std::runtime_error{"Project must contain at least one body."};
        }

        std::ostringstream serialized;
        serialized << "{\n  \"format_version\": " << document.formatVersion << ",\n";
        serialized << "  \"environment\": {\n    \"place\": ";
        WriteString(serialized, document.environment.place);
        serialized << ",\n";
        WriteNamedDouble(serialized, 2, "latitude", document.environment.latitude);
        WriteNamedDouble(serialized, 2, "longitude", document.environment.longitude);
        Indent(serialized, 2);
        serialized << "\"zone\": " << document.environment.coordinateZone << ",\n";
        WriteNamedDouble(serialized,
                         2,
                         "magnetic_declination",
                         document.environment.magneticDeclination);
        WriteNamedDouble(serialized, 2, "rail_len", document.environment.railLength);
        WriteNamedDouble(serialized, 2, "rail_azi", document.environment.railAzimuth);
        WriteNamedDouble(serialized, 2, "rail_elev", document.environment.railElevation, false);
        serialized << "  },\n  \"bodies\": [\n";
        for (size_t index = 0; index < document.bodies.size(); index++) {
            WriteBody(serialized, document.bodies[index], 2);
            if (index + 1 < document.bodies.size()) serialized << ',';
            serialized << '\n';
        }
        serialized << "  ],\n  \"separations\": [";
        if (!document.separations.empty()) serialized << '\n';
        for (size_t index = 0; index < document.separations.size(); index++) {
            const auto& separation = document.separations[index];
            serialized << "    {\n      \"source_body\": " << separation.sourceBodyIndex
                       << ",\n      \"product_bodies\": [";
            for (size_t productIndex = 0;
                 productIndex < separation.productBodyIndices.size();
                 productIndex++) {
                if (productIndex > 0) serialized << ", ";
                serialized << separation.productBodyIndices[productIndex];
            }
            serialized << "]\n    }";
            if (index + 1 < document.separations.size()) serialized << ',';
            serialized << '\n';
        }
        if (!document.separations.empty()) serialized << "  ";
        serialized << "]\n}\n";

        std::ofstream output(file, std::ios::binary);
        if (!output.is_open()) {
            throw std::runtime_error{"Failed to open project file for writing: " + file.string()};
        }
        output << serialized.str();
        output.close();
        if (!output) {
            throw std::runtime_error{"Failed to write project file: " + file.string()};
        }
    }
}
