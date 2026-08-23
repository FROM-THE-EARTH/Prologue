// ------------------------------------------------
// RocketSpec.hppの実装
// ------------------------------------------------

#include "RocketSpec.hpp"

#include <stdexcept>
#include <utility>

RocketSpecification::RocketSpecification(std::vector<BodySpecification> bodySpecs,
                                         std::vector<SeparationSpecification> separations) :
    m_bodySpecs(std::move(bodySpecs)), m_separations(std::move(separations)) {
    if (m_bodySpecs.empty()) {
        throw std::invalid_argument("Rocket specification must contain at least one body.");
    }

    for (const auto& separation : m_separations) {
        if (separation.sourceBodyIndex >= m_bodySpecs.size()) {
            throw std::invalid_argument("Separation source body index is out of range.");
        }
        if (separation.productBodyIndices.empty()) {
            throw std::invalid_argument("Separation must contain at least one product body.");
        }
        for (const size_t productBodyIndex : separation.productBodyIndices) {
            if (productBodyIndex >= m_bodySpecs.size() || productBodyIndex == separation.sourceBodyIndex) {
                throw std::invalid_argument("Separation product body index is invalid.");
            }
        }
    }
}
