#pragma once

#include <filesystem>

#include <QWidget>

#include "config/ApplicationSettings.hpp"

class QCheckBox;
class QComboBox;
class QLineEdit;
class NumericInput;

class SettingsEditor final : public QWidget {
    ApplicationSettings m_baseSettings;
    std::filesystem::path m_settingsFile;
    QCheckBox* m_multiThread = nullptr;
    QLineEdit* m_threadCount = nullptr;
    NumericInput* m_timeStep = nullptr;
    NumericInput* m_scatterMin = nullptr;
    NumericInput* m_scatterMax = nullptr;
    NumericInput* m_scatterDirectionInterval = nullptr;
    QLineEdit* m_precision = nullptr;
    QLineEdit* m_stepSaveInterval = nullptr;
    QComboBox* m_windType = nullptr;
    NumericInput* m_powerConstant = nullptr;
    NumericInput* m_powerLowBaseAltitude = nullptr;
    QLineEdit* m_measuredWindFilename = nullptr;
    NumericInput* m_basePressure = nullptr;
    NumericInput* m_baseTemperature = nullptr;

public:
    explicit SettingsEditor(QWidget* parent = nullptr);

    void setSettings(const ApplicationSettings& settings, std::filesystem::path settingsFile);
    [[nodiscard]] ApplicationSettings settings() const;
};
