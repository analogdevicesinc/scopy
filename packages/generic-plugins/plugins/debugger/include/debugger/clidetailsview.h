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
#include <iio.h>
#include "iiostandarditem.h"

namespace scopy::debugger {
class CliDetailsView : public QWidget
{
	Q_OBJECT
public:
	explicit CliDetailsView(QWidget *parent = nullptr);
	void setIIOStandardItem(IIOStandardItem *item);
	void refreshView();

private:
	void setupUi();

	void appendContextInfo(struct iio_context *ctx);
	void appendDeviceInfo(struct iio_device *dev);
	void appendChannelInfo(struct iio_channel *ch);

	QString readDeviceAttr(struct iio_device *dev, const char *attr, bool isDebug);
	QString readChannelAttr(struct iio_channel *ch, const char *attr);
	QString channelFormatString(struct iio_channel *ch);
	QString triggerStatusString(struct iio_device *dev);

	QString tabs(int level);

	IIOStandardItem *m_currentItem;
	QTextBrowser *m_textBrowser;
	QString m_currentText;
	int m_globalLevel;
	bool m_includeDebugAttrs;
};
} // namespace scopy::debugger

#endif // CLIDETAILSVIEW_H
