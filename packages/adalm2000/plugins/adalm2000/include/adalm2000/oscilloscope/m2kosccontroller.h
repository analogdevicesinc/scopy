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
 */

#pragma once

#include "m2kinstrumentcontroller.h"
#include "m2ksource.h"
#include "oscmath.h"
#include "sweepmath.h"

#include <core/acq_engine/measure.h>
#include <core/acq_engine/measureprocessor.h>
#include <core/acq_engine/scaleoffsetprocessor.h>

#include <QList>
#include <QMap>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVariantMap>

class QAbstractButton;
class QLabel;
class QScrollArea;
class QTimer;

namespace scopy {
class DockWrapperInterface;
class MeasurementLabel;
class MeasurementSelector;
class MenuCombo;
class MenuControlButton;
class MenuLineEdit;
class MenuOnOffSwitch;
class PlotAxisHandle;
class CustomSwitch;
class PlotCursors;
class StatsLabel;
namespace gui {
class MenuPlotChannelCurveStyleControl;
class MenuSpinbox;
} // namespace gui
namespace acq {
class GenalyzerFFTProcessor;
class MathProcessor;
class SpectrumProcessor;
class MeasureMarksItem;
class SnapshotSource;
} // namespace acq
namespace adc {
class AcqCurveChannel;
class AcqPlot;
class FileSourceBlock;
} // namespace adc
} // namespace scopy

namespace scopy::adalm2000 {
class HorizontalSettings;
class M2kOscHistogramProcessor;

// The ADALM2000 Oscilloscope: the two analog inputs on a time axis, with the hardware
// trigger placing t = 0. Traces are CH1, CH2, then math channels and references.
class M2kOscController : public M2kInstrumentController
{
	Q_OBJECT
public:
	static constexpr int CHANNELS = 2;

	M2kOscController(ToolMenuEntry *tme, M2kContext *ctx, const QString &uri, QObject *parent = nullptr);
	~M2kOscController() override;

	void single() override;

	int traceCount() const { return m_traces.size(); }
	QString traceName(int i) const;
	void setChannelEnabled(int i, bool on);
	bool isChannelEnabled(int i) const;
	void setVoltsPerDiv(int i, double v);
	double voltsPerDiv(int i) const;
	void setPosition(int i, double v);
	double position(int i) const;
	void setProbe(int ch, double attenuation);
	double probe(int ch) const;
	void setAcCoupled(int ch, bool on);
	bool isAcCoupled(int ch) const;

	// X1 is CH1 and X2 is CH2, S the sample index. Returns the new trace's index.
	int addMath(const QString &formula);
	QStringList mathFormulas() const;
	// Captures the trace into the snapshot source. Returns the new trace's index, -1 without data.
	int snapshot(int i);
	// Loads the file into the file source. Returns how many references it added.
	int importReferences(const QString &path);
	// Math channels and references only.
	void removeTrace(int i);

	void setSweep(sweep::Sweep s);
	sweep::Sweep sweep() const { return m_sweep; }
	void setTrigger(const osc::Trigger &t);
	osc::Trigger trigger() const { return m_trigger; }
	bool hasExternalTrigger() const { return m_hasExternal; }
	bool hasLogicTrigger() const { return m_hasLogic; }
	bool hasExternalOut() const { return m_hasExternalOut; }

	void setMeasured(int i, scopy::acq::Measure m, bool on);
	bool isMeasured(int i, scopy::acq::Measure m) const;
	void setStatsShown(int i, scopy::acq::Measure m, bool on);
	bool isStatsShown(int i, scopy::acq::Measure m) const;
	void setGated(bool on);
	bool isGated() const { return m_gated; }
	// The latest value, NaN when not measured.
	double measurement(int i, scopy::acq::Measure m) const;
	QVariantMap statistics(int i, scopy::acq::Measure m) const;

	enum View
	{
		ViewXY,
		ViewHistogram,
		ViewFFT,
		VIEW_COUNT,
	};
	void setViewEnabled(int view, bool on);
	bool isViewEnabled(int view) const { return m_views[view].on; }
	// Trace indexes of channels or math; Y against X.
	void setXYSources(int x, int y);
	int xySource(int axis) const;

	// Probes the inputs at a few rates, then sets the time base, the vertical scales and the
	// trigger for what it found. Asynchronous; autosetFinished() ends it.
	void autoset();
	bool isAutosetting() const { return m_autosetStep >= 0; }

	bool exportData(const QString &path) const;
	PlotCursors *plotCursors() const;
	scopy::acq::DataKey fftKey(int ch, const QString &stage) const;

Q_SIGNALS:
	void autosetFinished();

protected:
	void setupBlocks(scopy::component::Context *ctx) override;
	void setupProcessors() override;
	void setupPlots() override;
	void setupTrigger() override {}
	void setupDecoders() override {}
	void setupAnalysisPanel() override {}

private:
	friend class Adalm2000Osc_API;

	enum class Kind
	{
		Channel,
		Math,
		Reference,
	};

	struct Trace
	{
		Kind kind{Kind::Channel};
		int uid{0};
		QString name;
		QString menuId;
		scopy::acq::DataKey key;
		scopy::adc::AcqCurveChannel *curve{nullptr};
		MenuControlButton *row{nullptr};
		bool enabled{true};
		double voltsPerDiv{1.0};
		double position{0.0};
		QPointer<gui::MenuSpinbox> voltsPerDivSpin;
		QPointer<gui::MenuSpinbox> positionSpin;
		QPointer<MeasurementSelector> selector;
		scopy::acq::MeasureResult lastResult;
		std::array<scopy::acq::MeasureStat, scopy::acq::MEASURE_COUNT> lastStats{};
		scopy::acq::MathProcessor *math{nullptr};
	};

	struct TriggerUi
	{
		QPointer<QAbstractButton> enable;
		QPointer<CustomSwitch> mode;
		QPointer<QAbstractButton> analog;
		QPointer<CustomSwitch> channel;
		QPointer<MenuCombo> condition;
		QPointer<gui::MenuSpinbox> level;
		QPointer<gui::MenuSpinbox> hysteresis;
		QPointer<MenuCombo> digital;
		QPointer<MenuCombo> externalCondition;
		QPointer<MenuCombo> logic;
		QPointer<MenuCombo> out;
	};

	void setupCenter();
	void setupTopRail();
	void setupMeasurements();
	QWidget *buildGeneralPage();
	QWidget *buildTriggerPage();
	QWidget *buildCursorsPage();
	QWidget *buildTracePage(Trace *t);
	QWidget *buildMeasureSection(Trace *t);

	Trace *addTrace(Kind kind, const QString &name, const scopy::acq::DataKey &key,
			scopy::acq::MathProcessor *math = nullptr);
	Trace *trace(int i) const { return i >= 0 && i < m_traces.size() ? m_traces[i] : nullptr; }
	QString groupFor(Kind kind) const;
	void applyEnabled(Trace *t, bool on);

	void onStarted();
	void onCycleComplete();
	void onStopped();

	void setDelay(int samples);
	void applyVertical(Trace *t);
	void applyViews();
	void setupViews();
	void rebuildXY();
	void refreshXYSources();
	scopy::adc::AcqCurveChannel *newCurve(scopy::adc::AcqPlot *plot, const scopy::acq::DataKey &key,
					      const scopy::acq::DataKey &xKey, const scopy::acq::DataKey &infoKey,
					      int colorIndex, int depth, int uid);
	scopy::acq::DataKey probedKey(int ch) const;
	static std::pair<double, double> yRange(const Trace *t);
	void applyHardware();
	void applyXRange();
	void rollXRange();
	void applyTriggerUi();
	void applyTriggerLimits();
	void pushTrigger();
	void trackTriggerDc();
	void placeLevelHandle();
	void setTriggerStatus(const QString &status);
	void exportFile();
	void syncReferences();

	void runAutosetStep();
	void onAutosetCapture();
	void finishAutoset();

	void onMeasured(const scopy::acq::MeasureSnapshot &snap);
	void syncMeasureKeys();
	void applyGate();
	void drawMarks();
	void fitMeasureArea();
	bool eventFilter(QObject *watched, QEvent *event) override;

	scopy::component::Device *analogIn() const { return m_source->device(M2kSource::Analog); }
	scopy::component::Channel *input(int ch) const { return m_source->channel(M2kSource::Analog, ch); }
	bool isStreaming() const { return m_sweep.plotWindow > m_sweep.bufferSize; }
	// Only a changed value reaches the source, which restarts the acquisition for it.
	void setHw(QObject *node, const char *attr, const QString &value, M2kSource::Apply apply);
	void applyRate(double hz);
	void applyStreaming();

	M2kSource *m_source{nullptr};
	QList<double> m_hwRates;
	bool m_lowRange[CHANNELS]{};
	bool m_hasExternal{false};
	bool m_hasLogic{false};
	bool m_hasExternalOut{false};
	scopy::acq::ScaleOffsetProcessor *m_probe{nullptr};
	scopy::acq::ScaleOffsetProcessor::ChannelConfig *m_probeCfg[CHANNELS]{};
	double m_probeValue[CHANNELS]{1.0, 1.0};
	scopy::acq::SnapshotSource *m_snap{nullptr};
	scopy::adc::FileSourceBlock *m_file{nullptr};
	scopy::adc::AcqPlot *m_plot{nullptr};
	QList<Trace *> m_traces;

	struct ViewState
	{
		scopy::adc::AcqPlot *plot{nullptr};
		DockWrapperInterface *dock{nullptr};
		QList<scopy::adc::AcqCurveChannel *> curves;
		bool on{false};
		QPointer<QAbstractButton> sw;
	};
	ViewState m_views[VIEW_COUNT];
	M2kOscHistogramProcessor *m_hist{nullptr};
	scopy::acq::GenalyzerFFTProcessor *m_fft[CHANNELS]{};
	scopy::acq::SpectrumProcessor *m_spectrum[CHANNELS]{};
	int m_xyUid[2]{0, 1};
	QPointer<MenuCombo> m_xyCombo[2];
	QPointer<gui::MenuPlotChannelCurveStyleControl> m_xyStyle;
	QPointer<gui::MenuSpinbox> m_probeUi[CHANNELS];
	QPointer<CustomSwitch> m_couplingUi[CHANNELS];
	int m_uid{0};
	int m_mathCount{0};
	int m_refCount{0};

	sweep::Limits m_limits;
	sweep::Sweep m_sweep;
	int m_shownDelay{0};
	double m_trigFrac{0.5};
	QPointer<HorizontalSettings> m_horizontal;

	osc::Trigger m_trigger;
	// The input's DC level, which an AC-coupled trigger level is relative to.
	double m_trigDc{0.0};
	TriggerUi m_trigUi;
	PlotAxisHandle *m_timeHandle{nullptr};
	PlotAxisHandle *m_levelHandle{nullptr};
	QTimer *m_delayTimer{nullptr};
	QTimer *m_hwTimer{nullptr};
	QTimer *m_trigWaitTimer{nullptr};
	QTimer *m_frameTimer{nullptr};
	bool m_dirty{false};

	scopy::acq::MeasureProcessor *m_measure{nullptr};
	scopy::acq::MeasureMarksItem *m_marks{nullptr};
	QPointer<QScrollArea> m_measureArea;
	QWidget *m_measureFlow{nullptr};
	QWidget *m_statsFlow{nullptr};
	QMap<int, MeasurementLabel *> m_labels;
	QMap<int, StatsLabel *> m_statLabels;
	Trace *m_marksTrace{nullptr};
	bool m_gated{false};
	QPointer<MenuOnOffSwitch> m_gateSwitch;
	QPointer<QAbstractButton> m_cursorXEn;
	QPointer<QAbstractButton> m_cursorYEn;

	int m_autosetStep{-1};
	bool m_autosetResume{false};
	QMetaObject::Connection m_autosetConn;

	QPointer<QLabel> m_trigStatus;
	QPointer<QLabel> m_plotStatus;
	QPointer<QLabel> m_sweepInfo;
};
} // namespace scopy::adalm2000
