// ------------------------------------------------
// Algorithm.hppの実装
// ------------------------------------------------

#include "Algorithm.hpp"

#include "app/CommandLine.hpp"
#include "math/Interpolation.hpp"

namespace Algorithm {
    double Lerp(double x, double _x1, double _x2, double _y1, double _y2) {
        if (_x1 == _x2) {
            CommandLine::PrintInfo(PrintInfoType::Warning, "In Algorithm::Lerp()", "x1 == x2. Could not divide by 0.");
            return _y1;
        }

        return Interpolation::Linear(x, _x1, _x2, _y1, _y2);
    }
}
