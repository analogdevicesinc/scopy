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

#include "streamutils.h"

#include <component/attribute.h>
#include <component/channel.h>
#include <component/context.h>
#include <component/navigation.h>
#include <component/tags.h>

using namespace scopy;
using namespace scopy::component;

Attribute *scopy::dcdebugger::findSampleRate(QObject *stream, bool output)
{
	QObject *dev = stream ? stream->parent() : nullptr;
	for(QObject *scope : {stream, dev}) {
		if(!scope) {
			continue;
		}
		const QList<Attribute *> tagged = findByTag<Attribute>(scope, tags::SampleRate, false);
		if(!tagged.isEmpty()) {
			return tagged.first();
		}
	}
	if(dev) {
		for(Channel *ch : dev->findChildren<Channel *>(QString(), Qt::FindDirectChildrenOnly)) {
			const QList<Attribute *> tagged = findByTag<Attribute>(ch, tags::SampleRate, false);
			if(ch->isOutput() == output && !tagged.isEmpty()) {
				return tagged.first();
			}
		}
	}
	for(QObject *scope : {stream, dev}) {
		if(Attribute *a = attributeByName(scope, QStringLiteral("sampling_frequency"))) {
			return a;
		}
	}
	return nullptr;
}

double scopy::dcdebugger::sampleRateOf(const Attribute *attr)
{
	if(!attr) {
		return 0;
	}
	bool ok = false;
	const double v = attr->cachedValue().section(' ', 0, 0).toDouble(&ok);
	return ok ? v : 0;
}

Context *scopy::dcdebugger::contextOf(QObject *obj)
{
	for(QObject *o = obj; o; o = o->parent()) {
		if(auto *ctx = qobject_cast<Context *>(o)) {
			return ctx;
		}
	}
	return nullptr;
}

QString scopy::dcdebugger::scanElementName(QObject *stream, const QString &id, bool output)
{
	Channel *ch = channelById(stream ? stream->parent() : nullptr, id, output);
	if(!ch) {
		return id;
	}
	const QString label = ch->label().isEmpty() ? ch->name() : ch->label();
	return label.isEmpty() || label == id ? id : id + " (" + label + ")";
}
