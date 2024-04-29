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

#include "scopy-swiotrefactor_export.h"
#include "faultsgroup.h"
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

#include <iio.h>
#include <iioutil/commandqueue.h>

namespace scopy::swiotrefactor {
class FaultsGroup;

class SCOPY_SWIOTREFACTOR_EXPORT FaultsDevice : public QWidget
{
	Q_OBJECT
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
	void deviceConfigCmdFinished(scopy::Command *cmd);
	void functionConfigCmdFinished(scopy::Command *cmd);

private:
	void establishConnection(QString name);
	void initSpecialFaults();
	void initTutorialProperties();
	void connectSignalsAndSlots();
	QWidget *createTopWidget(QWidget *parent);
	QWidget *createExplanationSection(QWidget *parent);

	QLineEdit *m_registerNoLineEdit;
	QPushButton *m_resetBtn;
	QPushButton *m_clearBtn;
	QWidget *m_faultsExplanation;

	QString m_uri;
	CommandQueue *m_cmdQueue;

	FaultsGroup *m_faultsGroup;
	QVector<QWidget *> m_faultExplanationWidgets;

	QString m_name;

	struct iio_device *m_device;
	struct iio_device *m_swiot;
	struct iio_context *m_context;

	uint32_t m_faultNumeric = 0;
	QVector<uint32_t> m_registers;
	QMap<int, uint32_t> m_registerValues;
	QVector<Command *> m_deviceConfigCmds;
	QVector<Command *> m_functionConfigCmds;
};

} // namespace scopy::swiotrefactor

#endif // FAULTSDEVICE_H
