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

#include <QMap>
#include <QPointer>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QTableWidget;
class QTimer;

namespace scopy {
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

// Generator tab of an OutputStream: per scan element waveform / frequency / amplitude / offset /
// phase, samples, sample rate, cyclic, start / stop and a preview of the encoded buffer.
class GeneratorView : public QWidget
{
	Q_OBJECT
public:
	explicit GeneratorView(component::OutputStream *stream, QWidget *parent = nullptr);
	~GeneratorView() override;

	StreamGeneration *generation() const { return m_gen; }

private:
	struct Row
	{
		QString id;
		QCheckBox *on;
		QComboBox *waveform;
		QDoubleSpinBox *frequency;
		QDoubleSpinBox *amplitude;
		QDoubleSpinBox *offset;
		QDoubleSpinBox *phase;
	};

	QWidget *buildSettings(QWidget *parent);
	QWidget *buildTable(QWidget *parent);
	void buildPreview();
	void scheduleApply();
	void apply();
	void updatePreview();
	void onRunningChanged(bool running);
	void setStatus(const QString &text, bool error = false);
	double sampleRate() const;

	QPointer<component::OutputStream> m_stream;
	StreamGeneration *m_gen;
	component::Attribute *m_sampleRateAttr = nullptr;

	ToolTemplate *m_tool;
	QPushButton *m_runBtn;
	QLabel *m_status;
	gui::MenuSpinbox *m_samples;
	gui::MenuSpinbox *m_manualRate = nullptr;
	MenuOnOffSwitch *m_cyclic;
	QTableWidget *m_table;
	QList<Row> m_rows;
	PlotWidget *m_plot;
	QMap<QString, PlotChannel *> m_plotChannels;
	QTimer *m_applyTimer;
};

} // namespace scopy::dcdebugger

#endif // GENERATORVIEW_H
