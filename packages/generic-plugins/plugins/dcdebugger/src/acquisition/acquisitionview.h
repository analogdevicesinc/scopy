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

#ifndef ACQUISITIONVIEW_H
#define ACQUISITIONVIEW_H

#include "signalmath.h"
#include "streamacquisition.h"

#include <QMap>
#include <QPointer>
#include <QWidget>

class QCheckBox;
class QLabel;
class QPushButton;

namespace scopy {
class MenuCombo;
class MenuOnOffSwitch;
class MenuSectionCollapseWidget;
class PlotChannel;
class PlotWidget;
class ToolTemplate;
namespace gui {
class MenuSpinbox;
}
} // namespace scopy

namespace scopy::component {
class Attribute;
class InputStream;
class Triggerable;
} // namespace scopy::component

namespace scopy::dcdebugger {
class MeasurementsTable;
struct MeasurementRow;

// Acquisition tab of an InputStream: run / single, scan-element selection, samples, sample
// rate, hardware trigger assignment, software edge trigger; time / spectrum / XY plot with
// scaled or raw units, Y axis control, a measurements table and CSV export.
class AcquisitionView : public QWidget
{
	Q_OBJECT
public:
	explicit AcquisitionView(component::InputStream *stream, QWidget *parent = nullptr);
	~AcquisitionView() override;

	StreamAcquisition *acquisition() const { return m_acq; }

	enum PlotMode
	{
		TimeMode = 0,
		SpectrumMode,
		XYMode
	};

private:
	QWidget *buildSettings(QWidget *parent);
	QWidget *buildViewSection(QWidget *parent);
	void onViewChanged();
	void render();
	// Spectrum of consecutive enabled channels paired as I + jQ; returns the x range.
	QPair<double, double> renderIQSpectrum(bool raw, double fs, QList<MeasurementRow> &rows, double &ylo,
					       double &yhi);
	void applyAxes(double xlo, double xhi, double ylo, double yhi);
	void applyFixedYRange();
	void exportData();
	QWidget *buildAcquisitionSection(QWidget *parent);
	QWidget *buildSoftwareTriggerSection(QWidget *parent);
	QWidget *buildHardwareTrigger(QWidget *parent);
	void buildPlot();
	void applyChannelSelection();
	void refreshTriggerSources();
	void onFrame(const AcquiredFrame &frame);
	void onRunningChanged(bool running);
	void setStatus(const QString &text, bool error = false);
	double sampleRate() const;
	QString displayName(const QString &id) const;

	QPointer<component::InputStream> m_stream;
	StreamAcquisition *m_acq;
	component::Attribute *m_sampleRateAttr = nullptr;
	component::Triggerable *m_triggerable = nullptr;

	ToolTemplate *m_tool;
	QPushButton *m_runBtn;
	QPushButton *m_singleBtn;
	QLabel *m_status;
	PlotWidget *m_plot;
	QMap<QString, PlotChannel *> m_plotChannels;
	QMap<QString, QCheckBox *> m_channelBoxes;
	QMap<QString, int> m_channelIndex;
	gui::MenuSpinbox *m_samples;
	gui::MenuSpinbox *m_manualRate = nullptr;
	MenuCombo *m_hwTrigger = nullptr;
	MenuSectionCollapseWidget *m_swTrigger;
	MenuCombo *m_swSource;
	MenuCombo *m_swEdge;
	MenuCombo *m_swMode;
	gui::MenuSpinbox *m_swLevel;

	// View
	MenuCombo *m_mode;
	MenuCombo *m_units;
	MenuCombo *m_window;
	gui::MenuSpinbox *m_averages;
	MenuOnOffSwitch *m_complexIQ;
	MenuCombo *m_xySource;
	MenuCombo *m_xySourceY;
	MenuOnOffSwitch *m_autoscale;
	gui::MenuSpinbox *m_yMin;
	gui::MenuSpinbox *m_yMax;
	QMap<int, QPair<double, double>> m_yRange; // per PlotMode
	int m_lastMode = TimeMode;

	MeasurementsTable *m_table;
	QPushButton *m_exportBtn;
	SpectrumAverager m_averager;

	// Last shown buffer (re-rendered when the view changes while stopped).
	AcquiredFrame m_frame;
	bool m_hasFrame = false;
	int m_start = 0;
	int m_count = 0;

	// What the plot currently shows, for export: one shared x column.
	QString m_shownXLabel;
	QVector<double> m_shownX;
	QStringList m_shownNames;
	QVector<QVector<double>> m_shownY;
};

} // namespace scopy::dcdebugger

#endif // ACQUISITIONVIEW_H
