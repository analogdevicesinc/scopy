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

#ifndef CLIDETAILSVIEW_H
#define CLIDETAILSVIEW_H

#include <QWidget>
#include <QTextBrowser>
#include "iiostandarditem.h"

namespace scopy::debugger {
class CliDetailsView : public QWidget
{
	Q_OBJECT
public:
	explicit CliDetailsView(QWidget *parent = nullptr);
	void setIIOStandardItem(IIOStandardItem *item);
	void refreshView();

Q_SIGNALS:
	void addToWatchlist(IIOStandardItem *item);
	void removeFromWatchlist(IIOStandardItem *item);

private:
	void setupUi();
	void setupChannelAttr();
	void setupChannel();
	void setupDeviceAttr();
	void setupDevice();
	void setupContextAttr();
	void setupContext();

	QString tabs(int level);

	IIOStandardItem *m_currentItem;
	QTextBrowser *m_textBrowser;

	// used for creating the iio info view
	IIOStandardItem *m_channelIIOItem;
	IIOStandardItem *m_deviceIIOItem;
	IIOStandardItem *m_contextIIOItem;

	QString m_currentText;
	QString m_deviceAttrsString;

	int m_noCtxAttributes;
	int m_noDevices;
	int m_noDevAttributes;
	int m_noChannels;
	int m_noChnlAttributes;
	int m_globalLevel;
};
} // namespace scopy::debugger

#endif // CLIDETAILSVIEW_H
