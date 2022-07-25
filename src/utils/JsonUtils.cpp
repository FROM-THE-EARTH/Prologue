﻿// ------------------------------------------------
// JsonUtils.hppの実装
// ------------------------------------------------

#include "JsonUtils.hpp"

namespace JsonUtils {
    bool Exist(const boost::property_tree::ptree& pt, const std::string& key) {
        return pt.find(key) != pt.not_found();
    }
}
