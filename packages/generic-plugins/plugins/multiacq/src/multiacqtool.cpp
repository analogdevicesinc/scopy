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

#include "multiacqtool.h"

#include "iiosource.h"
#include "timeplot.h"

#include <acqinstrument.h>

#include <core/acq_engine/acquisitionengine.h>
#include <core/acq_engine/datastore.h>

#include <component/attribute.h>
#include <component/context.h>
#include <component/controller.h>
#include <component/navigation.h>

#include <gui/docking/dockablearea.h>
#include <gui/docking/dockwrapper.h>
#include <gui/instrumenttemplate.h>
#include <gui/plotchannel.h>
#include <gui/plotwidget.h>
#include <gui/smallOnOffSwitch.h>
#include <gui/style.h>
#include <gui/style_attributes.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/menulineedit.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menuspinbox.h>
#include <gui/widgets/toolbuttons.h>
#include <pluginbase/preferences.h>
#include <pluginbase/toolmenuentry.h>

#include <QApplication>
#include <QColor>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

using namespace scopy;
using namespace scopy::multiacq;

namespace {

constexpr int kFrameIntervalMs = 33;
// Settings changes arrive in bursts; one restart covers them.
constexpr int kRestartDelayMs = 300;

const QString kAddDeviceId = QStringLiteral("multiacq:device:add");

constexpr int kMaxNameLength = 40;

double nowMs() { return static_cast<double>(QDateTime::currentMSecsSinceEpoch()); }

QWidget *widgetPage(InstrumentTemplate *it, QWidget *body, const QString &title, QWidget *footer = nullptr)
{
	QWidget *page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(page);
	lay->setContentsMargins(0, 0, 0, 0);
	MenuSectionCollapseWidget *section = it->createMenuSection(title, SO_DEV, page);
	section->add(body);
	lay->addWidget(section);
	if(footer) {
		lay->addWidget(footer);
	}
	lay->addStretch();
	return page;
}

QPushButton *borderButton(const QString &text, QWidget *parent)
{
	QPushButton *btn = new QPushButton(text, parent);
	Style::setStyle(btn, style::properties::button::borderButton);
	return btn;
}

// A bare address is taken as a network one, the way most boards are reached.
QString normalizeUri(QString uri)
{
	uri = uri.trimmed();
	if(!uri.isEmpty() && !uri.contains(':')) {
		uri.prepend("ip:");
	}
	return uri;
}

} // namespace

MultiAcqTool::MultiAcqTool(ToolMenuEntry *tme, QWidget *parent)
	: QWidget(parent)
	, m_tme(tme)
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	m_ui = new adc::AcqInstrument(this);
	lay->addWidget(m_ui);

	InstrumentTemplate *it = m_ui->shell();
	m_dockArea = createDockableArea(it);
	QWidget *area = m_dockArea->asWidget();
	Style::setBackgroundColor(area, json::theme::background_subtle, true);
	it->setCenterWidget(area);

	setupTopRail();
	setupDevicesRail();
	setupPlotsRail();
	setupFrameTimer();
}

MultiAcqTool::~MultiAcqTool()
{
	// Stop before the plots go: the engine still writes the keys they claim.
	m_ui->stop();
	for(PlotEntry *e : std::as_const(m_plots)) {
		delete e->plot;
		delete e;
	}
	m_plots.clear();
}

void MultiAcqTool::setupTopRail()
{
	InstrumentTemplate *it = m_ui->shell();

	QPushButton *clearBtn = borderButton(tr("Clear"), it);
	clearBtn->setToolTip(tr("Clear every plot and restart the relative time"));
	connect(clearBtn, &QPushButton::clicked, this, &MultiAcqTool::clearData);
	it->addEngineControl(clearBtn);

	connect(it->singleBtn(), &QAbstractButton::toggled, this, [this](bool checked) {
		if(checked) {
			m_ui->single();
		}
	});

	if(m_tme) {
		connect(m_tme, &ToolMenuEntry::runToggled, m_ui, [this](bool on) { on ? m_ui->run() : m_ui->stop(); });
	}
	connect(m_ui, &adc::AcqInstrument::started, this, [this]() {
		if(m_devices.isEmpty()) {
			// Nothing paces an engine with no sources; it would only spin.
			QMetaObject::invokeMethod(m_ui, &adc::AcqInstrument::stop, Qt::QueuedConnection);
			return;
		}
		if(m_tme) {
			m_tme->setRunning(true);
		}
		if(m_needStartTime) {
			m_needStartTime = false;
			setStartTime(nowMs());
		}
		m_frameTimer->start();
	});
	connect(m_ui, &adc::AcqInstrument::stopped, this, [this]() {
		if(m_tme) {
			m_tme->setRunning(false);
		}
		m_frameTimer->stop();
		// The last cycle may have landed after the last frame.
		updatePlots();
	});
}

void MultiAcqTool::setupFrameTimer()
{
	// Plots move with the clock, not with the cycles: a slow device must not freeze the axis.
	m_frameTimer = new QTimer(this);
	m_frameTimer->setInterval(kFrameIntervalMs);
	connect(m_frameTimer, &QTimer::timeout, this, &MultiAcqTool::updatePlots);

	m_restartTimer = new QTimer(this);
	m_restartTimer->setSingleShot(true);
	m_restartTimer->setInterval(kRestartDelayMs);
	connect(m_restartTimer, &QTimer::timeout, this, [this]() {
		if(m_ui->engine()->isRunning()) {
			m_ui->stop();
			m_ui->run();
		}
	});
}

void MultiAcqTool::updatePlots()
{
	for(PlotEntry *e : std::as_const(m_plots)) {
		e->plot->update();
	}
}

void MultiAcqTool::clearData()
{
	for(PlotEntry *e : std::as_const(m_plots)) {
		e->plot->clear();
	}
	m_needStartTime = !m_ui->engine()->isRunning();
	if(!m_needStartTime) {
		setStartTime(nowMs());
	}
}

void MultiAcqTool::setStartTime(double ms)
{
	m_startTime = ms;
	for(PlotEntry *e : std::as_const(m_plots)) {
		e->plot->setStartTime(ms);
	}
}

void MultiAcqTool::scheduleRestart()
{
	if(m_ui->engine()->isRunning()) {
		m_restartTimer->start();
	}
}

void MultiAcqTool::setupDevicesRail()
{
	InstrumentTemplate *it = m_ui->shell();
	m_devicesGroup = it->addChannelGroup(tr("Devices"));
	it->addRailActionButton(m_devicesGroup, tr("Add device"), kAddDeviceId);

	QWidget *page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(page);
	lay->setContentsMargins(0, 0, 0, 0);
	MenuSectionCollapseWidget *section = it->createMenuSection(tr("ADD DEVICE"), SO_DEV, page);

	MenuLineEdit *uriEdit = new MenuLineEdit(section);
	uriEdit->edit()->setPlaceholderText(tr("URI, e.g. ip:192.168.2.1"));
	section->add(uriEdit);

	QPushButton *connectBtn = borderButton(tr("Connect"), section);
	section->add(connectBtn);

	QLabel *status = new QLabel(section);
	status->setWordWrap(true);
	section->add(status);

	auto doConnect = [this, uriEdit, connectBtn, status]() {
		const QString uri = normalizeUri(uriEdit->edit()->text());
		if(uri.isEmpty()) {
			status->setText(tr("Type the device URI first."));
			return;
		}
		status->setText(tr("Connecting to %1…").arg(uri));
		connectBtn->setEnabled(false);
		// Connecting blocks; paint the message before it does.
		status->repaint();
		QApplication::setOverrideCursor(Qt::WaitCursor);
		const QString err = addDevice(uri);
		QApplication::restoreOverrideCursor();
		connectBtn->setEnabled(true);
		if(err.isEmpty()) {
			status->setText(tr("Connected to %1. Enable its channels on its page, then add them to a plot.").arg(uri));
			uriEdit->edit()->clear();
		} else {
			status->setText(err);
		}
	};
	connect(connectBtn, &QPushButton::clicked, this, doConnect);
	connect(uriEdit->edit(), &QLineEdit::returnPressed, this, doConnect);

	lay->addWidget(section);
	lay->addStretch();
	it->addMenuPage(kAddDeviceId, page);
}

QString MultiAcqTool::nextSourceId() const
{
	// Internal and never shown: a DataKey source id may not hold '_', which a device name may.
	for(int n = 1;; ++n) {
		const QString id = QStringLiteral("dev") + QString::number(n);
		if(!sourceById(id)) {
			return id;
		}
	}
}

QString MultiAcqTool::deviceName(component::Context *ctx, const QString &uri) const
{
	QString name;
	if(component::Attribute *model = component::attributeByName(ctx, "hw_model")) {
		name = model->cachedValue().trimmed();
	}
	if(name.isEmpty()) {
		name = ctx->description().trimmed();
	}
	if(name.isEmpty() || name.size() > kMaxNameLength) {
		name = name.isEmpty() ? uri : name.left(kMaxNameLength - 1) + QStringLiteral("…");
	}
	QString unique = name;
	for(int n = 2;; ++n) {
		bool taken = false;
		for(const DeviceEntry &d : m_devices) {
			taken = taken || d.src->displayName() == unique;
		}
		if(!taken) {
			return unique;
		}
		unique = name + QStringLiteral(" (%1)").arg(n);
	}
}

QString MultiAcqTool::addDevice(const QString &uri)
{
	for(const DeviceEntry &d : std::as_const(m_devices)) {
		if(d.src->uri() == uri) {
			return tr("%1 is already added.").arg(uri);
		}
	}

	// Shared with a connection Scopy already holds for the same URI, if any.
	const int maxThreads =
		Preferences::get("dc_cmd_execution_parallel").toBool() ? QThread::idealThreadCount() : 1;
	component::ContextHandle ctx =
		component::Controller::connectCtx(uri, component::BackendKind::Libiio, maxThreads);
	if(!ctx) {
		return tr("Could not connect to %1.").arg(uri);
	}

	const QString id = nextSourceId();
	const QString name = deviceName(ctx.get(), uri);
	IioSource *src = new IioSource(id, uri, std::move(ctx), m_ui->engine());
	src->setDisplayName(name);
	if(!src->isAvailable()) {
		delete src;
		return tr("%1 has no channels that can be read.").arg(uri);
	}

	InstrumentTemplate *it = m_ui->shell();
	DeviceEntry entry;
	entry.src = src;
	entry.menuId = QStringLiteral("multiacq:device:") + id;
	entry.row = it->addChannelRow(m_devicesGroup, name, QColor(), entry.menuId);
	entry.row->setToolTip(uri);
	it->addMenuPage(entry.menuId, createDevicePage(src));

	QCheckBox *chk = entry.row->checkBox();
	chk->setChecked(src->isEnabled());
	connect(chk, &QAbstractButton::toggled, src, &acq::Block::setEnabled);
	connect(src, &acq::Block::enabledChanged, chk, &QCheckBox::setChecked);

	MenuControlButton *row = entry.row;
	connect(
		src, &IioSource::onlineChanged, row,
		[row, name](bool online) { row->setName(online ? name : name + tr(" (offline)")); },
		Qt::QueuedConnection);
	connect(src, &IioSource::configChanged, this, &MultiAcqTool::scheduleRestart);
	connect(src, &acq::SourceBlock::channelEnabledChanged, this, &MultiAcqTool::refreshPickers);

	m_devices.append(entry);
	m_ui->engine()->addSource(src);
	refreshPickers();
	return QString();
}

QWidget *MultiAcqTool::createDevicePage(IioSource *src)
{
	InstrumentTemplate *it = m_ui->shell();
	QPushButton *removeBtn = borderButton(tr("Remove device"), nullptr);
	// Queued: the page holding the button goes with the device.
	connect(
		removeBtn, &QPushButton::clicked, this, [this, src]() { removeDevice(src); }, Qt::QueuedConnection);
	return widgetPage(it, src->settingsWidget(), src->displayName().toUpper(), removeBtn);
}

void MultiAcqTool::removeDevice(IioSource *src)
{
	int idx = -1;
	for(int i = 0; i < m_devices.size(); ++i) {
		if(m_devices[i].src == src) {
			idx = i;
		}
	}
	if(idx < 0) {
		return;
	}
	const DeviceEntry entry = m_devices.takeAt(idx);

	for(PlotEntry *e : std::as_const(m_plots)) {
		for(TimeCurve *c : e->curves.keys()) {
			if(c->yKey().sourceId() == src->id()) {
				removeChannel(e->plot, c);
			}
		}
	}

	// Stop first, so the block's lanes and connection are gone before anything else runs.
	const bool wasRunning = m_ui->engine()->isRunning();
	m_ui->stop();
	m_ui->shell()->removeChannelRow(m_devicesGroup, entry.row, entry.menuId, /*deletePage=*/true);
	m_ui->engine()->removeSource(src);
	if(wasRunning && !m_devices.isEmpty()) {
		m_ui->run();
	}
	refreshPickers();
}

IioSource *MultiAcqTool::sourceById(const QString &id) const
{
	for(const DeviceEntry &d : m_devices) {
		if(d.src->id() == id) {
			return d.src;
		}
	}
	return nullptr;
}

QString MultiAcqTool::channelLabel(const acq::DataKey &yKey) const
{
	// "Alif E8 Linux › adxl345 › accel_x"
	IioSource *src = sourceById(yKey.sourceId());
	return (src ? src->displayName() : yKey.sourceId()) + QStringLiteral(" › ") +
		QString(yKey.channelId()).replace(':', QStringLiteral(" › "));
}

void MultiAcqTool::setupPlotsRail()
{
	InstrumentTemplate *it = m_ui->shell();
	m_plotsGroup = it->addChannelGroup(tr("Plots"));
	QPushButton *addBtn = it->addRailActionButton(m_plotsGroup, tr("Add plot"), QString());
	connect(addBtn, &QPushButton::clicked, this, [this]() {
		TimePlot *plot = addPlot(QString());
		// Opens its page, where channels are added.
		if(PlotEntry *e = plotEntry(plot)) {
			e->row->getControlBtn()->click();
		}
	});
}

MultiAcqTool::PlotEntry *MultiAcqTool::plotEntry(TimePlot *plot) const
{
	for(PlotEntry *e : m_plots) {
		if(e->plot == plot) {
			return e;
		}
	}
	return nullptr;
}

TimePlot *MultiAcqTool::addPlot(const QString &name)
{
	const quint32 uuid = m_plotUuid++;
	const QString title = name.isEmpty() ? tr("Plot %1").arg(uuid + 1) : name;

	PlotEntry *entry = new PlotEntry();
	entry->dock = createDockWrapper(title);
	entry->dock->setTitle(title);
	TimePlot *plot = new TimePlot(title, uuid, m_ui->store(), nullptr);
	entry->plot = plot;
	entry->dock->setInnerWidget(plot->plot());
	m_dockArea->addDockWrapper(entry->dock, DockableAreaInterface::Direction_BOTTOM);
	if(m_startTime > 0) {
		plot->setStartTime(m_startTime);
	}

	InstrumentTemplate *it = m_ui->shell();
	entry->row = it->addExpandableChannelRow(m_plotsGroup, title, QColor(), plot->menuId());
	it->addMenuPage(plot->menuId(), createPlotPage(entry));

	DockWrapperInterface *dock = entry->dock;
	MenuControlButton *hdr = entry->row->getControlBtn();
	connect(plot, &TimePlot::nameChanged, this, [dock, hdr](const QString &n) {
		dock->setTitle(n);
		hdr->setName(n);
	});

	m_plots.append(entry);
	return plot;
}

void MultiAcqTool::removePlot(TimePlot *plot)
{
	PlotEntry *entry = plotEntry(plot);
	if(!entry) {
		return;
	}
	m_plots.removeOne(entry);

	InstrumentTemplate *it = m_ui->shell();
	for(auto c = entry->curves.cbegin(); c != entry->curves.cend(); ++c) {
		it->removeChannelRow(entry->row, c.value().first, QString());
	}
	m_plotsGroup->remove(entry->row);
	it->removeChannelRow(nullptr, entry->row->getControlBtn(), plot->menuId(), /*deletePage=*/true);
	entry->row->deleteLater();

	delete plot;
	if(QWidget *w = dynamic_cast<QWidget *>(entry->dock)) {
		w->deleteLater();
	}
	delete entry;
}

QWidget *MultiAcqTool::createPlotPage(PlotEntry *entry)
{
	InstrumentTemplate *it = m_ui->shell();
	TimePlot *plot = entry->plot;

	QWidget *page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(page);
	lay->setContentsMargins(0, 0, 0, 0);

	MenuSectionCollapseWidget *plotSection = it->createMenuSection(tr("PLOT"), SO_VIEW, page);
	MenuLineEdit *nameEdit = new MenuLineEdit(plotSection);
	nameEdit->edit()->setText(plot->name());
	connect(nameEdit->edit(), &QLineEdit::editingFinished, plot,
		[plot, nameEdit]() { plot->setName(nameEdit->edit()->text()); });
	plotSection->add(nameEdit);
	MenuOnOffSwitch *legendSw = new MenuOnOffSwitch(tr("Show legend"), plotSection);
	legendSw->onOffswitch()->setChecked(plot->showLegend());
	connect(legendSw->onOffswitch(), &QAbstractButton::toggled, plot, &TimePlot::setShowLegend);
	plotSection->add(legendSw);
	lay->addWidget(plotSection);

	entry->channelList = it->createMenuSection(tr("CHANNELS"), SO_VIEW, page);
	entry->picker = new MenuCombo(tr("Channel"), entry->channelList);
	fillPicker(entry->picker);
	entry->channelList->add(entry->picker);
	QPushButton *addBtn = borderButton(tr("Add to plot"), entry->channelList);
	connect(addBtn, &QPushButton::clicked, this, [this, entry]() {
		const QString key = entry->picker->combo()->currentData().toString();
		if(!key.isEmpty()) {
			addChannel(entry->plot, acq::DataKey(key));
		}
	});
	entry->channelList->add(addBtn);
	lay->addWidget(entry->channelList);

	MenuSectionCollapseWidget *xSection = it->createMenuSection(tr("X AXIS"), SO_VIEW, page);
	MenuCombo *labelsCombo = new MenuCombo(tr("Time labels"), xSection);
	labelsCombo->combo()->addItems({tr("Since start"), tr("Time of day")});
	labelsCombo->combo()->setCurrentIndex(plot->relative() ? 0 : 1);
	connect(labelsCombo->combo(), &QComboBox::currentIndexChanged, plot,
		[plot](int i) { plot->setRelative(i == 0); });
	xSection->add(labelsCombo);

	const QList<gui::UnitPrefix> timeScales{{QString("s"), 1}, {QString("min"), 60}, {QString("H"), 3600}};
	gui::MenuSpinbox *span =
		new gui::MenuSpinbox(tr("Time span"), plot->span(), "", 0.1, 86400, true, false, false, xSection);
	span->setScaleList(timeScales);
	span->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	span->setValue(plot->span());
	span->setToolTip(tr("Time shown across the plot"));
	connect(span, &gui::MenuSpinbox::valueChanged, plot, &TimePlot::setSpan);
	xSection->add(span);

	gui::MenuSpinbox *history =
		new gui::MenuSpinbox(tr("Keep history"), plot->history(), "", 1, 86400, true, false, false, xSection);
	history->setScaleList(timeScales);
	history->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	history->setValue(plot->history());
	history->setToolTip(tr("Data kept for scrolling back, at least the time span"));
	connect(history, &gui::MenuSpinbox::valueChanged, plot, &TimePlot::setHistory);
	connect(span, &gui::MenuSpinbox::valueChanged, history, [plot, history]() {
		history->setValueSilent(plot->history());
	});
	xSection->add(history);

	MenuLineEdit *formatEdit = new MenuLineEdit(xSection);
	formatEdit->edit()->setText(plot->timeFormat());
	formatEdit->edit()->setToolTip(tr("Label format: hh:mm:ss, mm:ss.zzz, dd.MM hh:mm ..."));
	connect(formatEdit->edit(), &QLineEdit::editingFinished, plot,
		[plot, formatEdit]() { plot->setTimeFormat(formatEdit->edit()->text().trimmed()); });
	xSection->add(new QLabel(tr("Time format"), xSection));
	xSection->add(formatEdit);

	MenuOnOffSwitch *liveSw = new MenuOnOffSwitch(tr("Follow live"), xSection);
	liveSw->onOffswitch()->setChecked(plot->live());
	liveSw->setToolTip(tr("Keep the newest data in view. Zooming in pauses it until the zoom is reset."));
	connect(liveSw->onOffswitch(), &QAbstractButton::toggled, plot, &TimePlot::setLive);
	xSection->add(liveSw);
	lay->addWidget(xSection);

	MenuSectionCollapseWidget *ySection = it->createMenuSection(tr("Y AXIS"), SO_VIEW, page);
	MenuOnOffSwitch *autoSw = new MenuOnOffSwitch(tr("Autoscale"), ySection);
	autoSw->onOffswitch()->setChecked(plot->autoscale());
	ySection->add(autoSw);
	gui::MenuSpinbox *yMin =
		new gui::MenuSpinbox(tr("Min"), plot->yMin(), "", -1e12, 1e12, true, false, false, ySection);
	gui::MenuSpinbox *yMax =
		new gui::MenuSpinbox(tr("Max"), plot->yMax(), "", -1e12, 1e12, true, false, false, ySection);
	for(gui::MenuSpinbox *s : {yMin, yMax}) {
		s->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
		s->setEnabled(!plot->autoscale());
		ySection->add(s);
	}
	connect(autoSw->onOffswitch(), &QAbstractButton::toggled, plot, [plot, yMin, yMax](bool on) {
		plot->setAutoscale(on);
		yMin->setEnabled(!on);
		yMax->setEnabled(!on);
	});
	auto applyY = [plot, yMin, yMax]() { plot->setYRange(yMin->value(), yMax->value()); };
	connect(yMin, &gui::MenuSpinbox::valueChanged, plot, applyY);
	connect(yMax, &gui::MenuSpinbox::valueChanged, plot, applyY);
	// Autoscale moves the range; the fields show where it went.
	connect(plot, &TimePlot::yRangeChanged, ySection, [yMin, yMax](double lo, double hi) {
		yMin->setValueSilent(lo);
		yMax->setValueSilent(hi);
	});
	lay->addWidget(ySection);

	QPushButton *delBtn = borderButton(tr("Delete plot"), page);
	// Queued: this page goes with the plot.
	connect(
		delBtn, &QPushButton::clicked, this, [this, plot]() { removePlot(plot); }, Qt::QueuedConnection);
	lay->addWidget(delBtn);
	lay->addStretch();
	return page;
}

void MultiAcqTool::fillPicker(MenuCombo *picker) const
{
	QComboBox *combo = picker->combo();
	const QString current = combo->currentData().toString();
	QSignalBlocker b(combo);
	combo->clear();
	for(const DeviceEntry &d : m_devices) {
		for(const IioSource::DeviceDesc &dev : d.src->devices()) {
			for(const IioSource::ChannelDesc &ch : dev.channels) {
				// Only what is being read: enabling a channel is done on its device.
				if(!d.src->isChannelEnabled(ch.channelId)) {
					continue;
				}
				const acq::DataKey key = acq::DataKey::raw(d.src->id(), ch.channelId);
				combo->addItem(channelLabel(key), key.toString());
			}
		}
	}
	const int idx = combo->findData(current);
	if(idx >= 0) {
		combo->setCurrentIndex(idx);
	}
	if(combo->count() == 0) {
		combo->addItem(tr("Enable a channel on a device first"), QString());
	}
}

void MultiAcqTool::refreshPickers()
{
	for(PlotEntry *e : std::as_const(m_plots)) {
		fillPicker(e->picker);
	}
}

void MultiAcqTool::addChannel(TimePlot *plot, const acq::DataKey &yKey)
{
	PlotEntry *entry = plotEntry(plot);
	IioSource *src = sourceById(yKey.sourceId());
	if(!entry || !src) {
		return;
	}
	for(TimeCurve *c : entry->curves.keys()) {
		if(c->yKey() == yKey) {
			return;
		}
	}

	const QString label = channelLabel(yKey);
	const QColor color = nextColor(plot);
	TimeCurve *curve = plot->addCurve(yKey, IioSource::timeKey(yKey.sourceId(), yKey.channelId()), label, color);

	InstrumentTemplate *it = m_ui->shell();
	// Opens the plot's page: that is where the channel is managed.
	MenuControlButton *row = it->addChannelSwitchRow(entry->row, label, color, plot->menuId());
	if(SmallOnOffSwitch *sw = InstrumentTemplate::rowSwitch(row)) {
		sw->setChecked(true);
		connect(sw, &QAbstractButton::toggled, curve, &TimeCurve::setVisible);
	}

	QWidget *line = new QWidget(entry->channelList);
	QHBoxLayout *lineLay = new QHBoxLayout(line);
	lineLay->setContentsMargins(0, 0, 0, 0);
	QLabel *name = new QLabel(label, line);
	name->setStyleSheet(QStringLiteral("color: %1;").arg(color.name()));
	lineLay->addWidget(name, 1);
	QPushButton *removeBtn = borderButton(tr("Remove"), line);
	connect(
		removeBtn, &QPushButton::clicked, this, [this, plot, curve]() { removeChannel(plot, curve); },
		Qt::QueuedConnection);
	lineLay->addWidget(removeBtn);
	entry->channelList->add(line);

	entry->curves.insert(curve, {row, line});
}

QColor MultiAcqTool::nextColor(TimePlot *plot) const
{
	// The theme colour least used on this plot. Never an index past the palette:
	// Style::getChannelColor() returns an invalid colour for it, which draws black.
	const int count = Style::getChannelColorList().size();
	if(count == 0) {
		return Style::getChannelColor(0);
	}
	QVector<int> used(count, 0);
	for(TimeCurve *c : plot->curves()) {
		for(int i = 0; i < count; ++i) {
			if(c->color() == Style::getChannelColor(i)) {
				++used[i];
			}
		}
	}
	return Style::getChannelColor(static_cast<int>(std::min_element(used.cbegin(), used.cend()) - used.cbegin()));
}

void MultiAcqTool::removeChannel(TimePlot *plot, TimeCurve *curve)
{
	PlotEntry *entry = plotEntry(plot);
	if(!entry || !entry->curves.contains(curve)) {
		return;
	}
	const auto widgets = entry->curves.take(curve);
	m_ui->shell()->removeChannelRow(entry->row, widgets.first, QString());
	entry->channelList->remove(widgets.second);
	widgets.second->deleteLater();
	plot->removeCurve(curve);
}

#include "moc_multiacqtool.cpp"
