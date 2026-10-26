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

#include "scopy-core_export.h"

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
class SourceBlock;
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
class SigrokCliBackendFactory;
class SigrokCliCatalog;
} // namespace decoder
namespace adc {

class AcqInstrument;
class AcqPlotManager;
class FileSourceBlock;

// Composition root for one AcqInstrument: builds its blocks, plots, trigger and decoders
// and wires them together. The device sources come from the context it is given.
class SCOPY_CORE_EXPORT AcqInstrumentController : public QObject
{
	Q_OBJECT
public:
	explicit AcqInstrumentController(ToolMenuEntry *tme, QObject *parent = nullptr);
	~AcqInstrumentController() override;

	// Call once, after ui() has a parent. `ctx` must outlive the instrument; null leaves
	// only the file and snapshot sources.
	void init(scopy::component::Context *ctx = nullptr);

	// Stop the engine if running. Safe before init() and more than once.
	void stop();

	AcqInstrument *ui() const;
	AcqPlotManager *plots() const;
	scopy::decoder::DecoderPanel *decoders() const;

	// Single is one cycle, or with the trigger on, one fire: run until it stops itself.
	void single();

protected:
	// One step each of init(), in this order. An instrument with a fixed block set
	// overrides the ones it builds differently and empties the ones it does not want;
	// the defaults are the generic acquisition instrument's.
	virtual void setupBlocks(scopy::component::Context *ctx);
	virtual void setupProcessors();
	virtual void setupPlots();
	virtual void setupTrigger();
	virtual void setupDecoders();
	virtual void setupAnalysisPanel();

	QPointer<ToolMenuEntry> m_tme;
	QPointer<AcqInstrument> m_ui;
	QPointer<AcqPlotManager> m_plots;

private:
	// A plain rail row opening `body` as its right-menu page. With a block, the row's
	// checkbox also follows the block's enable state.
	void addBlockRow(MenuSectionCollapseWidget *group, QWidget *body, const QString &label,
			 const QString &pageTitle, const QString &menuId, scopy::acq::Block *block = nullptr);
	void addSource(MenuSectionCollapseWidget *group, scopy::acq::SourceBlock *src);
	void applyPlotSize(int n);
	void refreshKeyPickers(const QList<scopy::acq::DataKey> &keys);

	QPointer<FileSourceBlock> m_fileSrc;

	// Blocks are parented to the engine; everything else here to the instrument or this.
	scopy::acq::GenalyzerFFTProcessor *m_fftProc{nullptr};
	scopy::acq::TriggerProcessor *m_trigProc{nullptr};

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
	std::unique_ptr<scopy::decoder::SigrokCliCatalog> m_decoderCatalog;
	std::unique_ptr<scopy::decoder::SigrokCliBackendFactory> m_decoderFactory;
	std::unique_ptr<scopy::decoder::DecoderManager> m_decoderMgr;
	QPointer<scopy::decoder::DecoderPanel> m_decoderPanel;
};

} // namespace adc
} // namespace scopy

#endif // ACQINSTRUMENTCONTROLLER_H
