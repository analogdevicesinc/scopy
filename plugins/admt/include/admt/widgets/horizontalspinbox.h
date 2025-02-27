/*
 * Copyright (c) 2025 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef HORIZONTALSPINBOX_H
#define HORIZONTALSPINBOX_H

#include "../scopy-admt_export.h"

#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

namespace scopy::admt {
class SCOPY_ADMT_EXPORT HorizontalSpinBox : public QWidget {
  Q_OBJECT
public:
  HorizontalSpinBox(QString header = "", double initialValue = 0.0,
                    QString unit = "", QWidget *parent = nullptr);
  QLineEdit *lineEdit();
  void setEnabled(double value);
public Q_SLOTS:
  void setValue(double);
protected Q_SLOTS:
  void onMinusButtonPressed();
  void onPlusButtonPressed();
  void onLineEditTextEdited();

private:
  double m_value = 0;
  QString m_unit = "";
  QLineEdit *m_lineEdit;
  QPushButton *m_minusButton, *m_plusButton;
  QLabel *m_unitLabel;
  void applyLineEditStyle(QLineEdit *widget);
  void applyPushButtonStyle(QPushButton *widget, int topLeftBorderRadius = 0,
                            int topRightBorderRadius = 0,
                            int bottomLeftBorderRadius = 0,
                            int bottomRightBorderRadius = 0);
  void applyUnitLabelStyle(QLabel *widget, bool isEnabled = true);
};
} // namespace scopy::admt

#endif // HORIZONTALSPINBOX_H