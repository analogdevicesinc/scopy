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

#include "monitorview.h"

#include "acquisition/measurementstable.h"
#include "nodepanel.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSplitter>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <limits>

#include <component/attribute.h>
#include <component/channel.h>
#include <component/context.h>
#include <component/device.h>
#include <component/navigation.h>
#include <dcdebugger/style_properties.h>
#include <menucombo.h>
#include <menucontrolbutton.h>
#include <menuonoffswitch.h>
#include <menusectionwidget.h>
#include <menuspinbox.h>
#include <menuwidget.h>
#include <mousewheelwidgetguard.h>
#include <plot_utils.hpp>
#include <plotaxis.h>
#include <plotchannel.h>
#include <plotnavigator.hpp>
#include <plotwidget.h>
#include <style.h>
#include <stylehelper.h>
#include <toolbuttons.h>
#include <tooltemplate.h>
#include <verticalchannelmanager.h>

using namespace scopy;
using namespace scopy::dcdebugger;
using namespace scopy::component;

namespace {
const char *const SETTINGS_KEY = "settings";
const char *const CHANNELS_KEY = "channels";

QString channelMenuKey(const QString &key) { return "channel:" + key; }

// The attribute polled by default: a processed value first, then the raw one.
int defaultSource(const QList<Attribute *> &attrs)
{
	for(const QString &name : {"input", "processed", "raw", "value"}) {
		for(int i = 0; i < attrs.size(); ++i) {
			if(attrs[i]->name() == name) {
				return i;
			}
		}
	}
	return 0;
}

// Channels of the same kind share a plot by default: temp0 / temp1 -> temp, accel_x -> accel.
QString kindOf(const Channel *ch)
{
	QString id = ch->id();
	id.remove(QRegularExpression(QStringLiteral("\\d+$")));
	static const QRegularExpression axis(QStringLiteral("_[a-z]$"));
	id.remove(axis);
	return id + (ch->isOutput() ? QStringLiteral(":out") : QString());
}

QTableWidgetItem *cell(const QString &text, Qt::Alignment align = Qt::AlignRight | Qt::AlignVCenter)
{
	auto *item = new QTableWidgetItem(text);
	item->setTextAlignment(align);
	return item;
}
} // namespace

QList<Channel *> MonitorView::monitorableChannels(QObject *root)
{
	QList<Device *> devices;
	if(auto *dev = qobject_cast<Device *>(root)) {
		devices.append(dev);
	} else if(root) {
		devices = root->findChildren<Device *>(QString(), Qt::FindDirectChildrenOnly);
	}
	QList<Channel *> out;
	for(Device *dev : devices) {
		for(Channel *ch : dev->findChildren<Channel *>(QString(), Qt::FindDirectChildrenOnly)) {
			if(!ch->findChildren<Attribute *>(QString(), Qt::FindDirectChildrenOnly).isEmpty()) {
				out.append(ch);
			}
		}
	}
	return out;
}

MonitorView::MonitorView(QObject *root, QWidget *parent)
	: QWidget(parent)
	, m_root(root)
	, m_monitor(new AttributeMonitor(this))
	, m_redraw(new QTimer(this))
{
	m_redraw->setInterval(50); // redraw at most 20 times a second, whatever the poll period
	connect(m_redraw, &QTimer::timeout, this, [this]() {
		if(m_dirty) {
			render();
		}
	});
	connect(m_monitor, &AttributeMonitor::ticked, this, [this]() { m_dirty = true; });

	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	// ToolTemplate starts with every container hidden.
	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->centralContainer()->setVisible(true);
	m_tool->rightContainer()->setVisible(true);
	m_tool->leftContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	m_tool->setRightContainerWidth(300);
	m_tool->setLeftContainerWidth(220);
	lay->addWidget(m_tool);

	m_status = new QLabel("Stopped", this);
	Style::setStyle(m_status, style::properties::label::subtle);
	m_runBtn = new RunBtn(this);
	disconnect(m_runBtn, &QPushButton::toggled, m_runBtn, nullptr); // RunBtn's own Run / Stop text
	m_runBtn->setText("Start");
	m_clearBtn = new QPushButton("Clear", this);
	Style::setStyle(m_clearBtn, style::properties::dcdebugger::topButton);
	auto *gearBtn = new GearBtn(this);
	gearBtn->setChecked(true);
	m_tool->addWidgetToTopContainerHelper(m_status, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(m_clearBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(gearBtn, TTA_RIGHT);
	m_tool->rightStack()->add(SETTINGS_KEY, buildSettings(this));
	m_tool->openRightContainerHelper(true);
	connect(gearBtn, &QPushButton::toggled, m_tool, &ToolTemplate::openRightContainerHelper);

	buildPlots();
	buildChannels();
	m_tool->openLeftContainerHelper(true);

	connect(m_runBtn, &QPushButton::toggled, this, [this](bool on) {
		m_runBtn->setText(on ? "Stop" : "Start");
		if(on) {
			m_monitor->start();
			m_redraw->start();
		} else {
			m_monitor->stop();
			m_redraw->stop();
			render();
		}
		updateStatus();
	});
	connect(m_clearBtn, &QPushButton::clicked, m_monitor, &AttributeMonitor::clear);
	connect(m_clearBtn, &QPushButton::clicked, this, &MonitorView::render);
	connect(m_monitor, &AttributeMonitor::readFailed, this, [this](const QString &key, const QString &msg) {
		if(Entry *e = entry(key)) {
			m_status->setText(e->name + ": " + msg);
		}
	});
	updateStatus();
}

MonitorView::~MonitorView() = default;

QWidget *MonitorView::buildSettings(QWidget *parent)
{
	auto *scroll = new QScrollArea(parent);
	scroll->setWidgetResizable(true);
	scroll->setFrameShape(QFrame::NoFrame);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	auto *content = new QWidget(scroll);
	auto *outer = new QVBoxLayout(content);
	outer->setContentsMargins(0, 0, 0, 0);

	MenuSectionCollapseWidget *section = makeSection("Polling", content);
	QVBoxLayout *lay = section->contentLayout();
	lay->setSpacing(8);
	m_period = new gui::MenuSpinbox("Period", m_monitor->period() / 1000.0, "s", 0.001, 3600, true, false, false,
					section);
	m_period->setIncrementMode(gui::MenuSpinbox::IS_125);
	m_history =
		new gui::MenuSpinbox("History", m_monitor->history(), "s", 1, 24 * 3600, true, false, false, section);
	m_history->setIncrementMode(gui::MenuSpinbox::IS_125);
	m_scaled = new MenuOnOffSwitch("Scaled units", section);
	m_scaled->setToolTip("raw sources with scale / offset siblings are shown as (raw + offset) * scale");
	m_scaled->onOffswitch()->setChecked(m_monitor->scaled());
	lay->addWidget(m_period);
	lay->addWidget(m_history);
	lay->addWidget(m_scaled);
	connect(m_period, &gui::MenuSpinbox::valueChanged, this, [this](double s) {
		m_monitor->setPeriod(int(std::lround(s * 1000)));
		updateStatus();
	});
	connect(m_history, &gui::MenuSpinbox::valueChanged, this, [this](double s) {
		m_monitor->setHistory(s);
		render();
	});
	connect(m_scaled->onOffswitch(), &QAbstractButton::toggled, this, [this](bool on) {
		m_monitor->setScaled(on);
		render();
	});

	outer->addWidget(section);
	outer->addStretch();
	scroll->setWidget(content);
	return scroll;
}

void MonitorView::buildChannels()
{
	auto *vcm = new VerticalChannelManager(this);
	m_menuGroup = new QButtonGroup(this);
	m_menuGroup->setExclusive(true);

	auto *settingsBtn = new MenuControlButton(vcm);
	settingsBtn->setName("Polling");
	settingsBtn->setOpenMenuChecksThis(true);
	settingsBtn->setDoubleClickToOpenMenu(false);
	settingsBtn->checkBox()->setVisible(false);
	settingsBtn->button()->setVisible(false);
	settingsBtn->setCheckable(true);
	settingsBtn->setChecked(true);
	vcm->addTop(settingsBtn);
	m_menuGroup->addButton(settingsBtn);
	connect(settingsBtn, &QAbstractButton::clicked, this, [this]() { m_tool->rightStack()->show(SETTINGS_KEY); });

	QMap<QString, int> kinds; // channel kind -> plot
	QMap<Device *, CollapsableMenuControlButton *> groups;
	const QList<Channel *> channels = monitorableChannels(m_root);
	for(int i = 0; i < channels.size(); ++i) {
		Channel *ch = channels[i];
		auto *dev = qobject_cast<Device *>(ch->parent());
		const QString devName = dev ? (dev->name().isEmpty() ? dev->id() : dev->name()) : QString();

		// One collapsible group per device; its switch toggles all of the device's channels.
		CollapsableMenuControlButton *group = groups.value(dev);
		if(!group) {
			group = new CollapsableMenuControlButton(vcm);
			group->getControlBtn()->setName(devName);
			group->getControlBtn()->button()->setVisible(false);
			group->getControlBtn()->setCheckable(false);
			group->enableOnOffSwitch(true);
			group->onOffSwitch()->setChecked(false);
			vcm->add(group);
			groups.insert(dev, group);
		}

		Entry e;
		e.key = devName + "/" + ch->id() + (ch->isOutput() ? " (out)" : "");
		e.channel = ch;
		e.name = e.key;
		e.color = QColor(StyleHelper::getChannelColor(i));
		e.attributes = ch->findChildren<Attribute *>(QString(), Qt::FindDirectChildrenOnly);
		const QString kind = kindOf(ch);
		if(!kinds.contains(kind)) {
			kinds.insert(kind, kinds.size() % MaxPlots);
		}
		e.plotIndex = kinds.value(kind);

		e.button = new MenuControlButton(group);
		e.button->setName(ch->id() + (ch->isOutput() ? " (out)" : ""));
		e.button->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
		e.button->setColor(e.color);
		e.button->setOpenMenuChecksThis(true);
		e.button->setDoubleClickToOpenMenu(true);
		e.button->button()->setVisible(false);
		e.button->setCheckable(true);
		e.button->checkBox()->setChecked(false);
		e.button->enableToolTip(true);
		group->add(e.button);
		m_menuGroup->addButton(e.button);
		connect(group->onOffSwitch(), &QAbstractButton::toggled, e.button->checkBox(), &QCheckBox::setChecked);

		m_tool->rightStack()->add(channelMenuKey(e.key), buildChannelMenu(e));
		m_entries.append(e);
	}

	for(Entry &e : m_entries) {
		applySource(e);
		const QString key = e.key;
		connect(e.button, &QAbstractButton::clicked, this,
			[this, key]() { m_tool->rightStack()->show(channelMenuKey(key)); });
		connect(e.button->checkBox(), &QCheckBox::toggled, this, [this, key](bool on) {
			Entry *en = entry(key);
			m_monitor->setEnabled(key, on);
			if(en && en->curve) {
				on ? en->curve->enable() : en->curve->disable();
			}
			render();
			updateStatus();
		});
		connect(e.source->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, [this, key]() {
			if(Entry *en = entry(key)) {
				applySource(*en);
				render();
			}
		});
		connect(e.plot->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, [this, key](int idx) {
			if(Entry *en = entry(key)) {
				assignPlot(*en, idx);
				render();
			}
		});
		assignPlot(e, e.plotIndex);
	}
	if(m_entries.isEmpty()) {
		auto *none = new QLabel("No channel with attributes to poll.", vcm);
		none->setWordWrap(true);
		vcm->add(none);
	}
	m_tool->leftStack()->add(CHANNELS_KEY, vcm);
	m_tool->rightStack()->show(SETTINGS_KEY);
}

QWidget *MonitorView::buildChannelMenu(Entry &e)
{
	auto *menu = new gui::MenuWidget(e.name, QPen(e.color), this);

	auto *sourceSection = new MenuSectionCollapseWidget("SOURCE", MenuCollapseSection::MHCW_NONE,
							    MenuCollapseSection::MHW_BASEWIDGET, menu);
	e.source = new MenuCombo("Attribute", sourceSection);
	for(Attribute *a : std::as_const(e.attributes)) {
		e.source->combo()->addItem(a->name());
	}
	e.source->combo()->setCurrentIndex(defaultSource(e.attributes));
	sourceSection->contentLayout()->addWidget(e.source);
	menu->add(sourceSection, "source", gui::MenuWidget::MA_TOPLAST);

	auto *plotSection = new MenuSectionCollapseWidget("PLOT", MenuCollapseSection::MHCW_NONE,
							  MenuCollapseSection::MHW_BASEWIDGET, menu);
	e.plot = new MenuCombo("Plot", plotSection);
	for(int p = 0; p < MaxPlots; ++p) {
		e.plot->combo()->addItem(QStringLiteral("Plot %1").arg(p + 1));
	}
	e.plot->combo()->setCurrentIndex(e.plotIndex);
	e.plot->setToolTip("Channels on the same plot share its axes; pick another plot to separate them.");
	plotSection->contentLayout()->addWidget(e.plot);
	menu->add(plotSection, "plot", gui::MenuWidget::MA_TOPLAST);

	auto *guard = new MouseWheelWidgetGuard(menu);
	guard->installEventRecursively(menu);
	return menu;
}

void MonitorView::buildPlots()
{
	auto *splitter = new QSplitter(Qt::Vertical, this);
	splitter->setChildrenCollapsible(true);

	auto *plotsArea = new QWidget(splitter);
	auto *plotsLay = new QVBoxLayout(plotsArea);
	plotsLay->setContentsMargins(0, 0, 0, 0);
	plotsLay->setSpacing(6);
	for(int p = 0; p < MaxPlots; ++p) {
		auto *plot = new PlotWidget(plotsArea);
		plot->xAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
		plot->xAxis()->scaleDraw()->setFloatPrecision(2);
		plot->xAxis()->scaleDraw()->setUnitType("s");
		plot->yAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
		plot->yAxis()->scaleDraw()->setFloatPrecision(3);
		plot->xAxis()->setInterval(-m_monitor->history(), 0);
		plot->yAxis()->setInterval(-1, 1);
		plot->setShowXAxisLabels(true);
		plot->setShowYAxisLabels(true);
		plot->navigator()->setBounded(false);
		plot->setVisible(false); // shown once a channel on it is enabled
		plotsLay->addWidget(plot);
		m_plots.append(plot);
	}

	m_table = new QTableWidget(splitter);
	const QStringList headers = {"Channel", "Value", "Min", "Max", "Mean"};
	m_table->setColumnCount(headers.size());
	m_table->setHorizontalHeaderLabels(headers);
	m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	m_table->verticalHeader()->setVisible(false);
	m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_table->setSelectionMode(QAbstractItemView::NoSelection);
	m_table->setFocusPolicy(Qt::NoFocus);
	Style::setStyle(m_table, style::properties::widget::tableViewWidget);

	splitter->addWidget(plotsArea);
	splitter->addWidget(m_table);
	splitter->setStretchFactor(0, 4);
	splitter->setStretchFactor(1, 1);
	m_tool->addWidgetToCentralContainerHelper(splitter);
}

void MonitorView::applySource(Entry &e)
{
	const int idx = e.source->combo()->currentIndex();
	Attribute *src = idx >= 0 && idx < e.attributes.size() ? e.attributes[idx] : nullptr;
	Attribute *scale = nullptr;
	Attribute *offset = nullptr;
	// IIO convention: a raw value is scaled by its scale / offset siblings.
	if(src && src->name() == QLatin1String("raw")) {
		scale = attributeByName(e.channel, QStringLiteral("scale"));
		offset = attributeByName(e.channel, QStringLiteral("offset"));
	}
	if(m_monitor->keys().contains(e.key)) {
		m_monitor->setSource(e.key, src, scale, offset);
	} else {
		m_monitor->addItem(e.key, src, scale, offset);
	}
}

void MonitorView::assignPlot(Entry &e, int plotIndex)
{
	plotIndex = std::clamp(plotIndex, 0, MaxPlots - 1);
	if(e.curve) {
		m_plots[e.plotIndex]->removePlotChannel(e.curve);
		delete e.curve;
		e.curve = nullptr;
	}
	e.plotIndex = plotIndex;
	PlotWidget *plot = m_plots[plotIndex];
	e.curve = new PlotChannel(e.name, QPen(e.color, 1), plot->xAxis(), plot->yAxis(), plot);
	plot->addPlotChannel(e.curve);
	m_monitor->isEnabled(e.key) ? e.curve->enable() : e.curve->disable();
}

MonitorView::Entry *MonitorView::entry(const QString &key)
{
	for(Entry &e : m_entries) {
		if(e.key == key) {
			return &e;
		}
	}
	return nullptr;
}

void MonitorView::render()
{
	m_dirty = false;
	const double now = m_monitor->now();
	const double history = m_monitor->history();
	QVector<double> lo(MaxPlots, std::numeric_limits<double>::max());
	QVector<double> hi(MaxPlots, std::numeric_limits<double>::lowest());
	QVector<bool> used(MaxPlots, false);

	for(Entry &e : m_entries) {
		if(!e.curve) {
			continue;
		}
		if(!m_monitor->isEnabled(e.key)) {
			e.curve->curve()->setSamples(QVector<QPointF>());
			continue;
		}
		used[e.plotIndex] = true;
		const QVector<QPointF> s = m_monitor->samples(e.key);
		QVector<float> x(s.size());
		QVector<float> y(s.size());
		for(int i = 0; i < s.size(); ++i) {
			x[i] = float(s[i].x() - now); // now = 0, history to the left
			y[i] = float(s[i].y());
			lo[e.plotIndex] = std::min(lo[e.plotIndex], s[i].y());
			hi[e.plotIndex] = std::max(hi[e.plotIndex], s[i].y());
		}
		e.curve->setSamples(x.data(), y.data(), size_t(s.size()), true);
		if(Attribute *src = m_monitor->source(e.key)) {
			m_plots[e.plotIndex]->yAxis()->scaleDraw()->setUnitType(src->unit());
		}
	}

	for(int p = 0; p < MaxPlots; ++p) {
		PlotWidget *plot = m_plots[p];
		plot->setVisible(used[p]);
		if(!used[p] || plot->navigator()->isZoomed()) {
			continue;
		}
		plot->xAxis()->setInterval(-history, 0);
		if(lo[p] <= hi[p]) {
			const double span =
				hi[p] > lo[p] ? hi[p] - lo[p] : std::max(1e-9, std::abs(hi[p]) * 0.01 + 1e-9);
			plot->yAxis()->setInterval(lo[p] - span * 0.1, hi[p] + span * 0.1);
		}
		plot->replot();
	}
	updateTable();
}

void MonitorView::updateTable()
{
	QList<const Entry *> rows;
	for(const Entry &e : m_entries) {
		if(m_monitor->isEnabled(e.key)) {
			rows.append(&e);
		}
	}
	m_table->setRowCount(rows.size());
	for(int r = 0; r < rows.size(); ++r) {
		const Entry *e = rows[r];
		const AttributeMonitor::Stats st = m_monitor->stats(e->key);
		Attribute *src = m_monitor->source(e->key);
		const QString unit = src ? src->unit() : QString();
		QPixmap swatch(10, 10);
		swatch.fill(e->color);
		auto *name = new QTableWidgetItem(QIcon(swatch), e->name + (src ? " · " + src->name() : QString()));
		m_table->setItem(r, 0, name);
		const bool has = st.count > 0;
		m_table->setItem(r, 1, cell(has ? siFormat(st.value, unit, 6) : QStringLiteral("—")));
		m_table->setItem(r, 2, cell(has ? siFormat(st.min, unit, 6) : QStringLiteral("—")));
		m_table->setItem(r, 3, cell(has ? siFormat(st.max, unit, 6) : QStringLiteral("—")));
		m_table->setItem(r, 4, cell(has ? siFormat(st.mean, unit, 6) : QStringLiteral("—")));
	}
}

void MonitorView::updateStatus()
{
	const int n = m_monitor->enabledKeys().size();
	m_status->setText(m_monitor->isRunning() ? QStringLiteral("Polling %1 channel(s) every %2")
							   .arg(n)
							   .arg(siFormat(m_monitor->period() / 1000.0, "s"))
						 : QStringLiteral("Stopped · %1 channel(s) selected").arg(n));
}

#include "moc_monitorview.cpp"
