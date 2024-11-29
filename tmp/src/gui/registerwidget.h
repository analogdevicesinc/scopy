/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef REGISTERWIDGET_H
#define REGISTERWIDGET_H

#include <QWidget>
#include <QDomDocument>

#include "regmapparser.h"
#include "gui/bitfieldwidget.h"
#include "debug.h"

namespace Ui {
class RegisterWidget;
}

namespace adiscope {

class RegisterWidget : public QWidget
{
	Q_OBJECT

public:
	explicit RegisterWidget(QWidget *parent = 0,  Debug  *debug = nullptr);
	~RegisterWidget();
	uint32_t getValue(void) const;
	QStringList getSources(void) const;
	uint32_t readRegister(const QString *device, const uint32_t address);
	void writeRegister(const QString *device, const uint32_t address,
	                   uint32_t regVal);
	void verifyAvailableSources(const QString device);
	QString getDescription() const;
	uint32_t getDefaultValue(void) const;
	uint32_t getLastAddress(void) const;

Q_SIGNALS:
	void valueChanged(int);

public Q_SLOTS:
	void createRegMap(const QString *device, int *address,const QString *source);
	void setValue(int var);
	void setValue(uint32_t var, uint32_t mask);
	void updateBitfields(void);

private:
	void checkRegisterMap(void);

	Ui::RegisterWidget *ui;
	RegmapParser regMap;

	QDomNode *regNode;
	QVector<BitfieldWidget *> bitfieldsVector;

	uint32_t value;
	uint32_t defaultValue;
	int width;

	QString device;
	QString source;

	QString name;
	uint32_t address;
	QString description;
	QString notes;

	QStringList fileSources;


};
}
#endif // REGISTERWIDGET_H
