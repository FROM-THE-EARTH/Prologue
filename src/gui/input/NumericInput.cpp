#include "NumericInput.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

#include <QDoubleValidator>
#include <QLocale>

NumericInput::NumericInput(QWidget* parent) : QLineEdit(parent) {
    auto* numericValidator = new QDoubleValidator(-std::numeric_limits<double>::max(),
                                                  std::numeric_limits<double>::max(),
                                                  1000,
                                                  this);
    numericValidator->setNotation(QDoubleValidator::ScientificNotation);
    numericValidator->setLocale(QLocale::c());
    setValidator(numericValidator);
}

void NumericInput::setValue(double input) {
    setText(QString::number(input, 'g', 17));
}

double NumericInput::value(const QString& fieldName) const {
    const auto textValue = text().trimmed();
    bool converted = false;
    const double parsed = textValue.toDouble(&converted);
    if (textValue.isEmpty() || !converted || !std::isfinite(parsed)) {
        throw std::invalid_argument((fieldName + QStringLiteral(" must be a finite number."))
                                        .toStdString());
    }
    return parsed;
}

std::optional<double> NumericInput::optionalValue(const QString& fieldName) const {
    if (text().trimmed().isEmpty()) return std::nullopt;
    return value(fieldName);
}
