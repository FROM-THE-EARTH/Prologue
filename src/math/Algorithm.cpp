#include "Algorithm.hpp"

#include "app/CommandLine.hpp"

namespace Algorithm {
    double Lerp(double x, double _x1, double _x2, double _y1, double _y2) {
        if (_x1 == _x2) {
            CommandLine::PrintInfo(PrintInfoType::Warning, "In Algorithm::Lerp()", "x1 == x2. Could not divide by 0.");
            return _y1;
        }

        const double grad = (_y2 - _y1) / (_x2 - _x1);
        return _y1 + grad * (x - _x1);
    }
}
