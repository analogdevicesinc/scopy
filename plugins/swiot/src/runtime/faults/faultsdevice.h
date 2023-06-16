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

#include <QWidget>
#include <QTextEdit>
#include "ui_faultsdevice.h"
#include "faultsgroup.h"
#include <gui/subsection_separator.hpp>
#include <iio.h>
#include <iioutil/commandqueue.h>

namespace scopy::swiot {
class FaultsGroup;

class FaultsDevice : public QWidget {
	Q_OBJECT
public:
	explicit FaultsDevice(const QString& name, QString path, struct iio_device* device,
			      struct iio_device* swiot, struct iio_context *context,
			      QWidget* parent = nullptr);
	~FaultsDevice();

	void update();

public Q_SLOTS:
	void resetStored();
	void updateExplanations();

Q_SIGNALS:
	void specialFaultsUpdated(int index, QString channelFunction);

private:
	Ui::FaultsDevice *ui;
	QWidget *m_faults_explanation;
	scopy::gui::SubsectionSeparator *m_subsectionSeparator;

	CommandQueue *m_cmdQueue;

	FaultsGroup* m_faultsGroup;
	QVector<QWidget*> m_faultExplanationWidgets;

	QString m_name;
	QString m_path;

	struct iio_device* m_device;
	struct iio_device* m_swiot;
	struct iio_channel* m_faultsChannel;
	struct iio_context* m_context;

	uint32_t m_faultNumeric;
	Command *m_readFaultCommand;
	QVector<Command*> m_deviceConfigCmds;
	QVector<Command*> m_functionConfigCmds;

	void initFaultExplanations();
	void connectSignalsAndSlots();
	void readFaults();
	void initSpecialFaults();

private Q_SLOTS:
	void updateMinimumHeight();
	void deviceConfigCmdFinished(scopy::Command *cmd);
	void functionConfigCmdFinished(scopy::Command *cmd);
};

} // scopy::swiot

#endif //FAULTSDEVICE_H
