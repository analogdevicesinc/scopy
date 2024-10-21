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

#ifndef DACADDON_H_
#define DACADDON_H_

#include <QWidget>
#include <QMap>
#include <menucontrolbutton.h>

namespace scopy {
namespace dac {
class DacAddon : public QWidget
{
	Q_OBJECT
public:
	DacAddon(QWidget *parent = nullptr);
	virtual ~DacAddon();
	virtual void enable(bool enable);
	virtual QMap<QString, MenuControlButton *> getChannelBtns();
	virtual QMap<QString, QWidget *> getChannelMenus();
	virtual void setRunning(bool running);
	virtual bool isRunning();
Q_SIGNALS:
	void running(bool enabled);
	void requestChannelMenu(QString uuid);

protected:
	QMap<QString, MenuControlButton *> m_channelBtns;
	QMap<QString, QWidget *> m_channelMenus;
	bool m_isRunning;
};
} // namespace dac
} // namespace scopy

#endif // DACADDON_H_
