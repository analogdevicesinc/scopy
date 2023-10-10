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

#include "ui_faultsdevice.h"

#include <iio.h>

#include <QTextEdit>
#include <QWidget>

#include <gui/subsection_separator.hpp>
#include <iioutil/commandqueue.h>

namespace scopy::swiot {
class FaultsGroup;

class FaultsDevice : public QWidget
{
	Q_OBJECT
public:
	explicit FaultsDevice(const QString &name, QString path, struct iio_device *device, struct iio_device *swiot,
			      struct iio_context *context, QVector<uint32_t> &registers, QWidget *parent = nullptr);
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
	void deviceConfigCmdFinished(scopy::Command *cmd);
	void functionConfigCmdFinished(scopy::Command *cmd);

private:
	void initTutorialProperties();
	void initFaultExplanations();
	void connectSignalsAndSlots();
	void initSpecialFaults();

	Ui::FaultsDevice *ui;
	QWidget *m_faults_explanation;
	scopy::gui::SubsectionSeparator *m_subsectionSeparator;

	CommandQueue *m_cmdQueue;

	FaultsGroup *m_faultsGroup;
	QVector<QWidget *> m_faultExplanationWidgets;

	QString m_name;
	QString m_path;

	struct iio_device *m_device;
	struct iio_device *m_swiot;
	struct iio_context *m_context;

	uint32_t m_faultNumeric;
	QVector<uint32_t> m_registers;
	QVector<uint32_t> m_registerValues;
	QVector<Command *> m_deviceConfigCmds;
	QVector<Command *> m_functionConfigCmds;
};

} // namespace scopy::swiot

#endif // FAULTSDEVICE_H
