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

#include "multiacq_api.h"

#include "iiosource.h"
#include "multiacqtool.h"
#include "timeplot.h"

using namespace scopy;
using namespace scopy::multiacq;

MultiAcq_API::MultiAcq_API(MultiAcqTool *tool, QObject *parent)
	: ApiObject(parent)
	, m_tool(tool)
{}

IioSource *MultiAcq_API::source(const QString &uri) const { return m_tool ? m_tool->sourceByUri(uri) : nullptr; }

TimePlot *MultiAcq_API::plot(int uuid) const
{
	return m_tool && uuid >= 0 ? m_tool->plotByUuid(static_cast<quint32>(uuid)) : nullptr;
}

TimeCurve *MultiAcq_API::curve(int uuid, const QString &uri, const QString &channel) const
{
	TimePlot *p = plot(uuid);
	IioSource *src = source(uri);
	return p && src ? m_tool->curve(p, acq::DataKey::raw(src->id(), channel)) : nullptr;
}

// ---- run control --------------------------------------------------------------

void MultiAcq_API::run()
{
	if(m_tool) {
		m_tool->run();
	}
}

void MultiAcq_API::stop()
{
	if(m_tool) {
		m_tool->stop();
	}
}

void MultiAcq_API::single()
{
	if(m_tool) {
		m_tool->single();
	}
}

bool MultiAcq_API::isRunning() { return m_tool && m_tool->isRunning(); }

void MultiAcq_API::clear()
{
	if(m_tool) {
		m_tool->clearData();
	}
}

// ---- devices ------------------------------------------------------------------

QVariantList MultiAcq_API::getDevices()
{
	QVariantList out;
	if(!m_tool) {
		return out;
	}
	for(IioSource *src : m_tool->sources()) {
		QStringList iioDevices;
		for(const IioSource::DeviceDesc &dev : src->devices()) {
			iioDevices.append(dev.name);
		}
		out.append(QVariantMap{{"uri", src->uri()},
				       {"name", src->displayName()},
				       {"model", src->model()},
				       {"enabled", src->isEnabled()},
				       {"online", src->isOnline()},
				       {"readIntervalMs", src->readIntervalMs()},
				       {"iioDevices", iioDevices}});
	}
	return out;
}

QString MultiAcq_API::addDevice(const QString &uri)
{
	return m_tool ? m_tool->addDevice(uri) : QStringLiteral("The tool is not available.");
}

bool MultiAcq_API::removeDevice(const QString &uri)
{
	IioSource *src = source(uri);
	if(!src) {
		return false;
	}
	m_tool->removeDevice(src);
	return true;
}

bool MultiAcq_API::setDeviceEnabled(const QString &uri, bool en)
{
	IioSource *src = source(uri);
	if(!src) {
		return false;
	}
	src->setEnabled(en);
	return true;
}

bool MultiAcq_API::setReadInterval(const QString &uri, int ms)
{
	IioSource *src = source(uri);
	if(!src || ms < 1) {
		return false;
	}
	src->setReadIntervalMs(ms);
	return true;
}

QVariantList MultiAcq_API::getIioDevices(const QString &uri)
{
	QVariantList out;
	IioSource *src = source(uri);
	if(!src) {
		return out;
	}
	for(const IioSource::DeviceDesc &dev : src->devices()) {
		out.append(QVariantMap{{"name", dev.name},
				       {"bufferable", dev.bufferable},
				       {"hasRate", dev.hasRate},
				       {"rateOverride", src->rateOverride(dev.name)}});
	}
	return out;
}

bool MultiAcq_API::setRateOverride(const QString &uri, const QString &iioDevice, double hz)
{
	IioSource *src = source(uri);
	if(!src || hz < 0) {
		return false;
	}
	for(const IioSource::DeviceDesc &dev : src->devices()) {
		// Only where the settings page offers it: a published rate wins anyway.
		if(dev.name == iioDevice && dev.bufferable && !dev.hasRate) {
			src->setRateOverride(iioDevice, hz);
			return true;
		}
	}
	return false;
}

// ---- device channels ----------------------------------------------------------

QVariantList MultiAcq_API::getChannels(const QString &uri)
{
	QVariantList out;
	IioSource *src = source(uri);
	if(!src) {
		return out;
	}
	for(const IioSource::DeviceDesc &dev : src->devices()) {
		for(const IioSource::ChannelDesc &ch : dev.channels) {
			const IioSource::ChannelConfig cfg = src->config(ch.channelId);
			out.append(QVariantMap{{"id", ch.channelId},
					       {"enabled", src->isChannelEnabled(ch.channelId)},
					       {"mode", IioSource::modeName(cfg.mode)},
					       {"attribute", cfg.attribute},
					       {"scaled", cfg.scaled},
					       {"attributes", ch.attributes},
					       {"bufferable", ch.bufferable},
					       {"scalable", IioSource::canScale(ch, cfg)},
					       {"description", IioSource::describe(ch, cfg)}});
		}
	}
	return out;
}

bool MultiAcq_API::setChannelEnabled(const QString &uri, const QString &channel, bool en)
{
	IioSource *src = source(uri);
	if(!src || !src->channelDesc(channel)) {
		return false;
	}
	src->setChannelEnabled(channel, en);
	return true;
}

bool MultiAcq_API::setChannelMode(const QString &uri, const QString &channel, const QString &mode)
{
	IioSource *src = source(uri);
	const IioSource::ChannelDesc *ch = src ? src->channelDesc(channel) : nullptr;
	IioSource::ChannelConfig cfg = ch ? src->config(channel) : IioSource::ChannelConfig();
	if(!ch || !IioSource::modeFromName(mode, cfg.mode) || !IioSource::supports(*ch, cfg)) {
		return false;
	}
	src->setConfig(channel, cfg);
	return true;
}

bool MultiAcq_API::setChannelAttribute(const QString &uri, const QString &channel, const QString &attribute)
{
	IioSource *src = source(uri);
	const IioSource::ChannelDesc *ch = src ? src->channelDesc(channel) : nullptr;
	if(!ch || !ch->attributes.contains(attribute)) {
		return false;
	}
	IioSource::ChannelConfig cfg = src->config(channel);
	cfg.attribute = attribute;
	src->setConfig(channel, cfg);
	return true;
}

bool MultiAcq_API::setChannelScaled(const QString &uri, const QString &channel, bool scaled)
{
	IioSource *src = source(uri);
	if(!src || !src->channelDesc(channel)) {
		return false;
	}
	IioSource::ChannelConfig cfg = src->config(channel);
	cfg.scaled = scaled;
	src->setConfig(channel, cfg);
	return true;
}

// ---- plots --------------------------------------------------------------------

QVariantList MultiAcq_API::getPlots()
{
	QVariantList out;
	if(!m_tool) {
		return out;
	}
	const QList<IioSource *> sources = m_tool->sources();
	for(TimePlot *p : m_tool->plots()) {
		QVariantList curves;
		for(TimeCurve *c : p->curves()) {
			QString uri;
			for(IioSource *src : sources) {
				if(src->id() == c->yKey().sourceId()) {
					uri = src->uri();
				}
			}
			curves.append(QVariantMap{{"uri", uri},
						  {"channel", c->yKey().channelId()},
						  {"name", c->name()},
						  {"color", c->color().name()},
						  {"visible", c->isVisible()}});
		}
		out.append(QVariantMap{{"uuid", static_cast<int>(p->uuid())},
				       {"name", p->name()},
				       {"legend", p->showLegend()},
				       {"timeLabels", p->relative() ? "relative" : "timeofday"},
				       {"timeFormat", p->timeFormat()},
				       {"span", p->span()},
				       {"history", p->history()},
				       {"live", p->live()},
				       {"autoscale", p->autoscale()},
				       {"yMin", p->yMin()},
				       {"yMax", p->yMax()},
				       {"curves", curves}});
	}
	return out;
}

int MultiAcq_API::addPlot(const QString &name)
{
	TimePlot *p = m_tool ? m_tool->addPlot(name) : nullptr;
	return p ? static_cast<int>(p->uuid()) : -1;
}

bool MultiAcq_API::removePlot(int uuid)
{
	TimePlot *p = plot(uuid);
	if(!p) {
		return false;
	}
	m_tool->removePlot(p);
	return true;
}

bool MultiAcq_API::setPlotName(int uuid, const QString &name)
{
	TimePlot *p = plot(uuid);
	if(!p || name.trimmed().isEmpty()) {
		return false;
	}
	p->setName(name);
	return true;
}

bool MultiAcq_API::setPlotLegend(int uuid, bool on)
{
	TimePlot *p = plot(uuid);
	if(!p) {
		return false;
	}
	p->setShowLegend(on);
	return true;
}

bool MultiAcq_API::setTimeLabels(int uuid, const QString &labels)
{
	TimePlot *p = plot(uuid);
	const bool relative = labels.compare("relative", Qt::CaseInsensitive) == 0;
	if(!p || (!relative && labels.compare("timeofday", Qt::CaseInsensitive) != 0)) {
		return false;
	}
	p->setRelative(relative);
	return true;
}

bool MultiAcq_API::setTimeFormat(int uuid, const QString &format)
{
	TimePlot *p = plot(uuid);
	if(!p) {
		return false;
	}
	p->setTimeFormat(format.trimmed());
	return true;
}

bool MultiAcq_API::setSpan(int uuid, double seconds)
{
	TimePlot *p = plot(uuid);
	if(!p || seconds <= 0) {
		return false;
	}
	p->setSpan(seconds);
	return true;
}

bool MultiAcq_API::setHistory(int uuid, double seconds)
{
	TimePlot *p = plot(uuid);
	if(!p || seconds <= 0) {
		return false;
	}
	p->setHistory(seconds);
	return true;
}

bool MultiAcq_API::setLive(int uuid, bool on)
{
	TimePlot *p = plot(uuid);
	if(!p) {
		return false;
	}
	p->setLive(on);
	return true;
}

bool MultiAcq_API::setAutoscale(int uuid, bool on)
{
	TimePlot *p = plot(uuid);
	if(!p) {
		return false;
	}
	p->setAutoscale(on);
	return true;
}

bool MultiAcq_API::setYRange(int uuid, double min, double max)
{
	TimePlot *p = plot(uuid);
	if(!p || min >= max) {
		return false;
	}
	p->setAutoscale(false);
	p->setYRange(min, max);
	return true;
}

// ---- curves -------------------------------------------------------------------

bool MultiAcq_API::addCurve(int uuid, const QString &uri, const QString &channel)
{
	TimePlot *p = plot(uuid);
	IioSource *src = source(uri);
	if(!p || !src || !src->channelDesc(channel)) {
		return false;
	}
	// A curve of a channel that is not read would stay empty.
	src->setChannelEnabled(channel, true);
	return m_tool->addChannel(p, acq::DataKey::raw(src->id(), channel)) != nullptr;
}

bool MultiAcq_API::removeCurve(int uuid, const QString &uri, const QString &channel)
{
	TimeCurve *c = curve(uuid, uri, channel);
	if(!c) {
		return false;
	}
	m_tool->removeChannel(plot(uuid), c);
	return true;
}

bool MultiAcq_API::setCurveVisible(int uuid, const QString &uri, const QString &channel, bool visible)
{
	TimeCurve *c = curve(uuid, uri, channel);
	if(!c) {
		return false;
	}
	m_tool->setCurveVisible(plot(uuid), c, visible);
	return true;
}

// ---- setups -------------------------------------------------------------------

QStringList MultiAcq_API::getLoadReport() { return m_tool ? m_tool->loadReport() : QStringList(); }

#include "moc_multiacq_api.cpp"
