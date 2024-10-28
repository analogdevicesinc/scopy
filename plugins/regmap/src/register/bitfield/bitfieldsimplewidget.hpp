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

#ifndef BITFIELDSIMPLEWIDGET_HPP
#define BITFIELDSIMPLEWIDGET_HPP

#include <QFrame>
#include <QWidget>

class QHBoxLayout;

class QVBoxLayout;
class QLabel;
namespace scopy::regmap {
class BitFieldSimpleWidget : public QFrame
{
	friend class RegmapStyleHelper;
	Q_OBJECT
public:
	explicit BitFieldSimpleWidget(QString name, int defaultValue, QString description, int width, QString notes,
				      int regOffset, int streach, QWidget *parent = nullptr);

	~BitFieldSimpleWidget();
	void updateValue(QString newValue);
	int getWidth() const;

	QString getDescription() const;

	int getStreach() const;
	QString getName() const;

public Q_SLOTS:
	void applyStyle();
	void setSelected(bool selected);

private:
	QHBoxLayout *layout;
	QLabel *value;
	int width;
	int streach;
	QString description;
	QString name;
	QFrame *mainFrame;

Q_SIGNALS:
};
} // namespace scopy::regmap
#endif // BITFIELDSIMPLEWIDGET_HPP
