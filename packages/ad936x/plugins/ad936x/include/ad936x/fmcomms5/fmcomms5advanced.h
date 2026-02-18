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

#ifndef FMCOMMS5ADVANCED_H
#define FMCOMMS5ADVANCED_H

#include "scopy-ad936x_export.h"
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
#include "fmcomms5/fmcomms5tab.h"

namespace scopy {
class IIOWidgetManager;
namespace ad936x {
class SCOPY_AD936X_EXPORT Fmcomms5Advanced : public QWidget
{
	Q_OBJECT
public:
	explicit Fmcomms5Advanced(iio_context *ctx, IIOWidgetManager *mgr = nullptr, QWidget *parent = nullptr);
	~Fmcomms5Advanced();

Q_SIGNALS:
	void readRequested();

private:
	void init();

	QPushButton *m_ensmModeClocksBtn = nullptr;
	QPushButton *m_eLnaBtn = nullptr;
	QPushButton *m_rssiBtn = nullptr;
	QPushButton *m_gainBtn = nullptr;
	QPushButton *m_txMonitorBtn = nullptr;
	QPushButton *m_auxAdcDacIioBtn = nullptr;
	QPushButton *m_miscBtn = nullptr;
	QPushButton *m_bistBtn = nullptr;
	QPushButton *m_fmcomms5Btn = nullptr;

	QPushButton *m_syncBtn = nullptr;
	void ad9361MultichipSync();

	iio_context *m_ctx = nullptr;
	IIOWidgetManager *m_mgr = nullptr;
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
	Fmcomms5Tab *m_fmcomms5;

	iio_device *m_mainDevice = nullptr;
	iio_device *m_secondDevice = nullptr;
	QStackedWidget *m_centralWidget = nullptr;

	bool m_isToolInitialized;
	void showEvent(QShowEvent *event) override;
};

} // namespace ad936x
} // namespace scopy
#endif // FMCOMMS5ADVANCED_H
