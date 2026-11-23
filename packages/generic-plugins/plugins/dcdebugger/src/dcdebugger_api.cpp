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

#include "dcdebugger_api.h"

#include "acquisition/acquisitionview.h"
#include "acquisition/measurementstable.h"
#include "acquisition/streamacquisition.h"
#include "componenttreemodel.h"
#include "dcdebuggerplugin.h"
#include "dcdebuggertool.h"
#include "generation/generatorview.h"
#include "generation/streamgeneration.h"
#include "generation/waveform.h"
#include "log/codegeneratorpanel.h"
#include "log/commandlogmodel.h"
#include "log/commandlogview.h"
#include "monitor/attributemonitor.h"
#include "monitor/monitorview.h"
#include "nodepanel.h"
#include "ping/pingview.h"
#include "registers/registersession.h"
#include "registers/registerview.h"
#include "sidesplitter.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QLabel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QLoggingCategory>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTimer>

#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/attributewriter.h>
#include <component/commandmonitor.h>
#include <component/connection.h>
#include <component/ping.h>
#include <gui/docking/dockwrapperinterface.h>
#include <menucombo.h>
#include <menucontrolbutton.h>
#include <menuonoffswitch.h>
#include <menuspinbox.h>
#include <pluginbase/scopyjs.h>
#include <qcoro/qcorotask.h>
#include <widgets/searchbar.h>

#include <optional>

Q_LOGGING_CATEGORY(CAT_DCDEBUGGER_API, "DCDebugger_API")

using namespace scopy;
using namespace scopy::dcdebugger;

DCDebugger_API *DCDebugger_API::s_api = nullptr;

namespace {
constexpr const char *API_NAME = "dcdebugger";

// Runs `trigger`, then the event loop until `finish(code)` or the timeout (code 1). A finish
// during `trigger` itself is kept: no wait then.
template <typename Arm>
int waitFor(int timeoutMs, Arm arm)
{
	QEventLoop loop;
	int rc = -1;
	auto finish = [&rc, &loop](int code) {
		if(rc < 0) {
			rc = code;
			loop.quit();
		}
	};
	arm(&loop, finish);
	if(rc < 0) {
		QTimer::singleShot(timeoutMs, &loop, [&finish]() { finish(1); });
		loop.exec();
	}
	return rc;
}

// The response of an async device command, or none on timeout. The UI keeps running meanwhile.
template <typename T>
std::optional<T> await(QCoro::Task<T> &&task, int timeoutMs)
{
	std::optional<T> result;
	waitFor(timeoutMs, [&](QEventLoop *loop, auto finish) {
		QCoro::connect(std::move(task), loop, [&result, finish](T &&value) {
			result.emplace(std::move(value));
			finish(0);
		});
	});
	return result;
}

QString originPath(const QObject *obj) { return obj ? component::CommandMonitor::originOf(obj).path : QString(); }

bool writeFile(const QString &path, const QString &text)
{
	QFile f(path);
	if(!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
		qWarning(CAT_DCDEBUGGER_API) << "Cannot write" << path << f.errorString();
		return false;
	}
	f.write(text.toUtf8());
	return true;
}

QString toJson(const QJsonArray &array)
{
	return QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
}
} // namespace

DCDebugger_API::DCDebugger_API()
	: ApiObject()
{}

DCDebugger_API *DCDebugger_API::attach(DCDebuggerPlugin *plugin)
{
	if(!s_api) {
		s_api = new DCDebugger_API();
		s_api->setObjectName(API_NAME);
		ScopyJS::GetInstance()->registerApi(s_api);
	}
	s_api->m_plugins.removeAll(plugin);
	s_api->m_plugins.append(plugin);
	s_api->m_current = plugin;
	return s_api;
}

void DCDebugger_API::detach(DCDebuggerPlugin *plugin)
{
	if(!s_api) {
		return;
	}
	s_api->m_plugins.removeAll(plugin);
	s_api->m_plugins.removeAll(nullptr);
	if(s_api->m_current == plugin || !s_api->m_current) {
		s_api->m_current = s_api->m_plugins.isEmpty() ? nullptr : s_api->m_plugins.last();
	}
	if(s_api->m_plugins.isEmpty()) {
		ScopyJS::GetInstance()->unregisterApi(s_api);
		delete s_api;
		s_api = nullptr;
	}
}

DCDebuggerTool *DCDebugger_API::tool() const
{
	if(!m_current || !m_current->m_tool) {
		qWarning(CAT_DCDEBUGGER_API) << "No connected device";
		return nullptr;
	}
	return m_current->m_tool;
}

NodePanel *DCDebugger_API::panel() const
{
	DCDebuggerTool *t = tool();
	return t ? qobject_cast<NodePanel *>(t->m_panels->currentWidget()) : nullptr;
}

template <typename View>
View *DCDebugger_API::view() const
{
	NodePanel *p = panel();
	if(!p) {
		return nullptr;
	}
	if(View *v = p->findChild<View *>()) {
		return v;
	}
	// A floating dock is not a child of the panel.
	for(DockWrapperInterface *dock : std::as_const(p->m_docks)) {
		if(auto *w = dynamic_cast<QWidget *>(dock)) {
			if(View *v = w->findChild<View *>()) {
				return v;
			}
		}
	}
	return nullptr;
}

QModelIndex DCDebugger_API::find(const QString &path, const QString &backend) const
{
	DCDebuggerTool *t = tool();
	if(!t) {
		return {};
	}
	const QModelIndex idx = t->m_model->indexOfOrigin(path, backend, 0);
	if(!idx.isValid()) {
		qWarning(CAT_DCDEBUGGER_API) << "No node" << path << (backend.isEmpty() ? QString() : "in " + backend);
	}
	return idx;
}

QObject *DCDebugger_API::node(const QString &path, const QString &backend) const
{
	const QModelIndex idx = find(path, backend);
	return idx.isValid() ? tool()->m_model->objectAt(idx) : nullptr;
}

CommandLogView *DCDebugger_API::log() const
{
	DCDebuggerTool *t = tool();
	return t ? t->m_log : nullptr;
}

#define REQUIRE_VIEW(Type, name, fail)                                                                                 \
	Type *name = view<Type>();                                                                                     \
	if(!name) {                                                                                                    \
		qWarning(CAT_DCDEBUGGER_API) << "The selected node has no" << #Type;                                   \
		return fail;                                                                                           \
	}

// Layout

QVariantMap DCDebugger_API::layout() const
{
	if(m_current && m_current->m_tool) {
		m_current->m_tool->storeLayout();
	}
	return layoutstate::values();
}

void DCDebugger_API::setLayout(const QVariantMap &layout)
{
	layoutstate::setValues(layout);
	if(m_current && m_current->m_tool) {
		m_current->m_tool->restoreLayout();
	}
}

void DCDebugger_API::resetLayout() { setLayout({}); }

// Devices

QStringList DCDebugger_API::getDevices() const
{
	QStringList uris;
	for(const QPointer<DCDebuggerPlugin> &p : m_plugins) {
		if(p) {
			uris.append(p->param());
		}
	}
	return uris;
}

QString DCDebugger_API::getDevice() const { return m_current ? m_current->param() : QString(); }

bool DCDebugger_API::setDevice(const QString &uri)
{
	for(const QPointer<DCDebuggerPlugin> &p : m_plugins) {
		if(p && p->param() == uri) {
			m_current = p;
			return true;
		}
	}
	qWarning(CAT_DCDEBUGGER_API) << "No connected device" << uri << "- connected:" << getDevices();
	return false;
}

QStringList DCDebugger_API::getTools() const { return m_current ? QStringList{"DC Debugger"} : QStringList(); }

// Tree

QStringList DCDebugger_API::getBackends() const
{
	if(!m_current || !m_current->m_conn) {
		return {};
	}
	QStringList ids;
	for(const component::BackendId &id : m_current->m_conn->backends()) {
		ids.append(id);
	}
	return ids;
}

QStringList DCDebugger_API::getNodes(const QString &backend) const
{
	DCDebuggerTool *t = tool();
	if(!t) {
		return {};
	}
	QStringList paths;
	ComponentTreeModel *model = t->m_model;
	std::function<void(const QModelIndex &)> walk = [&](const QModelIndex &parent) {
		for(int r = 0; r < model->rowCount(parent); ++r) {
			const QModelIndex idx = model->index(r, 0, parent);
			if(!model->isGroup(idx)) {
				if(QObject *obj = model->objectAt(idx)) {
					const CommandOrigin origin = component::CommandMonitor::originOf(obj);
					if(backend.isEmpty() || origin.backendId == backend) {
						paths.append(origin.path);
					}
				}
			}
			walk(idx);
		}
	};
	walk({});
	return paths;
}

bool DCDebugger_API::selectNode(const QString &path, const QString &backend)
{
	const QModelIndex idx = find(path, backend);
	if(!idx.isValid()) {
		return false;
	}
	tool()->selectIndex(idx);
	return true;
}

QString DCDebugger_API::getSelectedNode() const
{
	NodePanel *p = panel();
	return p ? originPath(p->node()) : QString();
}

QStringList DCDebugger_API::getViews() const
{
	QStringList views;
	if(view<AcquisitionView>()) {
		views.append("Acquisition");
	}
	if(view<GeneratorView>()) {
		views.append("Generator");
	}
	if(view<MonitorView>()) {
		views.append("Monitor");
	}
	if(view<RegisterView>()) {
		views.append("Registers");
	}
	if(view<PingView>()) {
		views.append("Ping");
	}
	return views;
}

void DCDebugger_API::setFilter(const QString &text)
{
	if(DCDebuggerTool *t = tool()) {
		t->m_search->getLineEdit()->setText(text);
	}
}

QString DCDebugger_API::getFilter() const
{
	DCDebuggerTool *t = tool();
	return t ? t->m_search->getLineEdit()->text() : QString();
}

void DCDebugger_API::refresh()
{
	if(NodePanel *p = panel()) {
		p->refresh();
	}
}

void DCDebugger_API::rebuild()
{
	if(DCDebuggerTool *t = tool()) {
		t->rebuild();
	}
}

// Attributes

QStringList DCDebugger_API::getAttributes(const QString &path, const QString &backend) const
{
	QObject *obj = node(path, backend);
	if(!obj) {
		return {};
	}
	QStringList paths;
	for(component::Attribute *attr :
	    obj->findChildren<component::Attribute *>(QString(), Qt::FindDirectChildrenOnly)) {
		paths.append(originPath(attr));
	}
	return paths;
}

QString DCDebugger_API::readAttribute(const QString &path, const QString &backend, int timeoutMs)
{
	auto *attr = qobject_cast<component::Attribute *>(node(path, backend));
	if(!attr || !attr->readCapability()) {
		qWarning(CAT_DCDEBUGGER_API) << path << "is not a readable attribute";
		return {};
	}
	const auto response = await(attr->readCapability()->readAsync(), timeoutMs);
	if(!response) {
		qWarning(CAT_DCDEBUGGER_API) << "Reading" << path << "timed out";
		return {};
	}
	if(!*response) {
		qWarning(CAT_DCDEBUGGER_API) << "Reading" << path << "failed:" << response->error().errorString();
		return {};
	}
	return attr->cachedValue();
}

bool DCDebugger_API::writeAttribute(const QString &path, const QString &value, const QString &backend, int timeoutMs)
{
	auto *attr = qobject_cast<component::Attribute *>(node(path, backend));
	if(!attr || !attr->writeCapability()) {
		qWarning(CAT_DCDEBUGGER_API) << path << "is not a writable attribute";
		return false;
	}
	const auto response = await(attr->writeCapability()->writeAsync(value), timeoutMs);
	if(!response) {
		qWarning(CAT_DCDEBUGGER_API) << "Writing" << path << "timed out";
		return false;
	}
	if(!*response) {
		qWarning(CAT_DCDEBUGGER_API) << "Writing" << path << "failed:" << response->error().errorString();
		return false;
	}
	return true;
}

QString DCDebugger_API::getCachedValue(const QString &path, const QString &backend) const
{
	auto *attr = qobject_cast<component::Attribute *>(node(path, backend));
	return attr ? attr->cachedValue() : QString();
}

QStringList DCDebugger_API::getAttributeOptions(const QString &path, const QString &backend) const
{
	auto *attr = qobject_cast<component::Attribute *>(node(path, backend));
	return attr ? attr->options() : QStringList();
}

// Acquisition

bool DCDebugger_API::startAcquisition()
{
	REQUIRE_VIEW(AcquisitionView, v, false);
	v->m_runBtn->setChecked(true);
	return v->m_acq->isRunning();
}

bool DCDebugger_API::stopAcquisition()
{
	REQUIRE_VIEW(AcquisitionView, v, false);
	v->m_runBtn->setChecked(false);
	v->m_singleBtn->setChecked(false);
	return true;
}

bool DCDebugger_API::isAcquisitionRunning() const
{
	REQUIRE_VIEW(AcquisitionView, v, false);
	return v->m_acq->isRunning();
}

bool DCDebugger_API::singleAcquisition(int timeoutMs)
{
	REQUIRE_VIEW(AcquisitionView, v, false);
	if(v->m_acq->isRunning()) {
		qWarning(CAT_DCDEBUGGER_API) << "The acquisition is running: stop it first";
		return false;
	}
	const int rc = waitFor(timeoutMs, [&](QEventLoop *loop, auto finish) {
		connect(v->m_acq, &StreamAcquisition::runningChanged, loop, [finish](bool running) {
			if(!running) {
				finish(0);
			}
		});
		connect(v->m_acq, &StreamAcquisition::errorOccurred, loop, [finish](const QString &message) {
			qWarning(CAT_DCDEBUGGER_API) << "Single acquisition failed:" << message;
			finish(2);
		});
		v->m_singleBtn->setChecked(true);
	});
	if(rc == 1) {
		qWarning(CAT_DCDEBUGGER_API) << "Single acquisition timed out";
		stopAcquisition();
	}
	QCoreApplication::processEvents(); // the frame drawn and measured
	return rc == 0;
}

int DCDebugger_API::getAcquisitionSamples() const
{
	REQUIRE_VIEW(AcquisitionView, v, -1);
	return int(v->m_samples->value());
}

bool DCDebugger_API::setAcquisitionSamples(int samples)
{
	REQUIRE_VIEW(AcquisitionView, v, false);
	v->m_samples->setValue(samples);
	return int(v->m_samples->value()) == samples;
}

QString DCDebugger_API::getAcquisitionMode() const
{
	REQUIRE_VIEW(AcquisitionView, v, {});
	return v->m_mode->combo()->currentText();
}

bool DCDebugger_API::setAcquisitionMode(const QString &mode)
{
	REQUIRE_VIEW(AcquisitionView, v, false);
	const int i = v->m_mode->combo()->findText(mode, Qt::MatchFixedString);
	if(i < 0) {
		qWarning(CAT_DCDEBUGGER_API) << "Invalid mode" << mode << "- valid: Time, Spectrum, XY";
		return false;
	}
	v->m_mode->combo()->setCurrentIndex(i);
	return true;
}

QStringList DCDebugger_API::getAcquisitionChannels() const
{
	REQUIRE_VIEW(AcquisitionView, v, {});
	return v->m_channelBoxes.keys();
}

bool DCDebugger_API::setAcquisitionChannelEnabled(const QString &channel, bool enabled)
{
	REQUIRE_VIEW(AcquisitionView, v, false);
	QCheckBox *box = v->m_channelBoxes.value(channel);
	if(!box) {
		qWarning(CAT_DCDEBUGGER_API) << "No channel" << channel << "- channels:" << v->m_channelBoxes.keys();
		return false;
	}
	box->setChecked(enabled);
	return true;
}

QString DCDebugger_API::getMeasurements() const
{
	REQUIRE_VIEW(AcquisitionView, v, {});
	const QTableWidget *table = v->m_table;
	QJsonArray rows;
	for(int r = 0; r < table->rowCount(); ++r) {
		QJsonObject row;
		for(int c = 0; c < table->columnCount(); ++c) {
			const QTableWidgetItem *header = table->horizontalHeaderItem(c);
			const QTableWidgetItem *item = table->item(r, c);
			row.insert(header ? header->text() : QString::number(c), item ? item->text() : QString());
		}
		rows.append(row);
	}
	return toJson(rows);
}

bool DCDebugger_API::exportAcquisitionCsv(const QString &path) const
{
	REQUIRE_VIEW(AcquisitionView, v, false);
	const QString csv = v->csvData();
	if(csv.isEmpty()) {
		qWarning(CAT_DCDEBUGGER_API) << "Nothing to export: acquire a buffer first";
		return false;
	}
	return writeFile(path, csv);
}

QString DCDebugger_API::getAcquisitionStatus() const
{
	REQUIRE_VIEW(AcquisitionView, v, {});
	return v->m_status->text();
}

// Generator

bool DCDebugger_API::startGenerator()
{
	REQUIRE_VIEW(GeneratorView, v, false);
	v->apply(); // pending edits first
	v->m_runBtn->setChecked(true);
	return v->m_gen->isRunning();
}

bool DCDebugger_API::stopGenerator()
{
	REQUIRE_VIEW(GeneratorView, v, false);
	v->m_runBtn->setChecked(false);
	return true;
}

bool DCDebugger_API::isGeneratorRunning() const
{
	REQUIRE_VIEW(GeneratorView, v, false);
	return v->m_gen->isRunning();
}

QStringList DCDebugger_API::getGeneratorChannels() const
{
	REQUIRE_VIEW(GeneratorView, v, {});
	QStringList ids;
	for(const auto &ctrl : std::as_const(v->m_channels)) {
		ids.append(ctrl.id);
	}
	return ids;
}

QStringList DCDebugger_API::getWaveforms() const { return waveformNames(); }

bool DCDebugger_API::setGeneratorChannelEnabled(const QString &channel, bool enabled)
{
	REQUIRE_VIEW(GeneratorView, v, false);
	for(auto &ctrl : v->m_channels) {
		if(ctrl.id == channel) {
			ctrl.button->checkBox()->setChecked(enabled);
			v->apply();
			return true;
		}
	}
	qWarning(CAT_DCDEBUGGER_API) << "No channel" << channel << "- channels:" << getGeneratorChannels();
	return false;
}

bool DCDebugger_API::setGeneratorTone(const QString &channel, const QString &waveform, double frequency,
				      double amplitude, double offset, double phase)
{
	REQUIRE_VIEW(GeneratorView, v, false);
	for(auto &ctrl : v->m_channels) {
		if(ctrl.id != channel) {
			continue;
		}
		const int w = ctrl.waveform->combo()->findText(waveform, Qt::MatchFixedString);
		if(w < 0) {
			qWarning(CAT_DCDEBUGGER_API) << "Invalid waveform" << waveform << "- valid:" << waveformNames();
			return false;
		}
		ctrl.waveform->combo()->setCurrentIndex(w);
		ctrl.frequency->setValue(frequency);
		ctrl.amplitude->setValue(amplitude);
		ctrl.offset->setValue(offset);
		ctrl.phase->setValue(phase);
		v->apply();
		return true;
	}
	qWarning(CAT_DCDEBUGGER_API) << "No channel" << channel << "- channels:" << getGeneratorChannels();
	return false;
}

bool DCDebugger_API::setGeneratorSamples(int samples)
{
	REQUIRE_VIEW(GeneratorView, v, false);
	v->m_samples->setValue(samples);
	v->apply();
	return int(v->m_samples->value()) == samples;
}

bool DCDebugger_API::setGeneratorCyclic(bool cyclic)
{
	REQUIRE_VIEW(GeneratorView, v, false);
	if(!v->m_gen->supportsCyclic() && cyclic) {
		qWarning(CAT_DCDEBUGGER_API) << "The stream does not support cyclic buffers";
		return false;
	}
	v->m_cyclic->onOffswitch()->setChecked(cyclic);
	v->apply();
	return true;
}

QString DCDebugger_API::getGeneratorStatus() const
{
	REQUIRE_VIEW(GeneratorView, v, {});
	return v->m_status->text();
}

// Monitor

bool DCDebugger_API::startMonitor()
{
	REQUIRE_VIEW(MonitorView, v, false);
	v->m_runBtn->setChecked(true);
	return v->m_monitor->isRunning();
}

bool DCDebugger_API::stopMonitor()
{
	REQUIRE_VIEW(MonitorView, v, false);
	v->m_runBtn->setChecked(false);
	return true;
}

bool DCDebugger_API::isMonitorRunning() const
{
	REQUIRE_VIEW(MonitorView, v, false);
	return v->m_monitor->isRunning();
}

bool DCDebugger_API::clearMonitor()
{
	REQUIRE_VIEW(MonitorView, v, false);
	v->m_clearBtn->click();
	return true;
}

QStringList DCDebugger_API::getMonitorChannels() const
{
	REQUIRE_VIEW(MonitorView, v, {});
	return v->m_monitor->keys();
}

bool DCDebugger_API::setMonitorChannelEnabled(const QString &channel, bool enabled)
{
	REQUIRE_VIEW(MonitorView, v, false);
	auto *e = v->entry(channel);
	if(!e || !e->button) {
		qWarning(CAT_DCDEBUGGER_API) << "No channel" << channel << "- channels:" << v->m_monitor->keys();
		return false;
	}
	e->button->checkBox()->setChecked(enabled);
	return true;
}

bool DCDebugger_API::setMonitorPeriod(double seconds)
{
	REQUIRE_VIEW(MonitorView, v, false);
	v->m_period->setValue(seconds);
	return true;
}

QString DCDebugger_API::getMonitorValues() const
{
	REQUIRE_VIEW(MonitorView, v, {});
	const AttributeMonitor *m = v->m_monitor;
	QJsonArray rows;
	for(const QString &key : m->enabledKeys()) {
		const AttributeMonitor::Stats s = m->stats(key);
		rows.append(QJsonObject{{"channel", key},
					{"value", s.value},
					{"min", s.min},
					{"max", s.max},
					{"mean", s.mean},
					{"count", s.count},
					{"error", m->lastError(key)}});
	}
	return toJson(rows);
}

// Registers

QStringList DCDebugger_API::getRegisterTargets() const
{
	REQUIRE_VIEW(RegisterView, v, {});
	QStringList targets;
	for(int i = 0; i < v->m_target->combo()->count(); ++i) {
		targets.append(v->m_target->combo()->itemText(i));
	}
	return targets;
}

bool DCDebugger_API::setRegisterTarget(int index)
{
	REQUIRE_VIEW(RegisterView, v, false);
	if(index < 0 || index >= v->m_target->combo()->count()) {
		qWarning(CAT_DCDEBUGGER_API)
			<< "Invalid register target" << index << "- targets:" << getRegisterTargets();
		return false;
	}
	v->m_target->combo()->setCurrentIndex(index);
	return true;
}

QString DCDebugger_API::readRegister(const QString &address, int timeoutMs)
{
	REQUIRE_VIEW(RegisterView, v, {});
	RegisterSession *session = v->m_session;
	uint32_t addr = 0;
	if(!session->canRead() || !RegisterSession::parseHex(address, session->addressBits(), addr)) {
		qWarning(CAT_DCDEBUGGER_API) << "Cannot read register" << address;
		return {};
	}
	v->m_address->setText(RegisterSession::hex(addr, session->addressBits()));
	uint32_t value = 0;
	const int rc = waitFor(timeoutMs, [&](QEventLoop *loop, auto finish) {
		connect(session, &RegisterSession::readDone, loop, [&value, addr, finish](uint32_t a, uint32_t val) {
			if(a == addr) {
				value = val;
				finish(0);
			}
		});
		connect(session, &RegisterSession::entryAdded, loop, [addr, finish](const RegisterEntry &e) {
			if(e.address == addr && !e.ok) {
				finish(2);
			}
		});
		session->read(addr);
	});
	if(rc != 0) {
		qWarning(CAT_DCDEBUGGER_API) << "Reading register" << address << (rc == 1 ? "timed out" : "failed");
		return {};
	}
	return RegisterSession::hex(value, session->valueBits());
}

bool DCDebugger_API::writeRegister(const QString &address, const QString &value, int timeoutMs)
{
	REQUIRE_VIEW(RegisterView, v, false);
	RegisterSession *session = v->m_session;
	uint32_t addr = 0;
	uint32_t val = 0;
	if(!session->canWrite() || !RegisterSession::parseHex(address, session->addressBits(), addr) ||
	   !RegisterSession::parseHex(value, session->valueBits(), val)) {
		qWarning(CAT_DCDEBUGGER_API) << "Cannot write register" << address << "=" << value;
		return false;
	}
	v->m_address->setText(RegisterSession::hex(addr, session->addressBits()));
	v->m_value->setText(RegisterSession::hex(val, session->valueBits()));
	const int rc = waitFor(timeoutMs, [&](QEventLoop *loop, auto finish) {
		connect(session, &RegisterSession::entryAdded, loop, [addr, finish](const RegisterEntry &e) {
			if(e.address == addr && e.op == QLatin1String("write")) {
				finish(e.ok ? 0 : 2);
			}
		});
		session->write(addr, val);
	});
	if(rc != 0) {
		qWarning(CAT_DCDEBUGGER_API) << "Writing register" << address << (rc == 1 ? "timed out" : "failed");
	}
	return rc == 0;
}

// Ping

bool DCDebugger_API::ping(int timeoutMs)
{
	REQUIRE_VIEW(PingView, v, false);
	if(!v->m_ping) {
		return false;
	}
	const auto response = await(v->m_ping->checkReachableAsync(), timeoutMs);
	if(!response) {
		qWarning(CAT_DCDEBUGGER_API) << "Ping timed out";
		return false;
	}
	QCoreApplication::processEvents(); // the outcome is reported through reachabilityChecked
	return bool(*response) && v->m_ping->lastReachable();
}

bool DCDebugger_API::setPingMonitoring(bool enabled)
{
	REQUIRE_VIEW(PingView, v, false);
	v->m_monitoring->onOffswitch()->setChecked(enabled);
	return true;
}

// Command log

bool DCDebugger_API::setLogEnabled(bool enabled)
{
	CommandLogView *l = log();
	if(!l) {
		return false;
	}
	l->m_log->onOffswitch()->setChecked(enabled);
	return l->isRecording() == enabled;
}

bool DCDebugger_API::isLogEnabled() const
{
	CommandLogView *l = log();
	return l && l->isRecording();
}

void DCDebugger_API::clearLog()
{
	if(CommandLogView *l = log()) {
		l->m_clearBtn->click();
	}
}

int DCDebugger_API::getLogEntryCount() const
{
	CommandLogView *l = log();
	return l ? l->model()->entryCount() : 0;
}

QString DCDebugger_API::getLogCsv() const
{
	CommandLogView *l = log();
	return l ? l->model()->toCsv() : QString();
}

bool DCDebugger_API::exportLogCsv(const QString &path) const
{
	CommandLogView *l = log();
	return l && writeFile(path, l->model()->toCsv());
}

void DCDebugger_API::setLogFilter(const QString &component, const QString &library, const QString &function,
				  bool errorsOnly)
{
	CommandLogView *l = log();
	if(!l) {
		return;
	}
	l->m_component->setText(component);
	l->m_library->setText(library);
	l->m_function->setText(function);
	l->m_errorsOnly->onOffswitch()->setChecked(errorsOnly);
	l->applyFilter();
}

QStringList DCDebugger_API::getCodeLanguages()
{
	CommandLogView *l = log();
	if(!l) {
		return {};
	}
	l->refreshCodeGenerator();
	return l->m_codegen->languages();
}

QString DCDebugger_API::generateCode(const QString &language)
{
	CommandLogView *l = log();
	if(!l) {
		return {};
	}
	l->refreshCodeGenerator();
	if(!language.isEmpty()) {
		if(!l->m_codegen->languages().contains(language)) {
			qWarning(CAT_DCDEBUGGER_API)
				<< "Invalid language" << language << "- valid:" << l->m_codegen->languages();
			return {};
		}
		l->m_codegen->setLanguage(language);
	}
	return l->m_codegen->script();
}

bool DCDebugger_API::saveCode(const QString &path, const QString &language)
{
	CommandLogView *l = log();
	if(!l || generateCode(language).isEmpty()) {
		return false;
	}
	return l->m_codegen->saveTo(path);
}

#include "moc_dcdebugger_api.cpp"
