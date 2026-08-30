// ------------------------------------------------
// 緯度経度に関するクラス
// ------------------------------------------------

#include "geography/GeoCoordinate.hpp"

#include <array>
#include <stdexcept>
#include <string>

namespace {
    // 平面直角座標系（平成十四年国土交通省告示第九号）
    constexpr std::array<std::pair<double, double>, 19> Origins = {{
			{129.5,                   33.0},  // I系    : 長崎県、鹿児島県（一部）
			{131.0,                   33.0},  // II系   : 福岡県、佐賀県、熊本県、大分県、宮崎県、鹿児島県（一部）
			{132.1666666666667,       36.0},  // III系  : 山口県、島根県、広島県
			{133.5,                   33.0},  // IV系   : 香川県、愛媛県、徳島県、高知県
			{134.3333333333333,       36.0},  // V系    : 兵庫県、鳥取県、岡山県
			{136.0,                   36.0},  // VI系   : 京都府、大阪府、福井県、滋賀県、三重県、奈良県、和歌山県
			{137.1666666666667,       36.0},  // VII系  : 石川県、富山県、岐阜県、愛知県
			{138.5,                   36.0},  // VIII系 : 新潟県、長野県、山梨県、静岡県
			{139.8333333333333,       36.0},  // IX系   : 東京都（一部）、福島県、栃木県、茨城県、埼玉県、千葉県、群馬県、神奈川県
			{140.8333333333333,       40.0},  // X系    : 青森県、秋田県、山形県、岩手県、宮城県
			{140.25,                  44.0},  // XI系   : 北海道（西部）
			{142.25,                  44.0},  // XII系  : 北海道（中部）
			{144.25,                  44.0},  // XIII系 : 北海道（東部）
			{142.0,                   26.0},  // XIV系  : 東京都（南部）
			{127.5,                   26.0},  // XV系   : 沖縄県（中央）
			{124.0,                   26.0},  // XVI系  : 沖縄県（西部）
			{131.0,                   26.0},  // XVII系 : 沖縄県（東部）
			{136.0,                   20.0},  // XVIII系: 東京都（南西部）
			{154.0,                   26.0}   // XIX系  : 東京都（南東部）
	}};
}

GeoCoordinate::GeoCoordinate(double latitude, double longitude, int zone) {
        if (zone < 1 || zone > static_cast<int>(Origins.size())) {
            throw std::invalid_argument{"Coordinate zone must be in [1, 19]."};
        }
        double lat = Origins[zone - 1].second;
        double lon = Origins[zone - 1].first;

		// Parameter reference: https://epsg.io/
		std::string proj4   = "+proj=tmerc +lat_0=" + std::to_string(lat) + " +lon_0=" + std::to_string(lon) + " +k=0.9999 +x_0=0 +y_0=0 +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs +type=crs";
		m_prj.emplace(
			boost::geometry::srs::proj4(proj4)
		);
		m_ll = ll_point(longitude, latitude);
		m_prj->forward(m_ll, m_xy);
}

double GeoCoordinate::latitude() const {
        return boost::geometry::get<1>(m_ll);
}

double GeoCoordinate::longitude() const {
        return boost::geometry::get<0>(m_ll);
}

std::pair<double, double> GeoCoordinate::LatLonAt(double x, double y) const {
		boost::geometry::model::d2::point_xy<double> p_xy(x, y);
		boost::geometry::add_point(p_xy, m_xy);
		ll_point p_ll;
		m_prj->inverse(p_xy, p_ll);
		return { boost::geometry::get<1>(p_ll), boost::geometry::get<0>(p_ll) };
}
