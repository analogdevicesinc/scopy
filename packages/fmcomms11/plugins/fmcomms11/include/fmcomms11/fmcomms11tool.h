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
 *
 */

#ifndef FMCOMMS11TOOL_H
#define FMCOMMS11TOOL_H

#include "scopy-fmcomms11_export.h"
#include <QBoxLayout>
#include <QWidget>
#include <tooltemplate.h>

#include <iio-widgets/iiowidgetbuilder.h>

#include <animatedrefreshbtn.h>

namespace scopy {
class IIOWidgetGroup;
namespace fmcomms11 {

class SCOPY_FMCOMMS11_EXPORT Fmcomms11Tool : public QWidget
{
	Q_OBJECT
public:
	Fmcomms11Tool(iio_context *ctx, IIOWidgetGroup *group = nullptr, QWidget *parent = nullptr);
	~Fmcomms11Tool();

Q_SIGNALS:
	void readRequested();

private:
	QWidget *generateRxSection(iio_device *adcDev, iio_device *vgaDev, QWidget *parent);
	QWidget *generateTxSection(iio_device *dacDev, iio_device *attnDev, QWidget *parent);
	QWidget *generateDdsToneWidget(iio_channel *chn, const QString &title, QWidget *parent);
	QWidget *generateLoSection(iio_device *pllDev, QWidget *parent);
	QWidget *generateLoChannelWidget(iio_device *dev, const QString &channelId, const QString &title,
					 QWidget *parent);

	iio_context *m_ctx = nullptr;
	IIOWidgetGroup *m_group = nullptr;
	ToolTemplate *m_tool;
	QVBoxLayout *m_mainLayout;
	AnimatedRefreshBtn *m_refreshButton;
};
} // namespace fmcomms11
} // namespace scopy
#endif // FMCOMMS11TOOL_H
