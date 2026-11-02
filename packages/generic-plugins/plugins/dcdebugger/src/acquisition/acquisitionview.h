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

#include "streamacquisition.h"

#include <QMap>
#include <QPointer>
#include <QWidget>

class QCheckBox;
class QLabel;
class QPushButton;

namespace scopy {
class MenuCombo;
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

// Acquisition tab of an InputStream: run / single, scan-element selection, samples, sample
// rate, hardware trigger assignment, software edge trigger and a time plot.
class AcquisitionView : public QWidget
{
	Q_OBJECT
public:
	explicit AcquisitionView(component::InputStream *stream, QWidget *parent = nullptr);
	~AcquisitionView() override;

	StreamAcquisition *acquisition() const { return m_acq; }

private:
	QWidget *buildSettings(QWidget *parent);
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
};

} // namespace scopy::dcdebugger

#endif // ACQUISITIONVIEW_H
