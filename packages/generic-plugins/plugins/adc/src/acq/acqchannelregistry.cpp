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

#include "acqchannelregistry.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_ACQ_CHANNELREGISTRY, "AcqChannelRegistry")

using namespace scopy;
using namespace scopy::adc;

AcqChannelRegistry &AcqChannelRegistry::instance()
{
	// Function-local static: the registrations happen at static-init time from four
	// translation units, so a namespace-scope instance could be constructed after the
	// first one of them ran. This form is initialised on first use by definition.
	static AcqChannelRegistry s_instance;
	return s_instance;
}

void AcqChannelRegistry::registerKind(scopy::acq::ReprKind kind, Factory f)
{
	if(!f) {
		return;
	}
	m_kinds.insert(static_cast<int>(kind), std::move(f));
}

bool AcqChannelRegistry::isRegistered(scopy::acq::ReprKind kind) const
{
	return m_kinds.contains(static_cast<int>(kind));
}

AcqChannel *AcqChannelRegistry::create(scopy::acq::ReprKind kind, const AcqChannel::Args &args) const
{
	const auto it = m_kinds.constFind(static_cast<int>(kind));
	if(it == m_kinds.constEnd()) {
		// Hidden lands here by design: it is registered by nothing, because "do not
		// draw this" is the absence of a channel rather than a kind of one.
		if(kind != scopy::acq::ReprKind::Hidden) {
			qWarning(CAT_ACQ_CHANNELREGISTRY) << "no channel kind registered for"
							  << scopy::acq::reprKindName(kind);
		}
		return nullptr;
	}

	// No check that this kind can read the key's sample type. A mismatch draws nothing
	// rather than being refused — see registerKind.
	return (*it)(args);
}
