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

#ifndef RAWGENERATORVIEW_H
#define RAWGENERATORVIEW_H

#include "scopy-dcdebugger_export.h"
#include "streamgeneration.h"

#include <QList>
#include <QMap>
#include <QPointer>
#include <QWidget>

class QLabel;
class QPushButton;
class QTimer;

namespace scopy {
class MenuCombo;
class MenuOnOffSwitch;
class PlotChannel;
class PlotWidget;
class ToolTemplate;
namespace gui {
class MenuSpinbox;
}
} // namespace scopy

namespace scopy::component {
class Attribute;
class OutputStream;
} // namespace scopy::component

namespace scopy::dcdebugger {
class SideSplitter;

// Simplified Generator tab of an OutputStream, in raw codes: every member channel gets its own
// tone (waveform, periods per buffer, amplitude / offset in codes, phase), and every setting is in
// the right menu (Output, then one section per channel). The preview plots one buffer against the
// sample index, with a legend. One sample-rate control is applied to every member channel; it only
// sets the playback speed (the tone frequency shown is periods * rate / samples).
class SCOPY_DCDEBUGGER_EXPORT RawGeneratorView : public QWidget
{
	Q_OBJECT
public:
	static constexpr int DefaultPeriods = 8;
	static constexpr double DefaultAmplitude = 1024;

	explicit RawGeneratorView(component::OutputStream *stream, QWidget *parent = nullptr);
	~RawGeneratorView() override;

	StreamGeneration *generation() const { return m_gen; }
	PlotWidget *plot() const { return m_plot; }

public Q_SLOTS:
	// Applies the menu values to the generation now (they are otherwise applied shortly after a
	// change).
	void apply();

private:
	struct ChannelControls
	{
		QString id;
		MenuCombo *waveform = nullptr;
		gui::MenuSpinbox *periods = nullptr;
		gui::MenuSpinbox *amplitude = nullptr;
		gui::MenuSpinbox *offset = nullptr;
		gui::MenuSpinbox *phase = nullptr;
		QLabel *frequency = nullptr;
	};

	QWidget *buildSettings(QWidget *parent);
	QWidget *buildOutputSection(QWidget *parent);
	QWidget *buildChannelSection(ChannelControls &ctrl, int index, const ToneSpec &tone, QWidget *parent);
	void buildPreview();
	void placeLegend();
	void scheduleApply();
	void updatePreview();
	void updateFrequencies();
	void onRunningChanged(bool running);
	void setStatus(const QString &text, bool error = false);
	double sampleRate() const;

	QPointer<component::OutputStream> m_stream;
	StreamGeneration *m_gen;
	QList<component::Attribute *> m_sampleRateAttrs;

	ToolTemplate *m_tool;
	SideSplitter *m_sides;
	QPushButton *m_runBtn;
	QLabel *m_status;
	gui::MenuSpinbox *m_samples = nullptr;
	MenuOnOffSwitch *m_cyclic = nullptr;
	QList<ChannelControls> m_channels;
	PlotWidget *m_plot = nullptr;
	QMap<QString, PlotChannel *> m_plotChannels;
	QTimer *m_applyTimer;
};

} // namespace scopy::dcdebugger

#endif // RAWGENERATORVIEW_H
