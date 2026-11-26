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

#include "adalm2000logic_api.h"

#include "m2klogiccontroller.h"
#include "m2ksource.h"

#include <core/acq_engine/acquisitionengine.h>
#include <core/acq_engine/datastore.h>
#include <core/acq_ui/acqinstrument.h>
#include <core/decoder/decodermanager.h>
#include <core/decoder/decoderpanel.h>

#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menuspinbox.h>

#include <QAbstractButton>
#include <QComboBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>

using namespace scopy;
using namespace scopy::adalm2000;
using Controller = M2kLogicController;

namespace {
int pinOf(const scopy::acq::DataKey &key)
{
	const QString id = key.channelId();
	bool ok = false;
	const int pin = id.startsWith(QStringLiteral("dio")) ? id.mid(3).toInt(&ok) : -1;
	return ok ? pin : -1;
}
} // namespace

Adalm2000Logic_API::Adalm2000Logic_API(M2kLogicController *tool)
	: ApiObject()
	, m_tool(tool)
{}

bool Adalm2000Logic_API::running() const { return m_tool->ui()->engine()->isRunning(); }

void Adalm2000Logic_API::setRunning(bool on)
{
	if(on) {
		m_tool->ui()->run();
	} else {
		m_tool->ui()->stop();
	}
}

double Adalm2000Logic_API::sampleRate() const { return m_tool->m_sweep.sampleRate; }

void Adalm2000Logic_API::setSampleRate(double hz)
{
	sweep::Sweep s = m_tool->m_sweep;
	s.sampleRate = hz;
	m_tool->setSweep(s);
}

int Adalm2000Logic_API::bufferSize() const { return m_tool->m_sweep.bufferSize; }

void Adalm2000Logic_API::setBufferSize(int n)
{
	m_tool->setSweep(sweep::withBuffer(m_tool->m_sweep, n, m_tool->m_limits));
}

int Adalm2000Logic_API::plotWindow() const { return m_tool->m_sweep.plotWindow; }

void Adalm2000Logic_API::setPlotWindow(int n)
{
	const sweep::Sweep &s = m_tool->m_sweep;
	m_tool->setSweep(sweep::withWindow(s, s.bufferSize, n, m_tool->m_limits));
}

int Adalm2000Logic_API::delay() const { return m_tool->m_sweep.delay; }

void Adalm2000Logic_API::setDelay(int samples) { m_tool->setDelay(samples); }

bool Adalm2000Logic_API::showBuffers() const { return m_tool->m_buffersSwitch->onOffswitch()->isChecked(); }

void Adalm2000Logic_API::setShowBuffers(bool on) { m_tool->m_buffersSwitch->onOffswitch()->setChecked(on); }

QList<bool> Adalm2000Logic_API::enabled() const
{
	QList<bool> out;
	for(int pin = 0; pin < Controller::CHANNELS; ++pin) {
		out.append(m_tool->isChannelEnabled(pin));
	}
	return out;
}

void Adalm2000Logic_API::setEnabled(const QList<bool> &on)
{
	for(int pin = 0; pin < Controller::CHANNELS && pin < on.size(); ++pin) {
		m_tool->setChannelEnabled(pin, on[pin]);
	}
}

QStringList Adalm2000Logic_API::names() const
{
	QStringList out;
	for(int pin = 0; pin < Controller::CHANNELS; ++pin) {
		out.append(m_tool->channelName(pin));
	}
	return out;
}

void Adalm2000Logic_API::setNames(const QStringList &names)
{
	for(int pin = 0; pin < Controller::CHANNELS && pin < names.size(); ++pin) {
		m_tool->setChannelName(pin, names[pin]);
	}
}

bool Adalm2000Logic_API::triggerEnabled() const { return m_tool->m_trigEnable->isChecked(); }

void Adalm2000Logic_API::setTriggerEnabled(bool on) { m_tool->m_trigEnable->setChecked(on); }

QList<int> Adalm2000Logic_API::triggerConditions() const
{
	QList<int> out;
	for(int pin = 0; pin < Controller::CHANNELS; ++pin) {
		out.append(m_tool->triggerCondition(pin));
	}
	return out;
}

void Adalm2000Logic_API::setTriggerConditions(const QList<int> &conditions)
{
	for(int pin = 0; pin < Controller::CHANNELS && pin < conditions.size(); ++pin) {
		m_tool->setTriggerCondition(pin, conditions[pin]);
	}
}

int Adalm2000Logic_API::triggerLogic() const { return m_tool->m_modeCombo->combo()->currentIndex(); }

void Adalm2000Logic_API::setTriggerLogic(int idx) { m_tool->m_modeCombo->combo()->setCurrentIndex(idx); }

bool Adalm2000Logic_API::externalTrigger() const { return m_tool->m_extSwitch->onOffswitch()->isChecked(); }

void Adalm2000Logic_API::setExternalTrigger(bool on)
{
	if(m_tool->hasExternalTrigger()) {
		m_tool->m_extSwitch->onOffswitch()->setChecked(on);
	}
}

int Adalm2000Logic_API::externalSource() const { return m_tool->m_extSourceCombo->combo()->currentIndex(); }

void Adalm2000Logic_API::setExternalSource(int idx) { m_tool->m_extSourceCombo->combo()->setCurrentIndex(idx); }

int Adalm2000Logic_API::externalCondition() const { return m_tool->m_extConditionCombo->combo()->currentIndex(); }

void Adalm2000Logic_API::setExternalCondition(int idx) { m_tool->m_extConditionCombo->combo()->setCurrentIndex(idx); }

QStringList Adalm2000Logic_API::decoders() const
{
	QStringList out;
	for(const decoder::DecoderInstance &d : m_tool->decoders()->manager()->decoders()) {
		if(d.cfg.rootInput != decoder::RootInput::DigitalSamples || d.cfg.stack.empty()) {
			continue;
		}
		const decoder::DecoderStage &root = d.cfg.stack.front();
		QJsonObject channels;
		for(const decoder::ChannelMap &c : root.channels) {
			const int pin = pinOf(d.orderedRawKeys.value(c.bitIndex));
			if(pin >= 0) {
				channels.insert(QString::fromStdString(c.role), pin);
			}
		}
		QJsonObject options;
		for(const auto &[id, value] : root.options) {
			options.insert(QString::fromStdString(id), QString::fromStdString(value));
		}
		const QJsonObject entry{
			{"id", QString::fromStdString(root.decoderId)}, {"channels", channels}, {"options", options}};
		out.append(QString::fromUtf8(QJsonDocument(entry).toJson(QJsonDocument::Compact)));
	}
	return out;
}

void Adalm2000Logic_API::setDecoders(const QStringList &decoders)
{
	QStringList uids;
	for(const decoder::DecoderInstance &d : m_tool->decoders()->manager()->decoders()) {
		uids.append(d.uid);
	}
	for(const QString &uid : std::as_const(uids)) {
		removeDecoder(uid);
	}
	for(const QString &json : decoders) {
		const QJsonObject entry = QJsonDocument::fromJson(json.toUtf8()).object();
		const QString uid = addDecoder(entry.value("id").toString());
		if(uid.isEmpty()) {
			continue;
		}
		const QJsonObject channels = entry.value("channels").toObject();
		for(auto it = channels.begin(); it != channels.end(); ++it) {
			setDecoderChannel(uid, it.key(), it.value().toInt(-1));
		}
		const QJsonObject options = entry.value("options").toObject();
		for(auto it = options.begin(); it != options.end(); ++it) {
			setDecoderOption(uid, it.key(), it.value().toString());
		}
	}
}

void Adalm2000Logic_API::single() { m_tool->single(); }

QList<int> Adalm2000Logic_API::samples(int pin) const
{
	QList<int> out;
	if(pin < 0 || pin >= Controller::CHANNELS) {
		return out;
	}
	const QVector<quint8> bits = m_tool->ui()->store()->windowAs<QVector<quint8>>(
		m_tool->m_source->rawKey(M2kSource::Digital, pin), m_tool->m_sweep.plotWindow);
	for(quint8 b : bits) {
		out.append(b);
	}
	return out;
}

bool Adalm2000Logic_API::exportData(const QString &path) const { return m_tool->exportData(path); }

QString Adalm2000Logic_API::addDecoder(const QString &decoderId) { return m_tool->decoders()->addDecoder(decoderId); }

void Adalm2000Logic_API::removeDecoder(const QString &uid)
{
	if(m_tool->decoders()->editor(uid)) {
		m_tool->decoders()->removeDecoder(uid);
	}
}

bool Adalm2000Logic_API::setDecoderChannel(const QString &uid, const QString &role, int pin)
{
	decoder::DecoderEditor *editor = m_tool->decoders()->editor(uid);
	const QString key = pin >= 0 && pin < Controller::CHANNELS
		? m_tool->m_source->rawKey(M2kSource::Digital, pin).toString()
		: QString();
	return editor && editor->setChannel(role, key);
}

bool Adalm2000Logic_API::setDecoderOption(const QString &uid, const QString &option, const QString &value)
{
	decoder::DecoderEditor *editor = m_tool->decoders()->editor(uid);
	return editor && editor->setOption(option, value);
}

QVariantList Adalm2000Logic_API::annotations(const QString &uid, int stage) const
{
	QVariantList out;
	decoder::DecoderInstance *d = m_tool->decoders()->manager()->find(uid);
	if(!d || stage < 0 || stage >= d->outKeys.size()) {
		return out;
	}
	const auto anns = m_tool->ui()->store()->latestAs<QVector<scopy::acq::Annotation>>(d->outKeys[stage]);
	for(const scopy::acq::Annotation &a : anns.value_or(QVector<scopy::acq::Annotation>{})) {
		QVariantMap m{{"start", static_cast<double>(a.startSample)},
			      {"end", static_cast<double>(a.endSample)},
			      {"class", a.klass},
			      {"text", a.text}};
		if(a.value) {
			m["value"] = static_cast<double>(*a.value);
		}
		out.append(m);
	}
	return out;
}

#include "moc_adalm2000logic_api.cpp"
