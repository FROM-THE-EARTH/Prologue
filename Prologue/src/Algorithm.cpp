#include "Algorithm.h"

namespace Algorithm {
	double ToLinear(double x, double _x1, double _x2, double _y1, double _y2) {
		const double grad = (_y2 - _y1) / (_x2 - _x1);
		return _x1 + grad * (x - _x1);
	}
}