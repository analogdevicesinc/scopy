/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#ifndef REGISTERCONTROLLER_HPP
#define REGISTERCONTROLLER_HPP

#include <QWidget>
#include "titlespinbox.hpp"
#include "scopy-regmap_export.h"

#include <QWidget>

class QHBoxLayout;
class QSpinBox;
class TextSpinBox;
class QLabel;
class QPushButton;
class QLineEdit;
class QVBoxLayout;

namespace scopy::regmap {
class RegisterModel;
class RegisterValueWidget;
class AddressPicker;

class SCOPY_REGMAP_EXPORT RegisterController : public QWidget
{
	friend class RegMap_API;
	Q_OBJECT
public:
	explicit RegisterController(QWidget *parent = nullptr);

	~RegisterController();

	void registerChanged(uint32_t address);
	void registerValueChanged(QString value);
	QString getAddress();
	void setHasMap(bool hasMap);

private:
	QHBoxLayout *mainLayout;

	TitleSpinBox *adrPck = nullptr;

	QSpinBox *addressPicker;
	QPushButton *readButton;
	QPushButton *writeButton;
	QPushButton *detailedRegisterToggle = nullptr;
	QLineEdit *regValue;
	QLabel *addressLabel;
	QLabel *valueLabel;
	QHBoxLayout *writeWidgetLayout;

	bool addressChanged = false;
	bool hasMap = false;
	void applyStyle();

Q_SIGNALS:
	void requestRead(uint32_t address);
	void requestWrite(uint32_t address, uint32_t value);
	void registerAddressChanged(uint32_t address);
	void valueChanged(QString value);
	void toggleDetailedMenu(bool toggled);
};
} // namespace scopy::regmap
#endif // REGISTERCONTROLLER_HPP
