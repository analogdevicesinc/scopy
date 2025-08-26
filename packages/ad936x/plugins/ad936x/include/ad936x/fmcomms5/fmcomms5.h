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

#ifndef FMCOMMS5_H
#define FMCOMMS5_H

#include "scopy-ad936x_export.h"
#include <QBoxLayout>
#include <QWidget>
#include <tooltemplate.h>

#include <iio-widgets/iiowidgetbuilder.h>

#include <animatedrefreshbtn.h>
#include <ad936xhelper.h>

namespace scopy {
namespace ad936x {
class SCOPY_AD936X_EXPORT FMCOMMS5 : public QWidget
{
	Q_OBJECT
public:
	explicit FMCOMMS5(iio_context *ctx, QWidget *parent = nullptr);
	~FMCOMMS5();

Q_SIGNALS:
	void readRequested();

private:
	iio_context *m_ctx = nullptr;
	ToolTemplate *m_tool;
	QVBoxLayout *m_mainLayout;
	QWidget *m_controlsWidget;
	QWidget *m_blockDiagramWidget;
	AnimatedRefreshBtn *m_refreshButton;

	QWidget *generateRxChainWidget(iio_device *dev, QString title, QWidget *parent);
	QWidget *generateTxChainWidget(iio_device *dev, QString title, QWidget *parent);

	AD936xHelper *m_helper;
};
} // namespace ad936x
} // namespace scopy
#endif // FMCOMMS5_H
