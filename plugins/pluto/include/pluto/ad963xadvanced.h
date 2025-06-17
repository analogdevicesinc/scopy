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

#ifndef AD963XADVANCED_H
#define AD963XADVANCED_H

#include <QBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <animatedrefreshbtn.h>
#include <tooltemplate.h>
#include <iio.h>

#include "auxadcdaciowidget.h"
#include "elnawidget.h"
#include "ensmmodeclockswidget.h"
#include "gainwidget.h"
#include "rssiwidget.h"
#include "txmonitorwidget.h"
#include "miscwidget.h"
#include "bistwidget.h"

namespace scopy {
namespace pluto {
class AD936XAdvanced : public QWidget
{
	Q_OBJECT
public:
	explicit AD936XAdvanced(iio_context *ctx, QWidget *parent = nullptr);
	~AD936XAdvanced();

Q_SIGNALS:
	void readRequested();

private:
	iio_context *m_ctx = nullptr;
	ToolTemplate *m_tool;
	QVBoxLayout *m_mainLayout;
	AnimatedRefreshBtn *m_refreshButton;
	EnsmModeClocksWidget *m_ensmModeClocks;
	ElnaWidget *m_elna;
	RssiWidget *m_rssi;
	GainWidget *m_gainWidget;
	TxMonitorWidget *m_txMonitor;
	AuxAdcDacIoWidget *m_auxAdcDacIo;
	MiscWidget *m_misc;
	BistWidget *m_bist;
};
} // namespace pluto
} // namespace scopy
#endif // AD963XADVANCED_H
