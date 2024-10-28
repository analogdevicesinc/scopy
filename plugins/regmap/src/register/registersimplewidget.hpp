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

#ifndef REGISTERSIMPLEWIDGET_HPP
#define REGISTERSIMPLEWIDGET_HPP

#include "registermodel.hpp"

#include <QColor>
#include <QFrame>
#include <QWidget>
#include <qstring.h>

class QLabel;
class QHBoxLayout;

namespace scopy::regmap {
class BitFieldSimpleWidget;

class RegisterSimpleWidget : public QFrame
{
	friend class RegmapStyleHelper;

	Q_OBJECT
public:
	explicit RegisterSimpleWidget(RegisterModel *registerModel, QVector<BitFieldSimpleWidget *> *bitFields,
				      QWidget *parent = nullptr);
	~RegisterSimpleWidget();

	void valueUpdated(uint32_t value);
	void setRegisterSelected(bool selected);

	RegisterModel *getRegisterModel() const;

public Q_SLOTS:
	void applyStyle();

private:
	QHBoxLayout *layout;
	QLabel *value;
	QVector<BitFieldSimpleWidget *> *bitFields;
	QFrame *regBaseInfoWidget = nullptr;
	QLabel *registerNameLabel;
	RegisterModel *registerModel;

protected:
	bool eventFilter(QObject *object, QEvent *event) override;
Q_SIGNALS:
	void registerSelected(uint32_t address);
};
} // namespace scopy::regmap
#endif // REGISTERSIMPLEWIDGET_HPP
