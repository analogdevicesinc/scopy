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

#include "m2kcontextfactory.h"

#include "m2kcalibration.h"
#include "m2kcomponentbuilder.h"
#include "m2kcontext.h"

#include <component/backends/iio/iioconnection.h>
#include <component/backends/iio/iioprovider.h>
#include <iioutil/ibackend.h>
#include <iioutil/icontextops.h>

#include <libm2k/contextbuilder.hpp>
#include <libm2k/m2k.hpp>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_M2K_CTX_FACTORY, "M2kContextFactory")

using namespace scopy;
using namespace scopy::adalm2000;

component::ConnectionKind M2kContextFactory::connectionKind() const { return component::iio::IIOProvider::KIND; }

bool M2kContextFactory::compatible(const component::Connection &c) const
{
	const auto *conn = c.as<component::iio::IIOConnection>();
	if(!conn || !conn->handle().ptr) {
		return false;
	}
	scopy::iio::IContextOps *ops = conn->backend()->contextOps();
	for(const char *name : {"m2k-adc", "m2k-dac-a", "m2k-dac-b"}) {
		if(!ops->findDevice(conn->handle(), name).ptr) {
			return false;
		}
	}
	return true;
}

// The m2k view shares the connection's iio_context and executor.
component::Context *M2kContextFactory::create(component::Connection &c)
{
	auto *conn = c.as<component::iio::IIOConnection>();
	if(!conn || !conn->handle().ptr) {
		return nullptr;
	}
	const QString uri = c.uri();
	libm2k::context::M2k *m2k = nullptr;
	try {
		m2k = libm2k::context::m2kOpen(static_cast<struct iio_context *>(conn->handle().ptr),
					       uri.toStdString().c_str());
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_CTX_FACTORY) << "m2kOpen failed for" << uri << ":" << e.what();
		return nullptr;
	} catch(...) {
		qWarning(CAT_M2K_CTX_FACTORY) << "m2kOpen threw an unknown exception for" << uri;
		return nullptr;
	}
	if(!m2k) {
		qWarning(CAT_M2K_CTX_FACTORY) << "m2kOpen returned null for" << uri;
		return nullptr;
	}

	auto *ctx = new M2kContext;
	ctx->setUri(uri);
	ctx->setM2k(m2k);

	ICmdExecutor *executor = c.executor();

	new M2kCalibration(
		m2k, [m2k]() { return m2k->isCalibrated(); }, [m2k]() { return m2k->calibrate(); },
		[m2k]() { return m2k->resetCalibration(); }, executor, ctx);

	M2kComponentBuilder builder;
	if(!builder.build(ctx, executor)) {
		qWarning(CAT_M2K_CTX_FACTORY) << "component build failed for" << uri;
		delete ctx;
		return nullptr;
	}
	return ctx;
}
