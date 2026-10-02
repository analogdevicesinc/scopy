/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include "decoder/decoderlogger.h"

#include "acq_engine/acquisitionengine.h"
#include "acq_engine/acquisitionerror.h"

namespace scopy {
namespace decoder {

DecoderLogger::DecoderLogger(QObject *parent)
	: QObject(parent)
{}

DecoderLogger::~DecoderLogger() = default;

void DecoderLogger::setEngine(scopy::acq::AcquisitionEngine *e) { m_engine = e; }

void DecoderLogger::log(LogLevel lvl, const QString &id, const QString &msg)
{
	if(lvl < m_minLevel.load(std::memory_order_relaxed))
		return;

	Q_EMIT messageLogged(static_cast<int>(lvl), id, msg);

	if(!m_forward.load(std::memory_order_relaxed) || !m_engine)
		return;

	scopy::acq::AcquisitionError::Severity sev;
	switch(lvl) {
	case LogLevel::Info:
		sev = scopy::acq::AcquisitionError::Severity::Info;
		break;
	case LogLevel::Warning:
		sev = scopy::acq::AcquisitionError::Severity::Warning;
		break;
	case LogLevel::Critical:
		sev = scopy::acq::AcquisitionError::Severity::Critical;
		break;
	}
	Q_EMIT m_engine->error(static_cast<int>(sev), id, msg);
}

} // namespace decoder
} // namespace scopy

#include "moc_decoderlogger.cpp"
