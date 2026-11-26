/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#pragma once

#include "m2kinstrumentcontroller.h"

#include <QList>
#include <QMap>
#include <QPointer>
#include <QString>

#include <functional>

class QAbstractButton;
class QLabel;
class QScrollBar;
class QTimer;

namespace scopy {
class MenuCombo;
class MenuControlButton;
class MenuOnOffSwitch;
class PlotAxisHandle;
namespace component {
class Device;
}
namespace gui {
class MenuSpinbox;
}
namespace adc {
class AcqAnnotationChannel;
class AcqDigitalChannel;
class AcqPlot;
} // namespace adc
} // namespace scopy

namespace scopy::adalm2000 {
class M2kSource;

// The ADALM2000 Logic Analyzer: 16 logic tracks and their decodes on one time axis. A plot
// window wider than the buffer streams: one trigger, then gapless buffers stitched together.
class M2kLogicController : public M2kInstrumentController
{
	Q_OBJECT
public:
	static constexpr int CHANNELS = 16;

	M2kLogicController(ToolMenuEntry *tme, M2kContext *ctx, const QString &uri, QObject *parent = nullptr);
	~M2kLogicController() override;

	void single() override;

	void setChannelEnabled(int pin, bool on);
	bool isChannelEnabled(int pin) const;
	void setChannelName(int pin, const QString &name);
	QString channelName(int pin) const;
	void setTriggerCondition(int pin, int index);
	int triggerCondition(int pin) const;

	bool exportData(const QString &path) const;

	bool eventFilter(QObject *watched, QEvent *event) override;

protected:
	void setupBlocks(scopy::component::Context *ctx) override;
	void setupProcessors() override {}
	void setupPlots() override;
	void setupTrigger() override {}
	void setupDecoders() override;
	void setupAnalysisPanel() override {}

private:
	friend class Adalm2000Logic_API;

	void setupCenter();
	void setupChannels();
	void setupTopRail();
	QWidget *buildGeneralPage();
	QWidget *buildTriggerPage();
	QWidget *buildCursorsPage();

	void onStarted();
	void onCycleComplete();
	void onStopped();

	void setSampleRate(double hz);
	void setBufferSize(int n);
	void setPlotWindow(int n);
	void setDelay(int samples);
	void applySweep();
	void applyXTransform();
	void restartIfRunning(const std::function<void()> &change);

	void addDecode(const scopy::acq::DataKey &key);
	void removeDecodes(const QList<scopy::acq::DataKey> &keys);
	void addDecoderRow(const QString &uid);
	void removeDecoderRow(const QString &uid);
	void syncDecoderRates();

	int contentBottomPx() const;
	void applyTrackScroll();

	bool isStreaming() const { return m_plotWindow > m_bufferSize; }
	int delayMin() const;
	double timeOffset() const;
	scopy::component::Device *digital() const;
	bool hasExternalTrigger() const;
	void setTriggerStatus(const QString &status);
	void updateSweepInfo();
	void exportFile();

	M2kSource *m_source{nullptr};
	scopy::adc::AcqPlot *m_plot{nullptr};
	scopy::adc::AcqDigitalChannel *m_dio[CHANNELS]{};
	QList<scopy::adc::AcqAnnotationChannel *> m_decodes;
	QMap<QString, MenuControlButton *> m_decoderRows;
	QList<int> m_decodeRowCounts;
	PlotAxisHandle *m_trigHandle{nullptr};
	QTimer *m_delayTimer{nullptr};
	QScrollBar *m_scroll{nullptr};
	QTimer *m_frameTimer{nullptr};
	bool m_dirty{false};
	int m_uid{0};

	double m_sampleRate{1e6};
	int m_bufferSize{1000};
	int m_plotWindow{1000};
	int m_delay{0};
	// The delay of the data on screen, which is what places t = 0.
	int m_shownDelay{0};
	double m_trigFrac{0.0};

	MenuControlButton *m_chRow[CHANNELS]{};
	QPointer<MenuCombo> m_trigger[CHANNELS];
	QPointer<gui::MenuSpinbox> m_rateSpin;
	QPointer<gui::MenuSpinbox> m_bufferSpin;
	QPointer<gui::MenuSpinbox> m_windowSpin;
	QPointer<gui::MenuSpinbox> m_delaySpin;
	QPointer<MenuOnOffSwitch> m_buffersSwitch;
	QPointer<QAbstractButton> m_trigEnable;
	QPointer<MenuCombo> m_modeCombo;
	QPointer<MenuOnOffSwitch> m_extSwitch;
	QPointer<MenuCombo> m_extSourceCombo;
	QPointer<MenuCombo> m_extConditionCombo;
	QPointer<QLabel> m_trigStatus;
	QPointer<QLabel> m_plotStatus;
	QPointer<QLabel> m_sweepInfo;
	QPointer<QLabel> m_continuityWarning;
	QTimer *m_trigWaitTimer{nullptr};
};
} // namespace scopy::adalm2000
