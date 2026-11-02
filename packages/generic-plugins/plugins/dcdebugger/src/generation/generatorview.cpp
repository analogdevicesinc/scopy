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

#include "generatorview.h"

#include "nodepanel.h"
#include "streamutils.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSplitter>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/device.h>
#include <component/outputstream.h>
#include <component/scanelement.h>
#include <iiowidget.h>
#include <iiowidgetbuilder.h>
#include <menuonoffswitch.h>
#include <menusectionwidget.h>
#include <menuspinbox.h>
#include <mousewheelwidgetguard.h>
#include <plot_utils.hpp>
#include <plotaxis.h>
#include <plotchannel.h>
#include <plotnavigator.hpp>
#include <plotwidget.h>
#include <pluginbase/statusbarmanager.h>
#include <style.h>
#include <stylehelper.h>
#include <toolbuttons.h>
#include <tooltemplate.h>

using namespace scopy;
using namespace scopy::dcdebugger;
using namespace scopy::component;

namespace {
const char *const SETTINGS_KEY = "settings";

enum Column
{
	ColOn = 0,
	ColChannel,
	ColWaveform,
	ColFrequency,
	ColAmplitude,
	ColOffset,
	ColPhase,
	ColumnCount
};

QDoubleSpinBox *makeSpin(double min, double max, double step, int decimals, double value, QWidget *parent)
{
	auto *sb = new QDoubleSpinBox(parent);
	sb->setRange(min, max);
	sb->setSingleStep(step);
	sb->setDecimals(decimals);
	sb->setValue(value);
	sb->setKeyboardTracking(false); // apply on enter / focus out / arrows, not per keystroke
	sb->setFrame(false);
	return sb;
}

// Centers a checkbox in its table cell.
QWidget *centered(QWidget *w, QWidget *parent)
{
	auto *holder = new QWidget(parent);
	auto *lay = new QHBoxLayout(holder);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->addWidget(w, 0, Qt::AlignCenter);
	return holder;
}
} // namespace

GeneratorView::GeneratorView(OutputStream *stream, QWidget *parent)
	: QWidget(parent)
	, m_stream(stream)
	, m_gen(new StreamGeneration(stream, this))
	, m_applyTimer(new QTimer(this))
{
	m_sampleRateAttr = findSampleRate(stream, true);
	m_applyTimer->setSingleShot(true);
	m_applyTimer->setInterval(150);
	connect(m_applyTimer, &QTimer::timeout, this, &GeneratorView::apply);

	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	// ToolTemplate starts with every container hidden.
	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->centralContainer()->setVisible(true);
	m_tool->rightContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	m_tool->setRightContainerWidth(300);
	lay->addWidget(m_tool);

	m_status = new QLabel("Stopped", this);
	Style::setStyle(m_status, style::properties::label::subtle);
	m_runBtn = new RunBtn(this);
	m_runBtn->setText("Start");
	disconnect(m_runBtn, &QPushButton::toggled, m_runBtn, nullptr); // RunBtn's own Run / Stop text
	connect(m_runBtn, &QPushButton::toggled, m_runBtn,
		[this](bool on) { m_runBtn->setText(on ? "Stop" : "Start"); });
	auto *gearBtn = new GearBtn(this);
	gearBtn->setChecked(true);
	m_tool->addWidgetToTopContainerHelper(m_status, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(gearBtn, TTA_RIGHT);
	m_tool->rightStack()->add(SETTINGS_KEY, buildSettings(this));
	m_tool->openRightContainerHelper(true);
	connect(gearBtn, &QPushButton::toggled, m_tool, &ToolTemplate::openRightContainerHelper);

	auto *splitter = new QSplitter(Qt::Vertical, this);
	splitter->setChildrenCollapsible(false);
	splitter->addWidget(buildTable(splitter));
	buildPreview();
	splitter->addWidget(m_plot);
	splitter->setStretchFactor(0, 1);
	splitter->setStretchFactor(1, 3);
	m_tool->addWidgetToCentralContainerHelper(splitter);

	connect(m_runBtn, &QPushButton::toggled, this, [this](bool on) {
		if(on) {
			apply();
			m_gen->start();
		} else {
			m_gen->stop();
		}
	});
	connect(m_gen, &StreamGeneration::runningChanged, this, &GeneratorView::onRunningChanged);
	connect(m_gen, &StreamGeneration::pushed, this, [this](int channels, quint64 samples, bool cyclic) {
		setStatus(QStringLiteral("Running: %1 channel(s), %2 samples, %3")
				  .arg(channels)
				  .arg(samples)
				  .arg(cyclic ? QStringLiteral("cyclic")
					      : QStringLiteral("streaming (push %1)").arg(m_gen->pushes())));
	});
	connect(m_gen, &StreamGeneration::errorOccurred, this, [this](const QString &msg) {
		setStatus(msg, true);
		StatusBarManager::pushMessage("DC Debugger: " + msg, 5000);
	});

	apply();
}

GeneratorView::~GeneratorView() = default;

QWidget *GeneratorView::buildSettings(QWidget *parent)
{
	auto *scroll = new QScrollArea(parent);
	scroll->setWidgetResizable(true);
	scroll->setFrameShape(QFrame::NoFrame);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	auto *content = new QWidget(scroll);
	auto *outer = new QVBoxLayout(content);
	outer->setContentsMargins(0, 0, 0, 0);

	MenuSectionCollapseWidget *section = makeSection("Output", content);
	QVBoxLayout *lay = section->contentLayout();
	lay->setSpacing(8);
	if(auto *dev = qobject_cast<Device *>(m_stream->parent())) {
		auto *devLabel = new QLabel(
			"Device: " + (dev->label().isEmpty() ? dev->name() : dev->name() + " (" + dev->label() + ")"),
			section);
		devLabel->setWordWrap(true);
		Style::setStyle(devLabel, style::properties::label::subtle);
		lay->addWidget(devLabel);
	}

	m_samples = new gui::MenuSpinbox("Buffer samples", double(m_gen->samples()), "", 2, 4 * 1024 * 1024, true,
					 false, false, section);
	m_samples->setIncrementMode(gui::MenuSpinbox::IS_POW2);
	m_samples->setScalingEnabled(false);
	connect(m_samples, &gui::MenuSpinbox::valueChanged, this, &GeneratorView::scheduleApply);
	lay->addWidget(m_samples);

	if(m_sampleRateAttr) {
		IIOWidget *rate =
			IIOWidgetBuilder(section).attribute(m_sampleRateAttr).title("Sample rate").buildSingle();
		if(rate) {
			lay->addWidget(rate);
			rate->readAsync();
		}
		connect(m_sampleRateAttr, &Attribute::valueChanged, this, &GeneratorView::scheduleApply);
	} else {
		// No sample-rate attribute: the value only places the tones in the buffer.
		m_manualRate = new gui::MenuSpinbox("Sample rate", 1e6, "Hz", 1, 1e12, true, false, false, section);
		connect(m_manualRate, &gui::MenuSpinbox::valueChanged, this, &GeneratorView::scheduleApply);
		lay->addWidget(m_manualRate);
	}

	m_cyclic = new MenuOnOffSwitch("Cyclic", section);
	m_cyclic->onOffswitch()->setChecked(m_gen->cyclic());
	m_cyclic->setEnabled(m_gen->supportsCyclic());
	m_cyclic->setToolTip(m_gen->supportsCyclic()
				     ? "On: one buffer is pushed and replayed by the device. Off: the buffer is "
				       "pushed continuously."
				     : "This stream cannot replay a buffer: it is pushed continuously.");
	connect(m_cyclic->onOffswitch(), &QAbstractButton::toggled, this, &GeneratorView::scheduleApply);
	lay->addWidget(m_cyclic);

	outer->addWidget(section);
	outer->addStretch();
	scroll->setWidget(content);
	return scroll;
}

QWidget *GeneratorView::buildTable(QWidget *parent)
{
	auto *holder = new QWidget(parent);
	auto *lay = new QVBoxLayout(holder);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(6);

	m_table = new QTableWidget(holder);
	m_table->setColumnCount(ColumnCount);
	m_table->setHorizontalHeaderLabels(
		{"On", "Channel", "Waveform", "Frequency (Hz)", "Amplitude (FS)", "Offset (FS)", "Phase (°)"});
	m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	m_table->horizontalHeader()->setSectionResizeMode(ColOn, QHeaderView::ResizeToContents);
	m_table->horizontalHeader()->setSectionResizeMode(ColChannel, QHeaderView::ResizeToContents);
	m_table->verticalHeader()->setVisible(false);
	m_table->setSelectionMode(QAbstractItemView::NoSelection);
	m_table->setFocusPolicy(Qt::NoFocus);
	Style::setStyle(m_table, style::properties::widget::tableViewWidget);

	const QList<ScanElement *> elems = m_gen->scanElements();
	const QHash<QString, ToneSpec> tones = m_gen->tones();
	m_table->setRowCount(elems.size());
	for(int r = 0; r < elems.size(); ++r) {
		const QString id = elems[r]->id();
		const ToneSpec t = tones.value(id);
		Row row{id,
			new QCheckBox(m_table),
			new QComboBox(m_table),
			makeSpin(0, 1e12, 100, 3, t.frequency, m_table),
			makeSpin(0, 1, 0.05, 3, t.amplitude, m_table),
			makeSpin(-1, 1, 0.05, 3, t.offset, m_table),
			makeSpin(-360, 360, 15, 1, t.phase, m_table)};
		row.on->setChecked(t.enabled);
		row.waveform->addItems(waveformNames());
		row.waveform->setCurrentIndex(int(t.waveform));
		row.frequency->setSuffix(" Hz");

		QPixmap swatch(10, 10);
		swatch.fill(QColor(StyleHelper::getChannelColor(r)));
		m_table->setItem(r, ColChannel,
				 new QTableWidgetItem(QIcon(swatch), scanElementName(m_stream, id, true)));
		m_table->item(r, ColChannel)->setFlags(Qt::ItemIsEnabled);
		m_table->setCellWidget(r, ColOn, centered(row.on, m_table));
		m_table->setCellWidget(r, ColWaveform, row.waveform);
		m_table->setCellWidget(r, ColFrequency, row.frequency);
		m_table->setCellWidget(r, ColAmplitude, row.amplitude);
		m_table->setCellWidget(r, ColOffset, row.offset);
		m_table->setCellWidget(r, ColPhase, row.phase);

		connect(row.on, &QCheckBox::toggled, this, &GeneratorView::scheduleApply);
		connect(row.waveform, qOverload<int>(&QComboBox::currentIndexChanged), this,
			&GeneratorView::scheduleApply);
		for(QDoubleSpinBox *sb : {row.frequency, row.amplitude, row.offset, row.phase}) {
			connect(sb, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
				&GeneratorView::scheduleApply);
		}
		m_rows.append(row);
	}
	lay->addWidget(m_table);

	auto *note = new QLabel("Frequencies are snapped so a whole number of periods fits the buffer, otherwise the "
				"replay would glitch at the wrap-around. Default phases 0° / 90° give an I/Q pair "
				"for quadrature DACs.",
				holder);
	note->setWordWrap(true);
	Style::setStyle(note, style::properties::label::subtle);
	lay->addWidget(note);

	// Scrolling the table must not change the values under the cursor.
	auto *guard = new MouseWheelWidgetGuard(this);
	guard->installEventRecursively(m_table);
	return holder;
}

void GeneratorView::buildPreview()
{
	m_plot = new PlotWidget(this);
	m_plot->xAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	m_plot->xAxis()->scaleDraw()->setFloatPrecision(2);
	m_plot->xAxis()->scaleDraw()->setUnitType("s");
	m_plot->yAxis()->scaleDraw()->setUnitType("%FS");
	m_plot->yAxis()->setInterval(-110, 110);
	m_plot->xAxis()->setInterval(0, 1);
	m_plot->setShowXAxisLabels(true);
	m_plot->setShowYAxisLabels(true);
	m_plot->navigator()->setBounded(false);
	const QList<ScanElement *> elems = m_gen->scanElements();
	for(int c = 0; c < elems.size(); ++c) {
		QPen pen(QColor(StyleHelper::getChannelColor(c)), 1);
		auto *ch = new PlotChannel(elems[c]->id(), pen, m_plot->xAxis(), m_plot->yAxis(), m_plot);
		m_plot->addPlotChannel(ch);
		ch->enable();
		m_plotChannels.insert(elems[c]->id(), ch);
	}
}

double GeneratorView::sampleRate() const
{
	if(m_sampleRateAttr) {
		return sampleRateOf(m_sampleRateAttr);
	}
	return m_manualRate ? m_manualRate->value() : 0;
}

void GeneratorView::scheduleApply() { m_applyTimer->start(); }

void GeneratorView::apply()
{
	m_applyTimer->stop();
	QHash<QString, ToneSpec> tones;
	for(const Row &row : std::as_const(m_rows)) {
		ToneSpec t;
		t.enabled = row.on->isChecked();
		t.waveform = Waveform(row.waveform->currentIndex());
		t.frequency = row.frequency->value();
		t.amplitude = row.amplitude->value();
		t.offset = row.offset->value();
		t.phase = row.phase->value();
		tones.insert(row.id, t);
	}
	m_gen->setSampleRate(sampleRate());
	m_gen->setSamples(size_t(std::max(2.0, m_samples->value())));
	m_gen->setCyclic(m_cyclic->onOffswitch()->isChecked());
	m_gen->setTones(tones);

	// Show the frequency that is actually generated.
	const QHash<QString, ToneSpec> snapped = m_gen->snappedTones();
	for(const Row &row : std::as_const(m_rows)) {
		const double f = snapped.value(row.id).frequency;
		if(std::abs(f - row.frequency->value()) > 1e-9) {
			QSignalBlocker blocker(row.frequency);
			row.frequency->setValue(f);
		}
		row.frequency->setToolTip(QStringLiteral("Generated: %1 Hz").arg(f, 0, 'f', 3));
	}
	updatePreview();
}

void GeneratorView::updatePreview()
{
	const double fs = sampleRate();
	const int samples = int(m_gen->samples());
	const QHash<QString, ToneSpec> tones = m_gen->snappedTones();

	// A few periods of the slowest enabled tone (the whole buffer at most).
	double slowest = 0;
	for(const ToneSpec &t : tones) {
		if(t.enabled && t.frequency > 0 && t.waveform != Waveform::Constant) {
			slowest = slowest > 0 ? std::min(slowest, t.frequency) : t.frequency;
		}
	}
	int shown = samples;
	if(fs > 0 && slowest > 0) {
		shown = std::clamp(int(std::ceil(4 * fs / slowest)), 2, samples);
	}

	QVector<float> x(shown);
	for(int i = 0; i < shown; ++i) {
		x[i] = fs > 0 ? float(i / fs) : float(i);
	}
	m_plot->xAxis()->scaleDraw()->setUnitType(fs > 0 ? "s" : "");
	for(auto it = m_plotChannels.cbegin(); it != m_plotChannels.cend(); ++it) {
		const ToneSpec t = tones.value(it.key());
		if(!t.enabled) {
			it.value()->curve()->setSamples(QVector<QPointF>());
			continue;
		}
		const QVector<double> v = generateTone(t, shown, fs);
		QVector<float> y(shown);
		for(int i = 0; i < shown; ++i) {
			y[i] = float(v[i] * 100);
		}
		it.value()->setSamples(x.data(), y.data(), size_t(shown), true);
	}
	if(!m_plot->navigator()->isZoomed()) {
		m_plot->xAxis()->setInterval(0, shown > 1 ? x.last() : 1);
	}
	m_plot->replot();
}

void GeneratorView::onRunningChanged(bool running)
{
	if(!running) {
		QSignalBlocker blocker(m_runBtn);
		m_runBtn->setChecked(false);
		m_runBtn->setText("Start");
		if(!m_status->text().contains("failed") && !m_status->text().startsWith("No channel")) {
			setStatus("Stopped");
		}
	} else {
		setStatus("Starting");
	}
}

void GeneratorView::setStatus(const QString &text, bool error)
{
	m_status->setText(text);
	Style::setStyle(m_status, style::properties::label::warning, error);
	Style::setStyle(m_status, style::properties::label::subtle, !error);
}

#include "moc_generatorview.cpp"
