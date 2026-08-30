// ------------------------------------------------
// FileProjectResourceProvider.hppの実装
// ------------------------------------------------

#include "FileProjectResourceProvider.hpp"

#include <cmath>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

// disable all warnings of fast-cpp-csv-parser
#pragma warning(push, 0)
#include <fast-cpp-csv-parser/csv.h>
#pragma warning(pop)

namespace ProjectIO {
    namespace {
        std::filesystem::path Resolve(const std::filesystem::path& projectDirectory,
                                      const std::filesystem::path& referencedPath) {
            return referencedPath.is_absolute() ? referencedPath : projectDirectory / referencedPath;
        }
    }

    FileProjectResourceProvider::FileProjectResourceProvider(std::filesystem::path projectDirectory) :
        m_projectDirectory(std::move(projectDirectory)) {}

    std::vector<ThrustData> FileProjectResourceProvider::loadThrustData(
        const std::filesystem::path& projectRelativePath) const {
        const auto file = Resolve(m_projectDirectory, projectRelativePath);
        std::ifstream stream(file);
        if (!stream.is_open()) {
            throw std::runtime_error{"Failed to open thrust data file: " + file.string()};
        }

        while (true) {
            const auto next = stream.peek();
            if (next == std::char_traits<char>::eof()) {
                throw std::runtime_error{"No thrust data found in: " + file.string()};
            }
            if ('0' <= next && next <= '9') break;

            std::string headerLine;
            std::getline(stream, headerLine);
        }

        std::vector<ThrustData> data;
        while (true) {
            ThrustData point;
            if (!(stream >> point.time >> point.thrust)) break;
            data.emplace_back(point);
        }

        if (!stream.eof()) {
            throw std::runtime_error{"Invalid thrust data in: " + file.string()};
        }
        if (data.empty()) {
            throw std::runtime_error{"No thrust data found in: " + file.string()};
        }
        if (data.front().time != 0.0) {
            data.insert(data.begin(), ThrustData{0.0, 0.0});
        }
        if (data.size() < 2) {
            throw std::runtime_error{"Thrust data must contain at least two points: " + file.string()};
        }
        for (size_t index = 0; index < data.size(); index++) {
            if (!std::isfinite(data[index].time) || !std::isfinite(data[index].thrust)) {
                throw std::runtime_error{"Thrust data must contain only finite values: " + file.string()};
            }
            if (index > 0 && data[index - 1].time >= data[index].time) {
                throw std::runtime_error{"Thrust data time must be strictly increasing: " + file.string()};
            }
        }
        return data;
    }

    std::vector<AeroCoefficientData> FileProjectResourceProvider::loadAeroCoefficientData(
        const std::filesystem::path& projectRelativePath) const {
        const auto file = Resolve(m_projectDirectory, projectRelativePath);
        try {
            io::CSVReader<7> csv(file.string());
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
            if (data.empty()) {
                throw std::runtime_error{"No aerodynamic coefficient data found."};
            }
            return data;
        } catch (const std::exception& error) {
            throw std::runtime_error{"Failed to read aerodynamic coefficient file " + file.string()
                                     + ": " + error.what()};
        }
    }
}
