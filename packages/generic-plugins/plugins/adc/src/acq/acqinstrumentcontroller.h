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

#ifndef ACQINSTRUMENTCONTROLLER_H
#define ACQINSTRUMENTCONTROLLER_H

#include <QObject>
#include <QPointer>
#include <QString>

#include <pluginbase/toolmenuentry.h>

struct iio_context;

namespace scopy {
class CollapsableMenuControlButton;
class GenalyzerPanel;
class MenuSectionCollapseWidget;

namespace acq {
class Block;
class GenalyzerFFTProcessor;
class SourceBlock;
} // namespace acq
namespace adc {

class AcqInstrument;
class AcqPlotManager;

// Composition root for one AcqInstrument. Sources come from AcqSourceRegistry, so which
// ones appear is decided by what the opened context holds and not by anything named here.
class AcqInstrumentController : public QObject
{
	Q_OBJECT
public:
	explicit AcqInstrumentController(ToolMenuEntry *tme, QObject *parent = nullptr);
	~AcqInstrumentController() override;

	// Build the instrument, register blocks and wire it to the tool menu entry. Call once.
	// `ctx` may be null, in which case every device-backed source reports itself unavailable
	// and is skipped.
	void init(iio_context *ctx = nullptr);

	// Stop the engine if running. Safe before init() and more than once.
	void stop();

	AcqInstrument *ui() const;

private:
	// One pass over the registry: build each source, keep the available ones, add the rail row.
	void setupBlocks(iio_context *ctx);

	// The genalyzer FFT over Pluto's I/Q. Not part of the source registry — a processor
	// watches keys no single source owns. After setupBlocks().
	void setupProcessors();

	// The one rail entry every block gets, source or processor: an expandable row opening
	// the block's settings page, with the source's channel rows nested under it.
	CollapsableMenuControlButton *addBlockRow(MenuSectionCollapseWidget *group, scopy::acq::Block *block,
						  const QString &label, const QString &pageTitle,
						  const QString &menuId);

	// One nested row per channel the source declares, rebuilt on channelsChanged() — a
	// source can gain or lose channels after onStart() has read the device.
	void addSourceChannelRows(CollapsableMenuControlButton *parentRow, scopy::acq::SourceBlock *src);

	// The plot manager and its wiring. Creates no plot and no channel: every view is built
	// from the rail. Runs after setupBlocks() so the block keys exist to point at.
	void setupPlots();

	// The genalyzer results table, in the slot right of the plot. Shown only while analysis
	// is enabled.
	void setupAnalysisPanel();

	// Pluto's RX default. The source doesn't read the rate back, so the FFT has to be told.
	const double m_kPlutoSampleRate;

	ToolMenuEntry          *m_tme{nullptr};
	QPointer<AcqInstrument> m_ui;

	// Parented to the engine, kept for the analysis panel and the fallback rate.
	scopy::acq::GenalyzerFFTProcessor *m_fftProc{nullptr};

	// Parented to the instrument.
	QPointer<AcqPlotManager> m_plots;
	QPointer<GenalyzerPanel> m_genalyzerPanel;
};

} // namespace adc
} // namespace scopy

#endif // ACQINSTRUMENTCONTROLLER_H
