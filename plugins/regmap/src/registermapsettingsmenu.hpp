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

#ifndef REGISTERMAPSETTINGSMENU_HPP
#define REGISTERMAPSETTINGSMENU_HPP

#include <QObject>
#include <QPushButton>
#include <QWidget>
#include <menuheader.h>

class QCheckBox;

class QLineEdit;
namespace scopy {
namespace regmap {

class RegisterMapSettingsMenu : public QWidget
{
	friend class RegmapStyleHelper;
	friend class RegMap_API;
	Q_OBJECT

public:
	explicit RegisterMapSettingsMenu(QWidget *parent = nullptr);

Q_SIGNALS:
	void autoreadToggled(bool toggled);
	void requestRead(int address);
	void requestWrite(uint32_t address, uint32_t value);
	void requestRegisterDump(QString path);

private:
	MenuHeaderWidget *header;
	void applyStyle();
	QCheckBox *autoread;
	QLabel *hexaPrefix1;
	QLabel *hexaPrefix2;
	QLineEdit *startReadInterval;
	QLineEdit *endReadInterval;
	QPushButton *readInterval;
	QLineEdit *filePath;
	QPushButton *writeListOfValuesButton;
	QPushButton *registerDump;
	QPushButton *pathButton;
};
} // namespace regmap
} // namespace scopy
#endif // REGISTERMAPSETTINGSMENU_HPP
