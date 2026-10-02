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

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>

#include <pluginbase/toolmenuentry.h>
// Not forward-declared: DataKey is a struct passed by value below.
#include <core/acq_engine/DataKey.h>

#include <memory>

struct iio_context;

namespace scopy {
class GenalyzerPanel;
class MenuCombo;
class MenuControlButton;
class MenuSectionCollapseWidget;

namespace acq {
class Block;
class GenalyzerFFTProcessor;
class TriggerBinder;
class TriggerMarker;
class TriggerProcessor;
class TriggerProcessorWidget;
} // namespace acq
namespace decoder {
class IDecoderBackendFactory;
class IDecoderCatalog;
} // namespace decoder
namespace adc {

class AcqInstrument;
class AcqPlotManager;
class DecoderManager;
class DecoderPanel;

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

	// The one rail entry a Sources/Processors row gets: a plain row opening `body` as its
	// right-menu page. Plain and not expandable — nothing nests under it, and an expandable
	// row would draw its collapse arrow with nothing behind it. Only a plot's row nests,
	// because only a plot has children.
	//
	// The widget overload is for a page that is not one block's settings widget: the decoder
	// stack's panel, which stands for blocks the manager creates on demand.
	MenuControlButton *addBlockRow(MenuSectionCollapseWidget *group, QWidget *body, const QString &label,
				       const QString &pageTitle, const QString &menuId);
	// The same row plus the checkbox↔enable wiring, which needs a block to toggle.
	MenuControlButton *addBlockRow(MenuSectionCollapseWidget *group, scopy::acq::Block *block, const QString &label,
				       const QString &pageTitle, const QString &menuId);

	// The plot manager and its wiring. Creates no plot and no channel: every view is built
	// from the rail. Runs after setupBlocks() so the block keys exist to point at.
	void setupPlots();

	// The genalyzer results table, in the slot right of the plot. Shown only while analysis
	// is enabled.
	void setupAnalysisPanel();

	// The software trigger, as a Processors row over a right-menu page. Disabled until the
	// reader enables it. After setupPlots(): the fire window is the plot window.
	void setupTrigger();

	// The decoder stack: catalog, backend factory, manager and panel. No plot and no overlay
	// — a decoder's annotations are drawn as ordinary ReprKind::Annotations channels on
	// whichever plot the reader picks. After setupPlots(), which supplies the decode window.
	void setupDecoders();

	// Pluto's RX default. The source doesn't read the rate back, so the FFT has to be told.
	const double m_kPlutoSampleRate;

	ToolMenuEntry          *m_tme{nullptr};
	QPointer<AcqInstrument> m_ui;

	// Parented to the engine, kept for the analysis panel and the fallback rate.
	scopy::acq::GenalyzerFFTProcessor *m_fftProc{nullptr};

	// Parented to the instrument.
	QPointer<AcqPlotManager> m_plots;
	QPointer<GenalyzerPanel> m_genalyzerPanel;

	// Trigger. The processor is parented to the engine; the widget to its settings body.
	scopy::acq::TriggerProcessor		     *m_trigProc{nullptr};
	QPointer<scopy::acq::TriggerBinder>	      m_trigBinder;
	QPointer<scopy::acq::TriggerProcessorWidget> m_trigWidget;

	// cycleComplete → the plot manager, held so enabling the trigger can drop it: a
	// triggered run draws the cycle that fired, not every cycle the worker completes.
	QMetaObject::Connection m_cycleConn;

	// The draggable bar on the plot. Parented here, because it is built from the processor;
	// handed to the plot manager, which resolves and owns the question of which axis it rides.
	//
	// It rides an axis a channel actually draws against, borrowed and never owned. A private
	// hidden axis pinned to [0, plotSize-1] would make the bar's position *be* the sample
	// index, but PlotNavigator gives every QwtAxisId its own zoomer, so a zoom would move it
	// and the curve's axis to two different scale ranges and the bar would drift off the
	// feature it was aimed at. The sample index is recovered arithmetically instead.
	QPointer<scopy::acq::TriggerMarker> m_trigMarker;

	// The reader's X-source pick. Every key is offered, including ones no channel plots on X
	// yet: a pick that resolves to no axis hides the bar until one does.
	QPointer<MenuCombo> m_trigAxisCombo;

	// Decoders. Catalog and factory are owned here and must outlive the manager — the
	// factory holds the catalog and the manager holds the factory, both non-owning.
	std::unique_ptr<scopy::decoder::IDecoderCatalog>       m_decoderCatalog;
	std::unique_ptr<scopy::decoder::IDecoderBackendFactory> m_decoderFactory;
	QPointer<DecoderManager> m_decoderMgr;
	QPointer<DecoderPanel>   m_decoderPanel;
};

} // namespace adc
} // namespace scopy

#endif // ACQINSTRUMENTCONTROLLER_H
