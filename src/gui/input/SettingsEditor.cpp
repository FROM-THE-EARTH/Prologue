#include "SettingsEditor.hpp"

#include <limits>
#include <stdexcept>
#include <utility>

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QVBoxLayout>

#include "gui/FileActions.hpp"
#include "gui/input/NumericInput.hpp"

namespace {
    NumericInput* AddNumber(QFormLayout* form,
                            const QString& label,
                            double value,
                            QWidget* parent,
                            const QString& objectName) {
        auto* input = new NumericInput(parent);
        input->setObjectName(objectName);
        input->setValue(value);
        form->addRow(label, input);
        return input;
    }

    QLineEdit* AddInteger(QFormLayout* form,
                          const QString& label,
                          int value,
                          int minimum,
                          int maximum,
                          QWidget* parent,
                          const QString& objectName) {
        auto* input = new QLineEdit(QString::number(value), parent);
        input->setObjectName(objectName);
        input->setValidator(new QIntValidator(minimum, maximum, input));
        form->addRow(label, input);
        return input;
    }

    int IntegerValue(const QLineEdit* input, const QString& label, int minimum, int maximum) {
        bool ok = false;
        const int value = input->text().toInt(&ok);
        if (input->text().trimmed().isEmpty() || !ok || value < minimum || value > maximum) {
            throw std::invalid_argument((label + QStringLiteral(" must be an integer in [%1, %2].")
                                                  .arg(minimum).arg(maximum)).toStdString());
        }
        return value;
    }

    qulonglong UnsignedValue(const QLineEdit* input,
                             const QString& label,
                             qulonglong minimum,
                             qulonglong maximum) {
        bool ok = false;
        const auto value = input->text().toULongLong(&ok);
        if (input->text().trimmed().isEmpty() || !ok || value < minimum || value > maximum) {
            throw std::invalid_argument((label + QStringLiteral(" must be a non-negative integer in [%1, %2].")
                                                  .arg(minimum).arg(maximum)).toStdString());
        }
        return value;
    }

    std::filesystem::path SettingsBase(const std::filesystem::path& settingsFile) {
        const auto parent = settingsFile.parent_path();
        return settingsFile.empty() || parent.empty() ? std::filesystem::current_path() : parent;
    }

}

SettingsEditor::SettingsEditor(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);

    auto* processing = new QGroupBox(tr("Processing"), this);
    auto* processingForm = new QFormLayout(processing);
    auto* threadRow = new QWidget(processing);
    auto* threadLayout = new QHBoxLayout(threadRow);
    threadLayout->setContentsMargins(0, 0, 0, 0);
    m_multiThread = new QCheckBox(tr("Run scatter simulations in parallel"), threadRow);
    m_multiThread->setObjectName("multiThread");
    m_threadCount = new QLineEdit(threadRow);
    m_threadCount->setObjectName("threadCount");
    m_threadCount->setValidator(new QRegularExpressionValidator(
        QRegularExpression(QStringLiteral("^[0-9]{1,20}$")), m_threadCount));
    threadLayout->addWidget(m_multiThread);
    threadLayout->addStretch();
    threadLayout->addWidget(new QLabel(tr("Worker threads"), threadRow));
    threadLayout->addWidget(m_threadCount);
    processingForm->addRow(tr("Execution"), threadRow);
    m_timeStep = AddNumber(processingForm, tr("Simulation time step [s]"), 0.001, processing, "timeStep");
    m_scatterMin = AddNumber(processingForm, tr("Scatter minimum wind speed [m/s]"), 0.0, processing, "scatterMin");
    m_scatterMax = AddNumber(processingForm, tr("Scatter maximum wind speed [m/s]"), 0.0, processing, "scatterMax");
    m_scatterDirectionInterval = AddNumber(processingForm, tr("Scatter direction interval [deg]"), 0.0, processing, "scatterDirectionInterval");
    layout->addWidget(processing);

    auto* result = new QGroupBox(tr("Result output"), this);
    auto* resultForm = new QFormLayout(result);
    m_precision = AddInteger(resultForm, tr("CSV decimal places"), 8, 0, std::numeric_limits<int>::max(), result, "resultPrecision");
    m_stepSaveInterval = AddInteger(resultForm, tr("Save every N simulation steps"), 10, 1, std::numeric_limits<int>::max(), result, "stepSaveInterval");
    m_stepSaveInterval->setValidator(new QRegularExpressionValidator(
        QRegularExpression(QStringLiteral("^[0-9]{1,10}$")), m_stepSaveInterval));
    layout->addWidget(result);

    auto* wind = new QGroupBox(tr("Wind model"), this);
    auto* windForm = new QFormLayout(wind);
    m_windType = new QComboBox(wind);
    m_windType->setObjectName("windType");
    m_windType->addItem(tr("Measured profile"), static_cast<int>(WindModelType::Real));
    m_windType->addItem(tr("Original power law"), static_cast<int>(WindModelType::Original));
    m_windType->addItem(tr("Low altitude power law"), static_cast<int>(WindModelType::OnlyPowerLow));
    m_windType->addItem(tr("No wind"), static_cast<int>(WindModelType::NoWind));
    windForm->addRow(tr("Wind type"), m_windType);
    m_powerConstant = AddNumber(windForm, tr("Power-law constant"), 6.0, wind, "powerConstant");
    m_powerLowBaseAltitude = AddNumber(windForm, tr("Low-altitude base height [m]"), 2.0, wind, "powerLowBaseAltitude");
    auto* windFileRow = new QWidget(wind);
    auto* windFileLayout = new QHBoxLayout(windFileRow);
    windFileLayout->setContentsMargins(0, 0, 0, 0);
    m_measuredWindFilename = new QLineEdit(windFileRow);
    m_measuredWindFilename->setObjectName("measuredWindFilename");
    auto* windBrowse = new QPushButton(tr("Browse…"), windFileRow);
    windFileLayout->addWidget(m_measuredWindFilename, 1);
    windFileLayout->addWidget(windBrowse);
    windForm->addRow(tr("Measured wind CSV"), windFileRow);
    connect(windBrowse, &QPushButton::clicked, this, [this, windFileRow] {
        auto start = SettingsBase(m_settingsFile) / "input/wind";
        const auto value = m_measuredWindFilename->text().trimmed();
        if (!value.isEmpty()) {
            auto current = gui::FilePath(value);
            if (current.is_relative()) current = start / current;
            start = current;
        }
        const auto selected = QFileDialog::getOpenFileName(windFileRow,
                                                           tr("Select measured wind file"),
                                                           gui::PathText(start),
                                                           tr("CSV files (*.csv);;All files (*)"));
        if (selected.isEmpty()) return;
        const auto selectedPath = gui::FilePath(selected);
        std::error_code error;
        auto relative = std::filesystem::relative(
            selectedPath, SettingsBase(m_settingsFile) / "input/wind", error);
        if (error) relative = selectedPath;
        m_measuredWindFilename->setText(gui::PathText(relative));
    });
    layout->addWidget(wind);

    auto* atmosphere = new QGroupBox(tr("Atmosphere at launch site"), this);
    auto* atmosphereForm = new QFormLayout(atmosphere);
    m_basePressure = AddNumber(atmosphereForm, tr("Base pressure [Pa]"), 101325.0, atmosphere, "basePressure");
    m_baseTemperature = AddNumber(atmosphereForm, tr("Base temperature [°C]"), 15.0, atmosphere, "baseTemperature");
    layout->addWidget(atmosphere);
    layout->addStretch();
}

void SettingsEditor::setSettings(const ApplicationSettings& settings,
                                 std::filesystem::path settingsFile) {
    m_baseSettings = settings;
    m_settingsFile = std::move(settingsFile);
    m_multiThread->setChecked(settings.execution.multiThread);
    m_threadCount->setText(QString::number(static_cast<qulonglong>(settings.execution.threadCount)));
    m_timeStep->setValue(settings.solver.timeStep);
    m_scatterMin->setValue(settings.scatter.windSpeedMin);
    m_scatterMax->setValue(settings.scatter.windSpeedMax);
    m_scatterDirectionInterval->setValue(settings.scatter.windDirectionInterval);
    m_precision->setText(QString::number(settings.result.precision));
    m_stepSaveInterval->setText(QString::number(settings.solver.resultStepSaveInterval));
    const int windIndex = m_windType->findData(static_cast<int>(settings.solver.wind.type));
    if (windIndex >= 0) m_windType->setCurrentIndex(windIndex);
    m_powerConstant->setValue(settings.solver.wind.powerConstant);
    m_powerLowBaseAltitude->setValue(settings.solver.wind.powerLowBaseAltitude);
    m_measuredWindFilename->setText(QString::fromUtf8(settings.measuredWindFilename.data(),
                                                      static_cast<qsizetype>(settings.measuredWindFilename.size())));
    m_basePressure->setValue(settings.solver.atmosphere.basePressure);
    m_baseTemperature->setValue(settings.solver.atmosphere.baseTemperature);
}

ApplicationSettings SettingsEditor::settings() const {
    ApplicationSettings result = m_baseSettings;
    result.execution.multiThread = m_multiThread->isChecked();
    result.execution.threadCount = static_cast<size_t>(UnsignedValue(
        m_threadCount, tr("Worker thread count"), 1, std::numeric_limits<size_t>::max()));
    result.solver.timeStep = m_timeStep->value(tr("Simulation time step [s]"));
    result.scatter.windSpeedMin = m_scatterMin->value(tr("Scatter minimum wind speed [m/s]"));
    result.scatter.windSpeedMax = m_scatterMax->value(tr("Scatter maximum wind speed [m/s]"));
    result.scatter.windDirectionInterval = m_scatterDirectionInterval->value(tr("Scatter direction interval [deg]"));
    result.result.precision = IntegerValue(m_precision, tr("CSV decimal places"), 0, std::numeric_limits<int>::max());
    result.solver.resultStepSaveInterval = static_cast<unsigned int>(UnsignedValue(
        m_stepSaveInterval, tr("Step save interval"), 1, std::numeric_limits<unsigned int>::max()));
    result.solver.wind.type = static_cast<WindModelType>(m_windType->currentData().toInt());
    result.solver.wind.powerConstant = m_powerConstant->value(tr("Power-law constant"));
    result.solver.wind.powerLowBaseAltitude = m_powerLowBaseAltitude->value(tr("Low-altitude base height [m]"));
    const auto measuredWindFilename = m_measuredWindFilename->text().trimmed().toUtf8();
    result.measuredWindFilename.assign(measuredWindFilename.constData(),
                                       static_cast<size_t>(measuredWindFilename.size()));
    result.solver.atmosphere.basePressure = m_basePressure->value(tr("Base pressure [Pa]"));
    result.solver.atmosphere.baseTemperature = m_baseTemperature->value(tr("Base temperature [°C]"));
    return result;
}
