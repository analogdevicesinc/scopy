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

#include "channelmonitorview.h"

#include "acquisition/measurementstable.h"
#include "nodepanel.h"
#include "sidesplitter.h"

#include <QComboBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSet>
#include <QSignalBlocker>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <cmath>

#include <component/attribute.h>
#include <component/channel.h>
#include <component/device.h>
#include <component/navigation.h>
#include <dcdebugger/style_properties.h>
#include <menucombo.h>
#include <menusectionwidget.h>
#include <menuspinbox.h>
#include <mousewheelwidgetguard.h>
#include <style.h>
#include <toolbuttons.h>
#include <tooltemplate.h>

using namespace scopy;
using namespace scopy::dcdebugger;
using namespace scopy::component;

namespace {
const char *const SETTINGS_KEY = "settings";
const QString NONE = QStringLiteral("—");

enum Column
{
	COL_CHANNEL,
	COL_VALUE,
	COL_MIN,
	COL_MAX,
	COL_MEAN,
	COL_COUNT
};

QString number(double v) { return QString::number(v, 'g', 12); }

QTableWidgetItem *cell(const QString &text, Qt::Alignment align = Qt::AlignRight | Qt::AlignVCenter)
{
	auto *item = new QTableWidgetItem(text);
	item->setTextAlignment(align);
	return item;
}

QList<Attribute *> attributesOf(const Channel *ch)
{
	return ch->findChildren<Attribute *>(QString(), Qt::FindDirectChildrenOnly);
}
} // namespace

QList<Channel *> ChannelMonitorView::inputChannels(QObject *device)
{
	QList<Channel *> out;
	auto *dev = qobject_cast<Device *>(device);
	if(!dev) {
		return out;
	}
	for(Channel *ch : dev->findChildren<Channel *>(QString(), Qt::FindDirectChildrenOnly)) {
		if(!ch->isOutput() && !attributesOf(ch).isEmpty()) {
			out.append(ch);
		}
	}
	return out;
}

QStringList ChannelMonitorView::attributeNames(const QList<Channel *> &channels)
{
	QSet<QString> names;
	for(const Channel *ch : channels) {
		for(const Attribute *a : attributesOf(ch)) {
			if(a->readCapability()) {
				names.insert(a->name());
			}
		}
	}
	QStringList out(names.begin(), names.end());
	out.sort();
	return out;
}

ChannelMonitorView::ChannelMonitorView(QObject *device, QWidget *parent)
	: QWidget(parent)
	, m_root(device)
	, m_monitor(new AttributeMonitor(this))
	, m_redraw(new QTimer(this))
{
	m_monitor->setScaled(false); // raw values only
	m_monitor->setPeriod(DefaultPeriodMs);

	m_redraw->setInterval(100); // refresh the table at most 10 times a second, whatever the period
	connect(m_redraw, &QTimer::timeout, this, [this]() {
		if(m_dirty) {
			updateTable();
		}
	});
	connect(m_monitor, &AttributeMonitor::ticked, this, [this]() { m_dirty = true; });
	connect(m_monitor, &AttributeMonitor::sampled, this, [this]() { m_dirty = true; });

	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	m_tool = new ToolTemplate(this);
	m_tool->layout()->setContentsMargins(0, 0, 0, 0);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->centralContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	lay->addWidget(m_tool);
	// Table | Settings menu; no channel list.
	m_sides = new SideSplitter(0, 300, QStringLiteral("channelmonitor/sides"), this);
	m_tool->addWidgetToCentralContainerHelper(m_sides);

	m_status = new QLabel("Stopped", this);
	Style::setStyle(m_status, style::properties::label::subtle);
	m_runBtn = new RunBtn(this);
	disconnect(m_runBtn, &QPushButton::toggled, m_runBtn, nullptr); // RunBtn's own Run / Stop text
	m_runBtn->setText("Start");
	m_clearBtn = new QPushButton("Clear", this);
	Style::setStyle(m_clearBtn, style::properties::dcdebugger::topButton);
	auto *gearBtn = new GearBtn(this);
	gearBtn->setChecked(m_sides->isRightOpen());
	m_tool->addWidgetToTopContainerHelper(m_status, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(m_clearBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(gearBtn, TTA_RIGHT);
	connect(gearBtn, &QPushButton::toggled, m_sides, &SideSplitter::setRightOpen);
	connect(m_sides, &SideSplitter::rightOpenChanged, gearBtn, &QAbstractButton::setChecked);

	const QList<Channel *> channels = inputChannels(m_root);
	for(Channel *ch : channels) {
		Entry e;
		e.key = ch->id();
		// "voltage0 (vin)": the id, then the name when the channel has one.
		e.channel = ch->name().isEmpty() || ch->name() == ch->id()
			? ch->id()
			: QStringLiteral("%1 (%2)").arg(ch->id(), ch->name());
		e.ch = ch;
		m_monitor->addItem(e.key, nullptr);
		m_entries.append(e);
	}

	const QStringList names = attributeNames(channels);
	m_attribute = names.contains(QLatin1String(DefaultAttribute)) ? QString::fromLatin1(DefaultAttribute)
								      : names.value(0);
	m_sides->rightStack()->add(SETTINGS_KEY, buildSettings(this));
	m_sides->rightStack()->show(SETTINGS_KEY);
	buildTable();
	applyAttribute();

	connect(m_runBtn, &QPushButton::toggled, this, [this](bool on) {
		m_runBtn->setText(on ? "Stop" : "Start");
		if(on) {
			m_monitor->start();
			m_redraw->start();
		} else {
			m_monitor->stop();
			m_redraw->stop();
			updateTable();
		}
		updateStatus();
	});
	connect(m_clearBtn, &QPushButton::clicked, m_monitor, &AttributeMonitor::clear);
	connect(m_clearBtn, &QPushButton::clicked, this, &ChannelMonitorView::updateTable);
	connect(m_monitor, &AttributeMonitor::readFailed, this,
		[this](const QString &key, const QString &msg) { m_status->setText(key + ": " + msg); });
	updateStatus();
}

ChannelMonitorView::~ChannelMonitorView() = default;

QWidget *ChannelMonitorView::buildSettings(QWidget *parent)
{
	auto *scroll = new QScrollArea(parent);
	scroll->setWidgetResizable(true);
	scroll->setFrameShape(QFrame::NoFrame);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll->setObjectName("channelMonitorSettingsMenu");
	auto *content = new QWidget(scroll);
	auto *outer = new QVBoxLayout(content);
	outer->setContentsMargins(0, 0, 0, 0);
	outer->setSpacing(10);

	MenuSectionCollapseWidget *section = makeSection("Polling", content);
	section->setObjectName("channelMonitorPollingSection");
	QVBoxLayout *lay = section->contentLayout();
	lay->setSpacing(8);

	m_attrCombo = new MenuCombo("Attribute", section);
	m_attrCombo->combo()->setObjectName("channelMonitorAttribute");
	m_attrCombo->setToolTip("Attribute polled on every input channel that has it");
	m_attrCombo->combo()->addItems(attributeNames(inputChannels(m_root)));
	m_attrCombo->combo()->setCurrentText(m_attribute);
	connect(m_attrCombo->combo(), &QComboBox::currentTextChanged, this, &ChannelMonitorView::setAttributeName);

	m_period = new gui::MenuSpinbox("Period", m_monitor->period() / 1000.0, "s", 0.001, 3600, true, false, false,
					section);
	m_period->setObjectName("channelMonitorPeriod");
	m_period->setIncrementMode(gui::MenuSpinbox::IS_125);
	m_history =
		new gui::MenuSpinbox("History", m_monitor->history(), "s", 1, 24 * 3600, true, false, false, section);
	m_history->setObjectName("channelMonitorHistory");
	m_history->setIncrementMode(gui::MenuSpinbox::IS_125);
	m_history->setToolTip("Min / max / mean are computed over the samples of this window");

	lay->addWidget(m_attrCombo);
	lay->addWidget(m_period);
	lay->addWidget(m_history);
	connect(m_period, &gui::MenuSpinbox::valueChanged, this, [this](double s) {
		m_monitor->setPeriod(int(std::lround(s * 1000)));
		updateStatus();
	});
	connect(m_history, &gui::MenuSpinbox::valueChanged, this, [this](double s) {
		m_monitor->setHistory(s);
		m_dirty = true;
	});

	outer->addWidget(section);
	outer->addStretch();
	auto *guard = new MouseWheelWidgetGuard(content);
	guard->installEventRecursively(content);
	scroll->setWidget(content);
	return scroll;
}

void ChannelMonitorView::buildTable()
{
	m_table = new QTableWidget(this);
	m_table->setObjectName("channelMonitorTable");
	const QStringList headers = {"Channel", "Value", "Min", "Max", "Mean"};
	m_table->setColumnCount(COL_COUNT);
	m_table->setHorizontalHeaderLabels(headers);
	m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	m_table->horizontalHeader()->setSectionResizeMode(COL_CHANNEL, QHeaderView::ResizeToContents);
	m_table->verticalHeader()->setVisible(false);
	m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_table->setSelectionMode(QAbstractItemView::NoSelection);
	m_table->setFocusPolicy(Qt::NoFocus);
	Style::setStyle(m_table, style::properties::widget::tableViewWidget);
	m_sides->setCentralWidget(m_table);
}

void ChannelMonitorView::setAttributeName(const QString &name)
{
	if(name == m_attribute) {
		return;
	}
	m_attribute = name;
	if(m_attrCombo && m_attrCombo->combo()->currentText() != name) {
		const QSignalBlocker blocker(m_attrCombo->combo());
		m_attrCombo->combo()->setCurrentText(name);
	}
	applyAttribute();
}

void ChannelMonitorView::applyAttribute()
{
	for(const Entry &e : std::as_const(m_entries)) {
		Attribute *src = e.ch && !m_attribute.isEmpty() ? attributeByName(e.ch, m_attribute) : nullptr;
		if(src && !src->readCapability()) {
			src = nullptr;
		}
		m_monitor->setSource(e.key, src); // history restarts
		m_monitor->setEnabled(e.key, src != nullptr);
	}
	m_table->setRowCount(0); // rows follow the polled channels
	updateTable();
	updateStatus();
}

QStringList ChannelMonitorView::polledKeys() const
{
	QStringList out;
	for(const Entry &e : m_entries) {
		if(m_monitor->isEnabled(e.key)) {
			out.append(e.key);
		}
	}
	return out;
}

void ChannelMonitorView::updateTable()
{
	m_dirty = false;
	const QStringList keys = polledKeys();
	const bool rebuild = m_table->rowCount() != keys.size();
	m_table->setRowCount(keys.size());
	int r = 0;
	for(const Entry &e : std::as_const(m_entries)) {
		if(!m_monitor->isEnabled(e.key)) {
			continue;
		}
		if(rebuild || !m_table->item(r, COL_CHANNEL)) {
			m_table->setItem(r, COL_CHANNEL, cell(e.channel, Qt::AlignLeft | Qt::AlignVCenter));
			for(int c = COL_VALUE; c < COL_COUNT; ++c) {
				m_table->setItem(r, c, cell(NONE));
			}
		}
		const AttributeMonitor::Stats st = m_monitor->stats(e.key);
		const bool has = st.count > 0;
		m_table->item(r, COL_VALUE)->setText(has ? number(st.value) : NONE);
		m_table->item(r, COL_MIN)->setText(has ? number(st.min) : NONE);
		m_table->item(r, COL_MAX)->setText(has ? number(st.max) : NONE);
		m_table->item(r, COL_MEAN)->setText(has ? number(st.mean) : NONE);
		const QString err = m_monitor->lastError(e.key);
		for(int c = 0; c < COL_COUNT; ++c) {
			m_table->item(r, c)->setToolTip(err);
		}
		++r;
	}
}

void ChannelMonitorView::updateStatus()
{
	const int n = polledKeys().size();
	m_status->setText(m_monitor->isRunning()
				  ? QStringLiteral("Polling %1 · %2 channel(s) every %3")
					    .arg(m_attribute)
					    .arg(n)
					    .arg(siFormat(m_monitor->period() / 1000.0, "s"))
				  : QStringLiteral("Stopped · %1 channel(s) with %2").arg(n).arg(m_attribute));
}

#include "moc_channelmonitorview.cpp"
