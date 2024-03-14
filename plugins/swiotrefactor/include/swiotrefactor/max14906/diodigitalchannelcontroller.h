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

#ifndef SCOPY_DIODIGITALCHANNELCONTROLLER_H
#define SCOPY_DIODIGITALCHANNELCONTROLLER_H

#include "scopy-swiotrefactor_export.h"
#include "diodigitalchannel.h"
#include <iio.h>

#include <iioutil/commandqueue.h>

namespace scopy::swiotrefactor {
class SCOPY_SWIOTREFACTOR_EXPORT DioDigitalChannelController : public QWidget
{
	Q_OBJECT
public:
	explicit DioDigitalChannelController(struct iio_channel *channel, const QString &deviceName,
					     const QString &deviceType, CommandQueue *cmdQueue, QWidget *parent);
	~DioDigitalChannelController() override;

	DioDigitalChannel *getDigitalChannel() const;
private Q_SLOTS:
	void createWriteCurrentLimitCommand(int index);
	void createWriteRawCommand(bool value);
	void createWriteTypeCommand(int index);

	void readAvailableTypeCmdFinished(scopy::Command *cmd);
	void readTypeCmdFinished(scopy::Command *cmd);
	void readRawCmdFinished(scopy::Command *cmd);
	void readCurrentLimitCmdFinished(scopy::Command *cmd);
	void readCurrentLimitAvailableCmdFinished(scopy::Command *cmd);
	void writeCurrentLimitCmdFinished(scopy::Command *cmd);
	void writeTypeCmdFinished(scopy::Command *cmd);

private:
	DioDigitalChannel *m_digitalChannel;

	QString m_channelName;
	QString m_channelType; // output or input

	QString m_iioAttrAvailableTypes; // iio attribute
	QStringList m_availableTypes;

	QString m_iioAttrType; // iio attribute
	QString m_type;

	CommandQueue *m_cmdQueue;

	struct iio_channel *m_channel;
};

} // namespace scopy::swiotrefactor

#endif // SCOPY_DIODIGITALCHANNELCONTROLLER_H
