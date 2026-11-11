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
 */

#include "m2kcontext.h"
#include "m2kdigitalstreams.h"
#include "m2kinputstream.h"
#include "m2koutputstream.h"

#include <libm2k/contextbuilder.hpp>
#include <libm2k/m2k.hpp>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_M2K_CONTEXT, "M2kContext")

using namespace scopy::adalm2000;

M2kContext::~M2kContext()
{
	const QList<M2kInputStream *> inputs = findChildren<M2kInputStream *>();
	const QList<M2kDigitalInputStream *> digitalInputs = findChildren<M2kDigitalInputStream *>();
	const QList<M2kOutputStream *> outputs = findChildren<M2kOutputStream *>();
	const QList<M2kDigitalOutputStream *> digitalOutputs = findChildren<M2kDigitalOutputStream *>();

	for(M2kInputStream *s : inputs) {
		s->cancel();
	}
	for(M2kDigitalInputStream *s : digitalInputs) {
		s->cancel();
	}
	for(M2kOutputStream *s : outputs) {
		s->cancel();
	}
	for(M2kDigitalOutputStream *s : digitalOutputs) {
		s->cancel();
	}

	if(m_executor) {
		delete m_executor;
		m_executor = nullptr;
	}

	for(M2kInputStream *s : inputs) {
		s->detach();
	}
	for(M2kDigitalInputStream *s : digitalInputs) {
		s->detach();
	}
	for(M2kOutputStream *s : outputs) {
		s->detach();
	}
	for(M2kDigitalOutputStream *s : digitalOutputs) {
		s->detach();
	}

	if(!m_m2k) {
		return;
	}
	try {
		libm2k::context::contextClose(m_m2k, true);
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_CONTEXT) << "contextClose failed:" << e.what();
	}
	m_m2k = nullptr;
}
