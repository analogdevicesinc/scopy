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

#ifndef REGISTERBLOCKWIDGET_H
#define REGISTERBLOCKWIDGET_H

#include "../scopy-admt_export.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>

#include <menucollapsesection.h>
#include <menusectionwidget.h>

namespace scopy::admt {
class SCOPY_ADMT_EXPORT RegisterBlockWidget : public QWidget
{
	Q_OBJECT
public:
	enum ACCESS_PERMISSION
	{
		READ,
		WRITE,
		READWRITE
	};

	QPushButton *m_readButton, *m_writeButton;

	RegisterBlockWidget(QString header, QString description, uint32_t address, uint8_t cnvPage,
			    RegisterBlockWidget::ACCESS_PERMISSION accessPermission, QWidget *parent = nullptr);
	virtual ~RegisterBlockWidget();
	QPushButton *readButton();
	QPushButton *writeButton();
	uint32_t getAddress();
	uint32_t getCnvPage();
	uint32_t getValue();
	void setValue(uint32_t value);
	RegisterBlockWidget::ACCESS_PERMISSION getAccessPermission();
public Q_SLOTS:
	void onValueChanged(int);

private:
	uint32_t m_address, m_value, m_cnvPage;
	RegisterBlockWidget::ACCESS_PERMISSION m_accessPermission;

	QSpinBox *m_spinBox;

	void addReadButton(QWidget *parent);
	void addWriteButton(QWidget *parent);
};

class SCOPY_ADMT_EXPORT PaddedSpinBox : public QSpinBox
{
	Q_OBJECT
public:
	PaddedSpinBox(QWidget *parent = nullptr);
	virtual ~PaddedSpinBox();

protected:
	QString textFromValue(int value) const override;
};
} // namespace scopy::admt

#endif // REGISTERBLOCKWIDGET_H
