#include "MeasuredWindProfileReader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace MeasuredWindProfileReader {
    WindProfile Read(const std::filesystem::path& windFile) {
        std::ifstream stream(windFile);
        if (!stream.is_open()) {
            throw std::runtime_error{"Failed to open wind data file: " + windFile.string()};
        }

        std::string header;
        std::getline(stream, header);

        std::vector<WindData> data;
        std::string line;
        size_t lineNumber = 1;
        while (std::getline(stream, line)) {
            lineNumber++;
            if (line.empty()) continue;

            std::istringstream row(line);
            WindData point;
            char firstComma, secondComma;
            if (!(row >> point.geometricHeight >> firstComma >> point.speed >> secondComma >> point.direction)
                || firstComma != ',' || secondComma != ',') {
                throw std::runtime_error{"Invalid wind data at line " + std::to_string(lineNumber) + " in: "
                                         + windFile.string()};
            }
            row >> std::ws;
            if (!row.eof()) {
                throw std::runtime_error{"Unexpected value at line " + std::to_string(lineNumber) + " in: "
                                         + windFile.string()};
            }
            data.push_back(point);
        }
        return WindProfile(std::move(data), DirectionReference::MagneticNorth);
    }
}
