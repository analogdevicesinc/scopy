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

#ifndef CHANNEL_MANAGER_HPP
#define CHANNEL_MANAGER_HPP

#include "scopy-m2k-gui_export.h"
#include "channel_widget.hpp"
#include "customPushButton.h"

#include <QLabel>
#include <QScrollArea>
#include <QStackedWidget>
#include <QWidget>

namespace scopy {
namespace m2kgui {

enum class ChannelsPositionEnum
{
	VERTICAL,
	HORIZONTAL
};

class SCOPY_M2K_GUI_EXPORT ChannelManager : public QWidget
{
	Q_OBJECT

public:
	explicit ChannelManager(ChannelsPositionEnum position = ChannelsPositionEnum::HORIZONTAL,
				QWidget *parent = nullptr);
	~ChannelManager();

	void build(QWidget *parent);
	void insertAddBtn(QWidget *menu, bool dockable);

	ChannelWidget *buildNewChannel(int chId, bool deletable, bool simplefied, QColor color, const QString &fullName,
				       const QString &shortName);
	void removeChannel(ChannelWidget *ch);
	QList<ChannelWidget *> getEnabledChannels();

	CustomPushButton *getAddChannelBtn();
	QList<ChannelWidget *> getChannelsList();
	int getChannelsCount();

	void setChannelAlignment(ChannelWidget *ch, Qt::Alignment alignment);
	void setChannelIdVisible(bool visible);
	const QString &getToolStatus() const;
	void setToolStatus(const QString &newToolStatus);
	void enableSwitchButton(bool en);
	int getChannelID(ChannelWidget *ch);
	int getSelectedChannel();
	const ChannelWidget *getChannelAt(int id);
	void updatePosition(ChannelsPositionEnum position);
public Q_SLOTS:
	void changeParent(QWidget *newParent);
	void toggleChannelManager(bool toggled);

	void onChannelSelected(bool toggled);
	void onChannelEnabled(bool toggled);
	void onChannelDeleted();

Q_SIGNALS:
	void configureAddBtn(QWidget *menu, bool dockable);
	void positionChanged(ChannelsPositionEnum position);
	void channelManagerToggle(bool toggled);

	void selectedChannel(int id, bool toggled);
	void enabledChannel(int id, bool toggled);
	void deletedChannel(QString name);

private:
	QWidget *m_parent;
	QWidget *header;
	QScrollArea *m_scrollArea;
	QWidget *m_channelsWidget;
	bool m_hasAddBtn;
	QPushButton *m_switchBtn;
	CustomPushButton *m_addChannelBtn;
	ChannelsPositionEnum m_position;
	QList<ChannelWidget *> m_channelsList;
	bool m_channelIdVisible;
	QPushButton *toggleChannels;
	bool channelManagerToggled;
	QLabel *toolStatus;
	int m_maxChannelWidth;
	int m_minChannelWidth;
	int m_selectedChannel;
};
} // namespace m2kgui
} // namespace scopy

#endif // CHANNEL_MANAGER_HPP
