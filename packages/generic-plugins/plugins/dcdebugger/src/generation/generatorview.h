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

#ifndef GENERATORVIEW_H
#define GENERATORVIEW_H

#include "streamgeneration.h"

#include <QHash>
#include <QMap>
#include <QPointer>
#include <QWidget>

class QButtonGroup;
class QLabel;
class QPushButton;
class QTimer;

namespace scopy {
class MenuCombo;
class MenuControlButton;
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

// Generator tab of an OutputStream, laid out like the Scopy instruments: a channel list on the
// left (enable + select), the selected channel's menu (waveform / frequency / amplitude / offset /
// phase) or the general Output settings on the right, start / stop and a buffer preview.
class GeneratorView : public QWidget
{
	Q_OBJECT
public:
	explicit GeneratorView(component::OutputStream *stream, QWidget *parent = nullptr);
	~GeneratorView() override;

	StreamGeneration *generation() const { return m_gen; }

private:
	struct ChannelControls
	{
		QString id;
		MenuControlButton *button; // its checkbox enables the channel
		MenuCombo *waveform;
		gui::MenuSpinbox *frequency;
		gui::MenuSpinbox *amplitude;
		gui::MenuSpinbox *offset;
		gui::MenuSpinbox *phase;
	};

	QWidget *buildSettings(QWidget *parent);
	void buildChannels();
	QWidget *buildChannelMenu(ChannelControls &ctrl, int index, const ToneSpec &tone);
	void buildPreview();
	void scheduleApply();
	void apply();
	void updatePreview();
	void onRunningChanged(bool running);
	void setStatus(const QString &text, bool error = false);
	double sampleRate() const;
	// One period in the buffer: the frequency resolution (0 while the rate is unknown).
	double frequencyStep() const;
	// The frequency a change of the channel's spinbox to `requested` generates: snapped to whole
	// periods, but at least one period away from the shown one in the direction of the change.
	double steppedFrequency(const QString &id, double requested) const;

	QPointer<component::OutputStream> m_stream;
	StreamGeneration *m_gen;
	component::Attribute *m_sampleRateAttr = nullptr;

	ToolTemplate *m_tool;
	QPushButton *m_runBtn;
	QLabel *m_status;
	gui::MenuSpinbox *m_samples;
	gui::MenuSpinbox *m_manualRate = nullptr;
	MenuOnOffSwitch *m_cyclic;
	QButtonGroup *m_menuGroup;
	QList<ChannelControls> m_channels;
	PlotWidget *m_plot;
	QMap<QString, PlotChannel *> m_plotChannels;
	QHash<QString, double> m_shownFrequency; // per channel id, what its spinbox shows
	QTimer *m_applyTimer;
};

} // namespace scopy::dcdebugger

#endif // GENERATORVIEW_H
