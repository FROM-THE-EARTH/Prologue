// ------------------------------------------------
// Interpolation.hppの実装
// ------------------------------------------------

#include "Interpolation.hpp"

namespace Interpolation {
    double Linear(double x, double x1, double x2, double y1, double y2) {
        if (x1 == x2) {
            return y1;
        }

        const double grad = (y2 - y1) / (x2 - x1);
        return y1 + grad * (x - x1);
    }
}
