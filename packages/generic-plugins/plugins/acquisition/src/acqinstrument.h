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

#include <QWidget>

namespace scopy {
class InstrumentTemplate;
class MenuCombo;
namespace gui {
class MenuSpinbox;
}
namespace acq {
class AcquisitionEngine;
class DataStore;
class PipelineInspector;
} // namespace acq

namespace adc {

class DataStoreViewer;
class LogView;

// An acquisition instrument over InstrumentTemplate: the engine, its store, the run
// controls and the debug tabs, with no blocks and no plots. Whoever builds one registers
// the blocks and wires the Single button, since what one acquisition means depends on the
// blocks (with a trigger it is one fire, not one cycle).
//
// Every engine signal arrives from the worker thread, so every connection to one here is
// queued; the signals below re-emit them on the GUI thread.
class AcqInstrument : public QWidget
{
	Q_OBJECT
public:
	explicit AcqInstrument(QWidget *parent = nullptr);
	~AcqInstrument() override;

	// Both live as long as this widget.
	scopy::acq::AcquisitionEngine *engine() const { return m_engine; }
	scopy::acq::DataStore *store() const { return m_store; }

	InstrumentTemplate *shell() const { return m_it; }

	// The "Pipeline" tab, for an owner whose blocks the engine does not list (decoders).
	scopy::acq::PipelineInspector *pipeline() const { return m_pipeline; }

	// Set through the controls, which apply them to the engine, so the two stay in step.
	void setMode(int mode);
	void setBufferSize(int n);

public Q_SLOTS:
	// Safe in any state; ignored while running. The buttons follow the engine's own
	// started/stopped signals, so a refused or self-terminated run cannot desync them.
	void run();    // until stop()
	void single(); // exactly one engine cycle
	void stop();

Q_SIGNALS:
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

	// cycles == 0 runs until stop().
	void start(unsigned int cycles);

	// Reflects the engine's run state onto the controls.
	void setRunning(bool running);

	InstrumentTemplate *m_it{nullptr};

	scopy::acq::DataStore *m_store{nullptr};
	scopy::acq::AcquisitionEngine *m_engine{nullptr};

	// Disabled while running: the engine latches both at start.
	MenuCombo *m_modeCombo{nullptr};
	scopy::gui::MenuSpinbox *m_bufferSpin{nullptr};

	scopy::acq::PipelineInspector *m_pipeline{nullptr};
	DataStoreViewer *m_storeViewer{nullptr};
	LogView *m_logView{nullptr};
};

} // namespace adc
} // namespace scopy

#endif // ACQINSTRUMENT_H
