/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#ifndef FAULTSDEVICE_H
#define FAULTSDEVICE_H

#include "faultsgroup.h"
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include <gui/widgets/menucollapsesection.h>

#include <component/controller.h>
#include <qcoro/qcorotask.h>

namespace scopy {
namespace component {
class Device;
class Attribute;
namespace iio {
class IIORegisterReader;
}
} // namespace component

namespace swiot {
class FaultsGroup;
class SWIOT_API;

class FaultsDevice : public QWidget
{
	Q_OBJECT
	friend class SWIOT_API;

public:
	explicit FaultsDevice(const QString &name, QString path, QString uri, QVector<uint32_t> &registers,
			      QWidget *parent = nullptr);
	~FaultsDevice();

	void update();
	void readRegister();
public Q_SLOTS:
	void resetStored();
	void updateExplanations();
	void updateExplanation(int index);
	void onFaultNumericUpdated();
	void onFaultRegisterRead(int iReg, uint32_t value);

Q_SIGNALS:
	void specialFaultsUpdated(int index, QString channelFunction);
	void faultNumericUpdated();
	void faultRegisterRead(int iReg, uint32_t value);

private Q_SLOTS:
	void updateMinimumHeight();

private:
	void establishConnection(QString name);
	QCoro::Task<void> initSpecialFaults();
	QCoro::Task<void> readRegisters();
	void initTutorialProperties();
	void connectSignalsAndSlots();
	QWidget *createTopWidget(QWidget *parent);
	QWidget *createExplanationSection(QWidget *parent);

	QLineEdit *m_registerNoLineEdit;
	QPushButton *m_resetBtn;
	QPushButton *m_clearBtn;
	QWidget *m_faultsExplanation;
	MenuCollapseSection *m_explanationSection;

	QString m_uri;
	component::ContextHandle m_context;

	FaultsGroup *m_faultsGroup;
	QVector<QWidget *> m_faultExplanationWidgets;

	QString m_name;

	component::Device *m_device;
	component::Device *m_swiot;
	component::iio::IIORegisterReader *m_registerReader;

	uint32_t m_faultNumeric = 0;
	QVector<uint32_t> m_registers;
	QMap<int, uint32_t> m_registerValues;

	QCoro::Task<void> m_readRegTask;
};

} // namespace swiot
} // namespace scopy

#endif // FAULTSDEVICE_H
