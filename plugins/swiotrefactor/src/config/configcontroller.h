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

#ifndef CONFIGCONTROLLER_H
#define CONFIGCONTROLLER_H

#include <QObject>
#include "configmodel.h"
#include "src/config/configchannelview.h"

namespace scopy::swiotrefactor {

class ConfigController : public QObject
{
	Q_OBJECT
public:
	explicit ConfigController(ConfigChannelView *channelView, ConfigModel *model, int channelId);

	~ConfigController();

	void connectSignalsAndSlots();
	void initChannelView();

Q_SIGNALS:
	void clearDrawArea();
	void deviceReadingComplete();
	void functionAvailableReadingComplete();

private:
	ConfigChannelView *m_channelsView;
	ConfigModel *m_model;
	int m_channelId;
};

} // namespace scopy::swiotrefactor

#endif // CONFIGCONTROLLER_H
