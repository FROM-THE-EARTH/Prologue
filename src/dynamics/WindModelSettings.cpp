// ------------------------------------------------
// WindModelSettings.hppの実装
// ------------------------------------------------

#include "WindModelSettings.hpp"

#include <cmath>
#include <stdexcept>

void ValidateWindModelSettings(const WindModelSettings& settings) {
    if ((settings.type == WindModelType::Original || settings.type == WindModelType::OnlyPowerLow)
        && (!std::isfinite(settings.powerConstant) || settings.powerConstant <= 0.0)) {
        throw std::invalid_argument{"Wind power constant must be finite and positive."};
    }
    if ((settings.type == WindModelType::Original || settings.type == WindModelType::OnlyPowerLow)
        && (!std::isfinite(settings.powerLowBaseAltitude) || settings.powerLowBaseAltitude <= 0.0)) {
        throw std::invalid_argument{"Wind power-law base altitude must be finite and positive."};
    }
    if (settings.type == WindModelType::Real && !settings.measuredProfile.has_value()) {
        throw std::invalid_argument{"Measured wind model requires a wind profile."};
    }
}
