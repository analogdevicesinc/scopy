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

#ifndef RAWACQUISITIONVIEW_H
#define RAWACQUISITIONVIEW_H

#include "scopy-dcdebugger_export.h"
#include "streamacquisition.h"

#include <QMap>
#include <QPointer>
#include <QWidget>

class QCheckBox;
class QLabel;
class QPushButton;

namespace scopy {
class MenuOnOffSwitch;
class PlotChannel;
class PlotLegend;
class PlotWidget;
class ToolTemplate;
namespace gui {
class MenuPlotAxisRangeControl;
class MenuSpinbox;
} // namespace gui
} // namespace scopy

namespace scopy::component {
class Attribute;
class InputStream;
} // namespace scopy::component

namespace scopy::dcdebugger {
class SideSplitter;

// Simplified Acquisition tab of an InputStream: every member channel is acquired and plotted as
// raw decoded integers against the sample index (0 .. Samples), with a legend. The channel list
// only shows / hides curves, it never changes what is acquired. Run / single, samples, one sample
// rate applied to every member channel; Y axis autoscale or fixed range; CSV export.
// No view modes, triggers or measurements.
class SCOPY_DCDEBUGGER_EXPORT RawAcquisitionView : public QWidget
{
	Q_OBJECT
public:
	explicit RawAcquisitionView(component::InputStream *stream, QWidget *parent = nullptr);
	~RawAcquisitionView() override;

	StreamAcquisition *acquisition() const { return m_acq; }
	PlotWidget *plot() const { return m_plot; }
	// The acquired buffer as CSV (sample column, then one per channel); empty before a buffer.
	QString csvData() const;

private:
	QWidget *buildSettings(QWidget *parent);
	QWidget *buildAcquisitionSection(QWidget *parent);
	QWidget *buildYAxisSection(QWidget *parent);
	void buildChannels();
	void buildPlot();
	void placeLegend();
	void applyXAxis();
	void onFrame(const AcquiredFrame &frame);
	void render();
	void exportData();
	void onRunningChanged(bool running);
	void setStatus(const QString &text, bool error = false);
	QString displayName(const QString &id) const;

	QPointer<component::InputStream> m_stream;
	StreamAcquisition *m_acq;
	QList<component::Attribute *> m_sampleRateAttrs;

	ToolTemplate *m_tool;
	SideSplitter *m_sides;
	QPushButton *m_runBtn;
	QPushButton *m_singleBtn;
	QPushButton *m_exportBtn;
	QLabel *m_status;
	PlotWidget *m_plot = nullptr;
	PlotLegend *m_legend = nullptr;
	QMap<QString, PlotChannel *> m_plotChannels;
	QMap<QString, QCheckBox *> m_curveBoxes;
	gui::MenuSpinbox *m_samples = nullptr;
	MenuOnOffSwitch *m_yAutoscale = nullptr;
	gui::MenuPlotAxisRangeControl *m_yRangeCtrl = nullptr;

	AcquiredFrame m_frame;
	bool m_hasFrame = false;
};

} // namespace scopy::dcdebugger

#endif // RAWACQUISITIONVIEW_H
