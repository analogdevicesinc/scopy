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

#include "rawgeneratorview.h"

#include "nodepanel.h"
#include "readpolicy.h"
#include "sampletable.h"
#include "sidesplitter.h"
#include "streamutils.h"

#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontMetrics>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QShortcut>
#include <QSignalBlocker>
#include <QSplitter>
#include <QTableView>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <limits>

#include <component/attribute.h>
#include <component/channel.h>
#include <component/device.h>
#include <component/outputstream.h>
#include <dcdebugger/style_properties.h>
#include <hoverwidget.h>
#include <iiowidget.h>
#include <iiowidgetbuilder.h>
#include <menulineedit.h>
#include <menuonoffswitch.h>
#include <menusectionwidget.h>
#include <menuspinbox.h>
#include <mousewheelwidgetguard.h>
#include <plotaxis.h>
#include <plotchannel.h>
#include <plotlegend.h>
#include <plotnavigator.hpp>
#include <plotwidget.h>
#include <pluginbase/preferences.h>
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
constexpr int MaxRows = 4 * 1024 * 1024;
} // namespace

RawGeneratorView::RawGeneratorView(OutputStream *stream, QWidget *parent)
	: QWidget(parent)
	, m_stream(stream)
	, m_gen(new StreamGeneration(stream, this))
	, m_applyTimer(new QTimer(this))
{
	m_sampleRateAttrs = memberSampleRates(stream, true);
	m_gen->setRawMode(true);
	// Every member is pushed; a channel without table data would send 0 (a silent tone).
	QHash<QString, ToneSpec> tones = m_gen->tones();
	for(ToneSpec &t : tones) {
		t.enabled = true;
		t.waveform = Waveform::Constant;
		t.amplitude = 0;
		t.offset = 0;
	}
	m_gen->setTones(tones);
	for(Channel *ch : m_gen->channels()) {
		m_ids.append(ch->id());
	}

	m_applyTimer->setSingleShot(true);
	m_applyTimer->setInterval(150);
	connect(m_applyTimer, &QTimer::timeout, this, &RawGeneratorView::apply);

	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	// ToolTemplate starts with every container hidden.
	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->centralContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	lay->addWidget(m_tool);
	// Preview over table | Output menu.
	m_sides = new SideSplitter(0, 300, QStringLiteral("rawgenerator/sides"), this);
	m_tool->addWidgetToCentralContainerHelper(m_sides);

	m_status = new QLabel("Stopped", this);
	Style::setStyle(m_status, style::properties::label::subtle);
	m_runBtn = new RunBtn(this);
	m_runBtn->setObjectName("rawGeneratorRun");
	m_runBtn->setText("Start");
	disconnect(m_runBtn, &QPushButton::toggled, m_runBtn, nullptr); // RunBtn's own Run / Stop text
	connect(m_runBtn, &QPushButton::toggled, m_runBtn,
		[this](bool on) { m_runBtn->setText(on ? "Stop" : "Start"); });
	auto *gearBtn = new GearBtn(this);
	gearBtn->setChecked(m_sides->isRightOpen());
	m_tool->addWidgetToTopContainerHelper(m_status, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(gearBtn, TTA_RIGHT);
	connect(gearBtn, &QPushButton::toggled, m_sides, &SideSplitter::setRightOpen);
	connect(m_sides, &SideSplitter::rightOpenChanged, gearBtn, &QAbstractButton::setChecked);

	m_model = new SampleTableModel(m_ids, DefaultRows, this);
	connect(m_model, &SampleTableModel::samplesChanged, this, &RawGeneratorView::scheduleApply);

	auto *central = new QSplitter(Qt::Vertical, this);
	central->setObjectName("rawGeneratorSplitter");
	central->setChildrenCollapsible(true);
	buildPreview();
	buildTable();
	central->addWidget(m_plot);
	central->addWidget(m_table);
	central->setStretchFactor(0, 1);
	central->setStretchFactor(1, 1);
	m_sides->setCentralWidget(central);
	m_sides->rightStack()->add(SETTINGS_KEY, buildMenu(this));
	m_sides->rightStack()->show(SETTINGS_KEY);

	connect(m_runBtn, &QPushButton::toggled, this, [this](bool on) {
		if(on) {
			apply();
			m_gen->start();
		} else {
			m_gen->stop();
		}
	});
	connect(m_gen, &StreamGeneration::runningChanged, this, &RawGeneratorView::onRunningChanged);
	connect(m_gen, &StreamGeneration::pushed, this, [this](int channels, quint64 samples, bool cyclic) {
		const quint64 clipped = m_gen->clippedSamples();
		setStatus(
			QStringLiteral("Running: %1 channel(s), %2 samples, %3%4")
				.arg(channels)
				.arg(samples)
				.arg(cyclic ? QStringLiteral("cyclic")
					    : QStringLiteral("streaming (push %1)").arg(m_gen->pushes()))
				.arg(clipped ? QStringLiteral(" · %1 sample(s) clipped to the code range").arg(clipped)
					     : QString()),
			clipped > 0);
	});
	connect(m_gen, &StreamGeneration::errorOccurred, this, [this](const QString &msg) {
		setStatus(msg, true);
		StatusBarManager::pushMessage("DC Debugger: " + msg, 5000);
	});
	if(m_ids.isEmpty()) {
		setStatus("This stream exposes no streamable channels.", true);
		m_runBtn->setEnabled(false);
		m_loadBtn->setEnabled(false);
	}

	apply();
}

RawGeneratorView::~RawGeneratorView() = default;

QWidget *RawGeneratorView::buildMenu(QWidget *parent)
{
	auto *scroll = new QScrollArea(parent);
	scroll->setWidgetResizable(true);
	scroll->setFrameShape(QFrame::NoFrame);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll->setObjectName("rawGeneratorSettingsMenu");
	auto *content = new QWidget(scroll);
	auto *outer = new QVBoxLayout(content);
	outer->setContentsMargins(0, 0, 0, 0);
	outer->setSpacing(10);
	outer->addWidget(buildSamplesSection(content));
	outer->addWidget(buildSettingsSection(content));
	outer->addStretch();
	auto *guard = new MouseWheelWidgetGuard(content);
	guard->installEventRecursively(content);
	scroll->setWidget(content);
	return scroll;
}

QWidget *RawGeneratorView::buildSettingsSection(QWidget *parent)
{
	MenuSectionCollapseWidget *section = makeSection("Output", parent);
	section->setObjectName("rawGeneratorOutputSection");
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

	// One control for the stream: shows the first member's rate, a write is applied to every member.
	if(Attribute *first = m_sampleRateAttrs.value(0)) {
		IIOWidget *rate = IIOWidgetBuilder(section).attribute(first).title("Sample rate").buildSingle();
		if(rate) {
			rate->setObjectName("rawGeneratorSampleRate");
			lay->addWidget(rate);
			if(!ReadPolicy::suppressed()) { // e.g. built by a "go to origin" from the log
				rate->readAsync();
			}
		}
		linkSampleRates(this, m_sampleRateAttrs, [this](const QString &msg) { setStatus(msg, true); });
	}

	m_cyclic = new MenuOnOffSwitch("Cyclic", section);
	m_cyclic->setObjectName("rawGeneratorCyclic");
	m_cyclic->onOffswitch()->setChecked(m_gen->cyclic());
	m_cyclic->setEnabled(m_gen->supportsCyclic());
	m_cyclic->setToolTip(m_gen->supportsCyclic()
				     ? "On: one buffer is pushed and replayed by the device. Off: the buffer is "
				       "pushed continuously."
				     : "This stream cannot replay a buffer: it is pushed continuously.");
	connect(m_cyclic->onOffswitch(), &QAbstractButton::toggled, this, &RawGeneratorView::scheduleApply);
	lay->addWidget(m_cyclic);
	return section;
}

QWidget *RawGeneratorView::buildSamplesSection(QWidget *parent)
{
	MenuSectionCollapseWidget *section = makeSection("Samples", parent);
	section->setObjectName("rawGeneratorSamplesSection");
	QVBoxLayout *lay = section->contentLayout();
	lay->setSpacing(8);

	// A path typed (Enter) or picked with Load: CSV / text, one column per channel.
	m_path = new MenuLineEdit(section);
	m_path->setObjectName("rawGeneratorPath");
	m_path->edit()->setPlaceholderText("Path to a CSV / text file");
	m_path->setToolTip("One column per channel (raw codes); an optional header row and a leading "
			   "\"sample\" column are skipped. Enter loads it.");
	connect(m_path->edit(), &QLineEdit::returnPressed, this, [this]() {
		const QString path = m_path->edit()->text().trimmed();
		if(!path.isEmpty()) {
			loadFile(path);
		}
	});
	m_loadBtn = new QPushButton("Load", section);
	m_loadBtn->setObjectName("rawGeneratorLoad");
	m_loadBtn->setToolTip("Pick a file and load it");
	Style::setStyle(m_loadBtn, style::properties::button::basicButton);
	connect(m_loadBtn, &QPushButton::clicked, this, &RawGeneratorView::loadFromDialog);
	auto *fileRow = new QHBoxLayout();
	fileRow->setSpacing(6);
	fileRow->addWidget(m_path, 1);
	fileRow->addWidget(m_loadBtn);
	lay->addLayout(fileRow);

	// The buffer length is the number of rows: new rows are 0, dropped rows are lost.
	m_rows = new gui::MenuSpinbox("Samples", m_model->rowCount(), "", 1, MaxRows, true, false, false, section);
	m_rows->setObjectName("rawGeneratorRows");
	m_rows->setIncrementMode(gui::MenuSpinbox::IS_POW2);
	m_rows->setScalingEnabled(false);
	m_rows->setToolTip("Rows of the table: the buffer length");
	connect(m_rows, &gui::MenuSpinbox::valueChanged, this,
		[this](double v) { m_model->setRows(int(std::clamp(v, 1.0, double(MaxRows)))); });
	connect(m_model, &SampleTableModel::samplesChanged, this, [this]() {
		if(int(m_rows->value()) != m_model->rowCount()) {
			m_rows->setValueSilent(m_model->rowCount());
		}
	});
	lay->addWidget(m_rows);
	return section;
}

void RawGeneratorView::buildTable()
{
	m_table = new QTableView(this);
	m_table->setObjectName("rawGeneratorTable");
	m_table->setModel(m_model);
	m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	m_table->verticalHeader()->setDefaultSectionSize(22);
	m_table->setSelectionMode(QAbstractItemView::ContiguousSelection);
	// A header click selects the whole row / column (for Ctrl+C).
	m_table->horizontalHeader()->setSectionsClickable(true);
	m_table->verticalHeader()->setSectionsClickable(true);
	m_table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed |
				 QAbstractItemView::AnyKeyPressed);
	m_table->setToolTip("Raw codes; double click to edit, Ctrl+C copies the selection, Ctrl+V pastes "
			    "spreadsheet cells from the current one");
	Style::setStyle(m_table, style::properties::widget::tableViewWidget);
	// Scroll bars that stay grabbable on long tables.
	Style::setStyle(m_table->verticalScrollBar(), style::properties::dcdebugger::grabScrollBar);
	Style::setStyle(m_table->horizontalScrollBar(), style::properties::dcdebugger::grabScrollBar);
	fitRowHeader();
	connect(m_model, &SampleTableModel::modelReset, this, &RawGeneratorView::fitRowHeader);
	auto *paste = new QShortcut(QKeySequence::Paste, m_table);
	paste->setContext(Qt::WidgetWithChildrenShortcut);
	connect(paste, &QShortcut::activated, this, &RawGeneratorView::pasteClipboard);
	auto *copy = new QShortcut(QKeySequence::Copy, m_table);
	copy->setContext(Qt::WidgetWithChildrenShortcut);
	connect(copy, &QShortcut::activated, this, &RawGeneratorView::copySelection);
}

void RawGeneratorView::buildPreview()
{
	m_plot = new PlotWidget(this);
	m_plot->xAxis()->scaleDraw()->setFloatPrecision(0);
	m_plot->yAxis()->scaleDraw()->setFloatPrecision(0);
	m_plot->yAxis()->setInterval(-1, 1);
	m_plot->setShowXAxisLabels(true);
	m_plot->setShowYAxisLabels(true);
	m_plot->showAxisLabels();
	m_plot->navigator()->setBounded(false);
	for(int c = 0; c < m_ids.size(); ++c) {
		QPen pen(QColor(StyleHelper::getChannelColor(c)), 1);
		auto *ch = new PlotChannel(streamChannelName(m_stream, m_ids[c], true), pen, m_plot->xAxis(),
					   m_plot->yAxis(), m_plot);
		m_plot->addPlotChannel(ch);
		ch->enable();
		m_plotChannels.insert(m_ids[c], ch);
	}
	auto *legend = new PlotLegend(m_plot, m_plot);
	placeLegend();
	legend->setVisible(!m_ids.isEmpty());
}

void RawGeneratorView::fitRowHeader()
{
	QHeaderView *header = m_table->verticalHeader();
	const int last = std::max(0, m_model->rowCount() - 1);
	const int digits = int(QString::number(last).size());
	// The widest digit string of that length (bold, as a style may draw the sections), plus the
	// section padding.
	QFont font = header->font();
	font.setBold(true);
	const int text = QFontMetrics(font).horizontalAdvance(QString(digits, '0'));
	header->setFixedWidth(text + 2 * 8 + 4);
}

void RawGeneratorView::placeLegend()
{
	// PlotLegend keeps its overlay private: it is the HoverWidget it added to the plot. Top right,
	// half transparent so the curves under it stay visible.
	for(HoverWidget *hover : m_plot->findChildren<HoverWidget *>(QString(), Qt::FindDirectChildrenOnly)) {
		if(!hover->findChild<QLabel *>()) {
			continue; // not the legend (its panel holds the channel names)
		}
		hover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
		hover->setContentPos(HoverPosition::HP_BOTTOMLEFT);
		hover->setAnchorOffset(QPoint(-10, 10));
		auto *opacity = new QGraphicsOpacityEffect(hover);
		opacity->setOpacity(0.5);
		hover->setGraphicsEffect(opacity);
	}
}

void RawGeneratorView::scheduleApply() { m_applyTimer->start(); }

void RawGeneratorView::apply()
{
	m_applyTimer->stop();
	QHash<QString, QVector<double>> samples;
	for(int c = 0; c < m_ids.size(); ++c) {
		samples.insert(m_ids[c], m_model->column(c));
	}
	m_gen->setSamples(size_t(std::max(1, m_model->rowCount())));
	m_gen->setCyclic(m_cyclic->onOffswitch()->isChecked());
	m_gen->setRawSamples(samples);
	updatePreview();
}

void RawGeneratorView::updatePreview()
{
	const int rows = m_model->rowCount();
	QVector<float> x(rows);
	for(int i = 0; i < rows; ++i) {
		x[i] = float(i);
	}
	double ylo = std::numeric_limits<double>::max();
	double yhi = std::numeric_limits<double>::lowest();
	for(int c = 0; c < m_ids.size(); ++c) {
		const QVector<double> &v = m_model->column(c);
		QVector<float> y(v.size());
		for(int i = 0; i < v.size(); ++i) {
			y[i] = float(v[i]);
			ylo = std::min(ylo, v[i]);
			yhi = std::max(yhi, v[i]);
		}
		m_plotChannels.value(m_ids[c])->setSamples(x.data(), y.data(), size_t(y.size()), true);
	}
	if(!m_plot->navigator()->isZoomed()) {
		m_plot->xAxis()->setInterval(0, std::max(1, rows));
		if(ylo <= yhi) {
			const double span = yhi > ylo ? yhi - ylo : std::max(1.0, std::abs(yhi));
			m_plot->yAxis()->setInterval(ylo - span * 0.1, yhi + span * 0.1);
		}
	}
	m_plot->replot();
}

bool RawGeneratorView::loadFile(const QString &path)
{
	QFile file(path);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		setStatus("Cannot open " + path + ": " + file.errorString(), true);
		return false;
	}
	ParsedSamples parsed;
	QString error;
	if(!parseSamples(QString::fromUtf8(file.readAll()), parsed, &error)) {
		setStatus(QFileInfo(path).fileName() + ": " + error, true);
		return false;
	}
	if(parsed.rows() > MaxRows) {
		setStatus(QStringLiteral("%1: %2 rows, at most %3")
				  .arg(QFileInfo(path).fileName())
				  .arg(parsed.rows())
				  .arg(MaxRows),
			  true);
		return false;
	}
	m_model->setColumns(parsed.columns);
	apply();
	QString text = QStringLiteral("Loaded %1 sample(s) from %2").arg(parsed.rows()).arg(QFileInfo(path).fileName());
	const int columns = int(parsed.columns.size());
	if(columns > m_ids.size()) {
		text += QStringLiteral(" · %1 extra column(s) ignored").arg(columns - m_ids.size());
	} else if(columns < m_ids.size()) {
		text += QStringLiteral(" · %1 set to 0").arg(QStringList(m_ids.mid(columns)).join(", "));
	}
	setStatus(text);
	return true;
}

void RawGeneratorView::loadFromDialog()
{
	const bool native = Preferences::get("general_use_native_dialogs").toBool();
	const QString path = QFileDialog::getOpenFileName(
		this, "Load samples", QString(), "CSV / text (*.csv *.txt);;All files (*)", nullptr,
		native ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(!path.isEmpty()) {
		m_path->edit()->setText(path);
		loadFile(path);
	}
}

void RawGeneratorView::pasteClipboard()
{
	ParsedSamples parsed;
	QString error;
	if(!parseSamples(QApplication::clipboard()->text(), parsed, &error)) {
		setStatus("Paste: " + error, true);
		return;
	}
	const QModelIndex at = m_table->currentIndex();
	m_model->paste(at.isValid() ? at.row() : 0, at.isValid() ? at.column() : 0, parsed.columns);
	setStatus(QStringLiteral("Pasted %1 row(s) × %2 column(s)").arg(parsed.rows()).arg(parsed.columns.size()));
}

void RawGeneratorView::copySelection()
{
	// Contiguous selection: one range (cells, whole rows or whole columns).
	const QItemSelection sel = m_table->selectionModel()->selection();
	if(sel.isEmpty()) {
		return;
	}
	const QItemSelectionRange &r = sel.first();
	QApplication::clipboard()->setText(m_model->toTsv(r.top(), r.bottom(), r.left(), r.right()));
	setStatus(QStringLiteral("Copied %1 row(s) × %2 column(s)").arg(r.height()).arg(r.width()));
}

void RawGeneratorView::onRunningChanged(bool running)
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

void RawGeneratorView::setStatus(const QString &text, bool error)
{
	m_status->setText(text);
	Style::setStyle(m_status, style::properties::label::warning, error);
	Style::setStyle(m_status, style::properties::label::subtle, !error);
}

#include "moc_rawgeneratorview.cpp"
