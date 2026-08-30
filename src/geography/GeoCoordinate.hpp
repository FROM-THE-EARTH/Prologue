// ------------------------------------------------
// 緯度経度に関するクラス
// ------------------------------------------------

#pragma once

#include <optional>
#include <utility>

#include <boost/geometry.hpp>
#include <boost/geometry/srs/transformation.hpp>

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
    explicit GeoCoordinate(double latitude, double longitude, int zone);

    double latitude() const;

    double longitude() const;

	std::pair<double, double> LatLonAt(double x, double y) const;
};
