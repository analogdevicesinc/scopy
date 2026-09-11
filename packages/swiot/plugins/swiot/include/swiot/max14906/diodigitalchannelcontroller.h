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

#include "diodigitalchannel.h"

namespace scopy {
namespace component {
class Channel;
class Attribute;
} // namespace component

namespace swiot {
class DioDigitalChannelController : public QWidget
{
	Q_OBJECT
public:
	explicit DioDigitalChannelController(component::Channel *channel, const QString &deviceName,
					     const QString &deviceType, QWidget *parent);
	~DioDigitalChannelController() override;

	DioDigitalChannel *getDigitalChannel() const;
private Q_SLOTS:
	void createWriteCurrentLimitCommand(int index);
	void createWriteRawCommand(bool value);
	void createWriteTypeCommand(int index);

private:
	void initChannelAttributes();

	DioDigitalChannel *m_digitalChannel;

	QString m_channelName;
	QString m_channelType; // output or input

	QStringList m_availableTypes;
	QString m_type;

	component::Channel *m_channel;
	component::Attribute *m_typeAttr;
	component::Attribute *m_rawAttr;
	component::Attribute *m_currentLimitAttr;
};

} // namespace swiot
} // namespace scopy

#endif // SCOPY_DIODIGITALCHANNELCONTROLLER_H
