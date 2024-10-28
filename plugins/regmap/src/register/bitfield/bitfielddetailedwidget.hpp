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

#ifndef BitFieldDetailedWidget_HPP
#define BitFieldDetailedWidget_HPP

#include "bitfieldoption.hpp"

#include <QFrame>
#include <smallOnOffSwitch.h>

class QLabel;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QVBoxLayout;

class QHBoxLayout;
namespace scopy::regmap {

class BitFieldDetailedWidget : public QFrame
{
	friend class RegmapStyleHelper;

	Q_OBJECT
public:
	explicit BitFieldDetailedWidget(QString name, QString access, int defaultValue, QString description, int width,
					QString notes, int regOffset, QVector<BitFieldOption *> *options,
					QWidget *parent = nullptr);
	~BitFieldDetailedWidget();

	QString getToolTip() const;
	void updateValue(QString newValue);
	void registerValueUpdated(QString newValue);
	QString getValue();

	int getWidth() const;

	int getRegOffset() const;

private:
	QFrame *mainFrame;
	QVBoxLayout *layout;
	QString toolTip;
	int width;
	QString description;
	bool reserved;
	int regOffset;
	QString access;

	QVector<BitFieldOption *> *options;
	QLabel *nameLabel;
	QLabel *descriptionLabel;
	QLabel *lastReadValue;
	QLabel *defaultValueLabel;
	QLabel *value = nullptr;
	QComboBox *valueComboBox = nullptr;
	SmallOnOffSwitch *valueSwitch = nullptr;
	QLineEdit *valueLineEdit = nullptr;

	bool isFirstRead = true;

	void firstRead();

Q_SIGNALS:
	void valueUpdated(QString value);
};
} // namespace scopy::regmap
#endif // BitFieldDetailedWidget_HPP
