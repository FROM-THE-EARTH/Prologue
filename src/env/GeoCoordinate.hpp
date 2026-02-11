// ------------------------------------------------
// 緯度経度に関するクラス
// ------------------------------------------------

#pragma once

#include <string>
#include <optional>

#include <boost/geometry.hpp>
#include <boost/geometry/srs/transformation.hpp>

#include "misc/Constant.hpp"
#include "app/CommandLine.hpp"

class GeoCoordinate {
private:
	using ll_point = boost::geometry::model::point<
			double, 2, boost::geometry::cs::geographic<boost::geometry::degree>
		>;
	using xy_point = boost::geometry::model::d2::point_xy<double>;

	ll_point m_ll; // launchpoint [deg]
	xy_point m_xy; // launchpoint in XY (rectangular coordinate)

	std::optional<boost::geometry::srs::projection<>> m_prj;

public:
    explicit GeoCoordinate(double latitude, double longitude, int zone = -1);

    double latitude() const;

    double longitude() const;

	std::pair<double, double> LatLonAt(double x, double y) const;
};
