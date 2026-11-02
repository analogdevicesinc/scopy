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

#include "spectrumsweep.h"

#include <core/acq_ui/acqinstrumentcontroller.h>

#include <pluginbase/resourcemanager.h>

#include <QElapsedTimer>
#include <QList>
#include <QPointer>
#include <QString>
#include <QVector>

#include <functional>

class QLabel;
class QPushButton;
class QTimer;

namespace scopy {
class DockWrapperInterface;
class MarkerPanel;
class SemiExclusiveButtonGroup;
class MenuOnOffSwitch;
class MenuCombo;
class MenuControlButton;
class MenuSectionCollapseWidget;
namespace gui {
class MenuSpinbox;
}
namespace acq {
class DataStore;
class GenalyzerFFTProcessor;
class SnapshotSource;
class SpectrumProcessor;
} // namespace acq
namespace adc {
class AcqChannel;
class AcqCurveChannel;
class FileSourceBlock;
class AcqMarkerController;
class AcqPlot;
} // namespace adc
} // namespace scopy

namespace scopy::adalm2000 {
class M2kAnalogInSource;
class M2kContext;

class M2kSpectrumController : public scopy::adc::AcqInstrumentController, public scopy::ResourceUser
{
	Q_OBJECT
public:
	static constexpr int CHANNELS = 2;

	M2kSpectrumController(ToolMenuEntry *tme, M2kContext *ctx, const QString &uri, QObject *parent = nullptr);
	~M2kSpectrumController() override;

	void stop() override;

	void setChannelEnabled(int ch, bool on);
	void snapshot(int ch);

	static scopy::acq::DataKey spectrumKey(int ch);
	static scopy::acq::DataKey freqKey(int ch);

	bool eventFilter(QObject *watched, QEvent *event) override;

protected:
	void setupBlocks(scopy::component::Context *ctx) override;
	void setupProcessors() override;
	void setupPlots() override;
	void setupTrigger() override {}
	void setupDecoders() override {}
	void setupAnalysisPanel() override {}

private:
	friend class Adalm2000Spectrum_API;

	struct Reference
	{
		MenuControlButton *row{nullptr};
		QString menuId;
		scopy::adc::AcqCurveChannel *curve{nullptr};
		scopy::acq::DataKey key;
		scopy::acq::DataKey xKey;
		int size{0};
		std::function<void()> drop;
	};

	struct ChannelUi
	{
		QPointer<MenuCombo> averaging;
		QPointer<scopy::gui::MenuSpinbox> history;
		QPointer<MenuCombo> window;
		QPointer<MenuCombo> range;
	};

	void setupCenter();
	void setupChannelRows();
	void setupTopRail();
	QWidget *page(MenuSectionCollapseWidget *section);
	QWidget *buildChannelPage(int ch);
	QWidget *buildGeneralPage();
	QWidget *buildMarkersPage();
	QWidget *buildCursorsPage();
	void addHoverButton(const QString &label, QWidget *content);
	void setupCalibrationGate();

	QList<scopy::adc::AcqCurveChannel *> curves() const;
	QList<scopy::adc::AcqChannel *> allChannels() const;
	void attachChannel(scopy::adc::AcqChannel *ch, scopy::adc::AcqPlot *plot);
	void rebuildWaterfall();
	void setWaterfallEnabled(bool on);
	void onStarted();
	void onCycleComplete();
	void onStopped();

	void setSpan(const spec::Span &s);
	void applyXRange();
	void setLogFrequency(bool on);
	void setFftSize(int n);
	void refreshSweepReadouts();
	void applyYRange();
	void onUnitsChanged(int idx);
	void applyChannelEnabled(int ch, bool on);
	void setMarkerType(int type);
	void setMarkerCount(int n);
	void selectMarker(int ch, int idx);
	void refreshSelectedMarker();
	void updateProgress();
	void restartIfRunning(const std::function<void()> &change);
	void updateRunEnabled();

	void addReference(const QString &name, const scopy::acq::DataKey &key, const scopy::acq::DataKey &xKey,
			  int size, const std::function<void()> &drop);
	void removeReference(const QString &menuId);
	void syncImportedReferences();
	void exportCsv();
	void saveCsv(const QList<QPair<QString, scopy::acq::DataKey>> &columns, const scopy::acq::DataKey &xKey);

	bool claim();
	void release();

	int bins() const { return spec::bins(m_fftSize); }
	QString unitSymbol() const;

	M2kContext *m_ctx{nullptr};
	const QString m_resourceKey;
	bool m_claimed{false};
	bool m_pingPaused{false};
	bool m_calibrating{false};

	M2kAnalogInSource *m_source{nullptr};
	scopy::acq::GenalyzerFFTProcessor *m_fft[CHANNELS]{};
	scopy::acq::SpectrumProcessor *m_spectrum[CHANNELS]{};

	scopy::adc::AcqPlot *m_specPlot{nullptr};
	scopy::adc::AcqPlot *m_wfPlot{nullptr};
	DockWrapperInterface *m_wfDock{nullptr};
	scopy::adc::AcqCurveChannel *m_curve[CHANNELS]{};
	scopy::adc::AcqChannel *m_waterfall{nullptr};
	scopy::adc::AcqMarkerController *m_markers[CHANNELS]{};
	QPointer<MarkerPanel> m_markerPanel;
	SemiExclusiveButtonGroup *m_hoverGroup{nullptr};
	QList<Reference> m_refs;
	scopy::acq::SnapshotSource *m_snap{nullptr};
	scopy::adc::FileSourceBlock *m_file{nullptr};
	int m_refCount{0};
	QTimer *m_frameTimer{nullptr};
	bool m_dirty{false};
	int m_uid{0};

	bool m_chEnabled[CHANNELS]{true, true};
	int m_waterfallCh{0};
	bool m_waterfallOn{false};
	bool m_logX{false};
	bool m_logY{false};
	spec::Span m_span;
	int m_fftSize{0};
	int m_markerType{0};
	bool m_deltaTable{false};
	bool m_xPending{true};
	int m_selCh{0};
	int m_selMarker{0};
	QElapsedTimer m_cycleClock;
	QTimer *m_progressTimer{nullptr};

	MenuControlButton *m_chRow[CHANNELS]{};
	ChannelUi m_chUi[CHANNELS];
	QPointer<MenuOnOffSwitch> m_logSwitch;
	QPointer<MenuOnOffSwitch> m_wfSwitch;
	QPointer<MenuCombo> m_wfChannelCombo;
	QPointer<MenuCombo> m_markerTypeCombo;
	QPointer<scopy::gui::MenuSpinbox> m_markerCountSpin;
	QPointer<QWidget> m_selSection;
	QPointer<MenuCombo> m_selChCombo;
	QPointer<MenuCombo> m_selMarkerCombo;
	QPointer<scopy::gui::MenuSpinbox> m_selFreqSpin;
	QPointer<QLabel> m_progressLabel;
	QPointer<scopy::gui::MenuSpinbox> m_startSpin;
	QPointer<scopy::gui::MenuSpinbox> m_stopSpin;
	QPointer<scopy::gui::MenuSpinbox> m_centerSpin;
	QPointer<scopy::gui::MenuSpinbox> m_spanSpin;
	QPointer<scopy::gui::MenuSpinbox> m_topSpin;
	QPointer<scopy::gui::MenuSpinbox> m_bottomSpin;
	QPointer<MenuCombo> m_rbwCombo;
	QPointer<MenuCombo> m_unitsCombo;
	QPointer<QLabel> m_rateLabel;
};
} // namespace scopy::adalm2000
