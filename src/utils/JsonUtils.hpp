#pragma once

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace JsonUtils {
    template <typename T>
    T GetValue(const boost::property_tree::ptree& pt, const std::string& key) {
        if (boost::optional<T> value = pt.get_optional<T>(key)) {
            return value.get();
        }
        return T();
    }

    template <typename T>
    bool HasValue(const boost::property_tree::ptree& pt, const std::string& key) {
        if (boost::optional<T> value = pt.get_optional<T>(key))
            return true;
        else
            return false;
    }

    bool Exist(const boost::property_tree::ptree& pt, const std::string& key);
}
