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

#ifndef ACQINSTRUMENT_H
#define ACQINSTRUMENT_H

#include <QLabel>
#include <QWidget>

#include <core/acq_engine/AcquisitionEngine.h>
#include <gui/instrumenttemplate.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuspinbox.h>

namespace scopy {
namespace acq {
class DataStore;
}
namespace decoder {
class DecoderLogger;
}

namespace adc {

class DataStoreViewer;
class LogView;
class PipelineInspector;

// An acquisition instrument over InstrumentTemplate: engine, store, run
// controls and the debug panel — with no blocks and no plots.
//
// This is deliberately block-free. It owns the DataStore and the
// AcquisitionEngine and everything that is true of *any* pipeline — run/single/
// stop, mode, buffer size, fps, the four debug tabs — and nothing that names a
// concrete source, processor, plot or channel. Blocks get registered by whoever
// builds one of these; the rail and the menus start empty and are meant to be
// filled from engine->sources()/processors() rather than from hardcoded rows.
//
// Threading: every AcquisitionEngine and DataStore signal is emitted from the
// worker thread, so every connection to one here is queued.
class AcqInstrument : public QWidget
{
	Q_OBJECT
public:
	explicit AcqInstrument(QWidget *parent = nullptr);
	~AcqInstrument() override;

	// The engine and its store, for an owner registering blocks. Both live as
	// long as this widget.
	scopy::acq::AcquisitionEngine *engine() const { return m_engine; }
	scopy::acq::DataStore *store() const { return m_store; }

	// The shell, for an owner adding rail rows, menu pages or a PlotManager.
	InstrumentTemplate *shell() const { return m_it; }

	// The sink behind the "Decoder logs" tab. Handed to a decoder catalog or
	// backend factory when one gets built; exists up front so the tab has a
	// source whether or not anything decodes. Does not forward to the engine —
	// decoder chatter would otherwise drown the acquisition log.
	scopy::decoder::DecoderLogger *decoderLogger() const { return m_decoderLogger; }

	// Adds a tab to the debug popup, next to the four built in here.
	void addDebugTab(QWidget *w, const QString &title);

public Q_SLOTS:
	// Start/stop the engine. Safe to call regardless of current state; the run
	// button follows the engine's own started/stopped signals rather than the
	// click, so a refused or self-terminated run doesn't desync the UI.
	void run();
	void single();
	void stop();

Q_SIGNALS:
	// Mirrors the engine's own signals on the GUI thread, for an owner that
	// wants to refresh plots without connecting to the worker itself.
	void started();
	void stopped();
	void cycleComplete();

private Q_SLOTS:
	void onStarted();
	void onStopped();
	void onCycleComplete();

private:
	void setupEngine();
	void setupTopRail();
	void setupDebugTabs();

	// True while the engine is mid-run. Only used to keep the run button and the
	// engine controls consistent — the engine itself is the authority.
	void setRunning(bool running);

	InstrumentTemplate *m_it{nullptr};

	scopy::acq::DataStore         *m_store{nullptr};
	scopy::acq::AcquisitionEngine *m_engine{nullptr};
	scopy::decoder::DecoderLogger *m_decoderLogger{nullptr};

	// Engine controls in the top rail. Both are disabled while running: they write
	// the engine's own fields, and the worker reads them between cycles.
	MenuCombo        *m_modeCombo{nullptr};
	gui::MenuSpinbox *m_bufferSpin{nullptr};

	// Debug tabs.
	PipelineInspector *m_pipeline{nullptr};
	DataStoreViewer   *m_storeViewer{nullptr};
	LogView           *m_logView{nullptr};
	LogView           *m_decoderLogView{nullptr};
};

} // namespace adc
} // namespace scopy

#endif // ACQINSTRUMENT_H
