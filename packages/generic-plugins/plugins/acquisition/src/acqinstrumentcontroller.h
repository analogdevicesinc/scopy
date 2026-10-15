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
#include <core/acq_engine/datakey.h>

#include <memory>

namespace scopy {
class GenalyzerPanel;
class MenuCombo;
class MenuControlButton;
class MenuSectionCollapseWidget;

namespace acq {
class Block;
class GenalyzerFFTProcessor;
class TriggerMarker;
class TriggerProcessor;
class TriggerProcessorWidget;
} // namespace acq
namespace component {
class Context;
} // namespace component
namespace decoder {
class DecoderLogger;
class DecoderManager;
class DecoderPanel;
class IDecoderBackendFactory;
class IDecoderCatalog;
} // namespace decoder
namespace adc {

class AcqInstrument;
class AcqPlotManager;

// Composition root for one AcqInstrument: builds its blocks, plots, trigger and decoders
// and wires them together. Sources come from AcqSourceRegistry, so which ones appear is
// decided by the opened context.
class AcqInstrumentController : public QObject
{
	Q_OBJECT
public:
	explicit AcqInstrumentController(ToolMenuEntry *tme, QObject *parent = nullptr);
	~AcqInstrumentController() override;

	// Call once, after ui() has a parent. `ctx` must outlive the instrument; null makes
	// every device-backed source unavailable.
	void init(scopy::component::Context *ctx = nullptr);

	// Stop the engine if running. Safe before init() and more than once.
	void stop();

	AcqInstrument *ui() const;
	AcqPlotManager *plots() const;
	scopy::decoder::DecoderPanel *decoders() const;

	// Single is one cycle, or with the trigger on, one fire: run until it stops itself.
	void single();

private:
	void setupBlocks(scopy::component::Context *ctx);
	void setupProcessors();
	void setupPlots();
	void setupTrigger();
	void setupDecoders();
	void setupAnalysisPanel();

	// A plain rail row opening `body` as its right-menu page. The block overload also
	// binds the row's checkbox to the block's enable state.
	MenuControlButton *addBlockRow(MenuSectionCollapseWidget *group, QWidget *body, const QString &label,
				       const QString &pageTitle, const QString &menuId);
	MenuControlButton *addBlockRow(MenuSectionCollapseWidget *group, scopy::acq::Block *block, const QString &label,
				       const QString &pageTitle, const QString &menuId);

	QPointer<ToolMenuEntry> m_tme;
	QPointer<AcqInstrument> m_ui;
	bool m_initialized{false};

	// Blocks are parented to the engine; everything else here to the instrument or this.
	scopy::acq::GenalyzerFFTProcessor *m_fftProc{nullptr};
	scopy::acq::TriggerProcessor *m_trigProc{nullptr};

	QPointer<AcqPlotManager> m_plots;
	QPointer<GenalyzerPanel> m_genalyzerPanel;

	// The one-shot stop armed by a triggered Single.
	QMetaObject::Connection m_singleShot;
	QPointer<scopy::acq::TriggerProcessorWidget> m_trigWidget;
	// The draggable bar on the plot; the plot manager decides which axis it rides.
	QPointer<scopy::acq::TriggerMarker> m_trigMarker;
	QPointer<MenuCombo> m_trigAxisCombo;

	// Declared in borrow order — the factory borrows the catalog, the manager the factory,
	// all of them the logger — so members destroy each before what it borrows.
	std::unique_ptr<scopy::decoder::DecoderLogger> m_decoderLogger;
	std::unique_ptr<scopy::decoder::IDecoderCatalog> m_decoderCatalog;
	std::unique_ptr<scopy::decoder::IDecoderBackendFactory> m_decoderFactory;
	std::unique_ptr<scopy::decoder::DecoderManager> m_decoderMgr;
	QPointer<scopy::decoder::DecoderPanel> m_decoderPanel;
};

} // namespace adc
} // namespace scopy

#endif // ACQINSTRUMENTCONTROLLER_H
