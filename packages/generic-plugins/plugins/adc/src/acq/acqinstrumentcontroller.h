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

#include <pluginbase/toolmenuentry.h>

struct iio_context;

namespace scopy {
class CollapsableMenuControlButton;

namespace acq {
class GenalyzerFFTProcessor;
class SourceBlock;
} // namespace acq
namespace adc {

namespace sim {
class PlutoIIOSource;
}

class AcqInstrument;
class AcqPlotManager;

// Composition root for one AcqInstrument.
//
// Blocks are constructed and registered here — the instrument itself never names
// one. Currently a PlutoSDR source and a genalyzer FFT over its I/Q, for testing.
// Plots and channels are built here too, for the same reason: the instrument owns
// the engine and the store, not a view of them.
class AcqInstrumentController : public QObject
{
	Q_OBJECT
public:
	explicit AcqInstrumentController(ToolMenuEntry *tme, QObject *parent = nullptr);
	~AcqInstrumentController() override;

	// Build the instrument, register blocks and wire it to the tool menu entry.
	// Call once. `ctx` may be null, in which case no blocks are registered — the
	// only source here needs real hardware.
	void init(iio_context *ctx = nullptr);

	// Stop the engine if running. Safe before init() and more than once.
	void stop();

	AcqInstrument *ui() const;

private:
	// Registers the test pipeline on the instrument's engine.
	void setupBlocks(iio_context *ctx);

	// One nested row per channel the source declares, each with a switch bound to the
	// source's own enable state. Rebuilt on channelsChanged(), which is why the rows
	// are not written out by hand at composition time — a source can gain or lose
	// channels after onStart() has read the device.
	void addSourceChannelRows(CollapsableMenuControlButton *parentRow, scopy::acq::SourceBlock *src);

	// Builds the plot manager, makes it the center widget, and adds one channel per
	// key the pipeline publishes. Runs after setupBlocks() so the block keys exist to
	// point at.
	void setupPlots();

	ToolMenuEntry          *m_tme{nullptr};
	QPointer<AcqInstrument> m_ui;

	// Parented to the engine, so listed here only for the settings pages.
	sim::PlutoIIOSource            *m_plutoSrc{nullptr};
	scopy::acq::GenalyzerFFTProcessor *m_fftProc{nullptr};

	// Parented to the instrument.
	QPointer<AcqPlotManager> m_plots;
};

} // namespace adc
} // namespace scopy

#endif // ACQINSTRUMENTCONTROLLER_H
