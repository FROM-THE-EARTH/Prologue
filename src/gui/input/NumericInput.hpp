#pragma once

#include <optional>

#include <QLineEdit>

class NumericInput final : public QLineEdit {
public:
    explicit NumericInput(QWidget* parent = nullptr);

    void setValue(double value);
    [[nodiscard]] double value(const QString& fieldName) const;
    [[nodiscard]] std::optional<double> optionalValue(const QString& fieldName) const;
};
