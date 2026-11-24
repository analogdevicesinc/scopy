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

#ifndef STREAMUTILS_H
#define STREAMUTILS_H

#include <QString>

class QObject;

namespace scopy::component {
class Attribute;
class Context;
} // namespace scopy::component

namespace scopy::dcdebugger {

// Sample-rate attribute of a stream: tagged (tags::SampleRate) on the stream, its device or one of
// the device's channels of the stream's direction; the conventional name as fallback.
component::Attribute *findSampleRate(QObject *stream, bool output);

// The sample rate held by `attr` (its cached value), 0 when unknown.
double sampleRateOf(const component::Attribute *attr);

component::Context *contextOf(QObject *obj);

// "voltage0" or "voltage0 (label)" for a member channel id of `stream`.
QString streamChannelName(QObject *stream, const QString &id, bool output);

} // namespace scopy::dcdebugger

#endif // STREAMUTILS_H
