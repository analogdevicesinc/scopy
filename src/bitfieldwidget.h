/*
 * Copyright 2017 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef BITFIELD_H
#define BITFIELD_H

#include <QDomDocument>
#include <QWidget>

#include <math.h>

namespace Ui {
class BitfieldWidget;
}

namespace adiscope {

class BitfieldWidget : public QWidget {
	Q_OBJECT

public:
	explicit BitfieldWidget(QWidget *parent = 0, QDomElement *bitfield = nullptr);
	explicit BitfieldWidget(QWidget *parent, int bitNumber);

	~BitfieldWidget();
	void updateValue(uint32_t &value);
	int getRegOffset(void) const;
	int getSliceWidth(void) const;
	uint32_t getDefaultValue(void) const;

private:
	void createWidget(void);

Q_SIGNALS:
	void valueChanged(uint32_t value, uint32_t mask);

private Q_SLOTS:
	void setValue(int value);

private:
	Ui::BitfieldWidget *ui;
	QDomElement *bitfield;
	QDomElement options;

	int width;
	int sliceWidth;
	int regOffset;

	uint32_t value;
	uint32_t defaultValue;

	QString description;
	QString notes;
	QString name, access;
	QString option;
};
} // namespace adiscope
#endif // BITFIELD_H
