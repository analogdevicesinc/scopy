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
 */

#ifndef AD9084_H
#define AD9084_H

#include "scopy-ad9084_export.h"
#include "ad9084channel.h"

#include <gui/tooltemplate.h>
#include <gui/widgets/toolbuttons.h>
#include <gui/animationpushbutton.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/filebrowserwidget.h>
#include <iiowidget.h>
#include <QWidget>
#include <iio.h>

namespace scopy {
namespace ad9084 {
class SCOPY_AD9084_EXPORT Ad9084 : public QWidget
{
	Q_OBJECT
public:
	Ad9084(struct iio_device *dev, QWidget *parent = nullptr);
	~Ad9084();

Q_SIGNALS:
	void triggerRead();

private:
	void scanChannels();
	bool extractChannelPaths(iio_channel *chn);
	void mapPathsUnique();
	QWidget *createMenu();
	void loadCfir(QString path);
	void loadPfir(QString path);
	QString readFile(QString file);

	struct iio_device *m_device;
	ToolTemplate *tool;
	GearBtn *settingsBtn;
	QPushButton *m_deviceName;
	QPushButton *m_rxChain;
	QPushButton *m_txChain;
	AnimationPushButton *m_refreshBtn;
	FileBrowserWidget *m_cfirFileBrowser;
	FileBrowserWidget *m_pfirFileBrowser;
	QVector<Ad9084Channel *> m_channelsRx;
	QVector<Ad9084Channel *> m_channelsTx;
	QSplitter *m_hSplitter;

	const QString settingsMenuId = "settings";

	using ChannelList = QList<QString>;	    // channels list
	using FddcMap = QMap<QString, ChannelList>; // FDDC map
	using CddcMap = QMap<QString, FddcMap>;
	using AdcMap = QMap<QString, CddcMap>;
	using SideMap = QMap<QString, AdcMap>;
	SideMap m_channelPaths;

	QList<QString> m_rx_fine_ddc_channel_names;
	QList<QString> m_rx_coarse_ddc_channel_names;
	QList<QString> m_tx_fine_duc_channel_names;
	QList<QString> m_tx_coarse_duc_channel_names;
};
} // namespace ad9084
} // namespace scopy
#endif // AD9084_H
