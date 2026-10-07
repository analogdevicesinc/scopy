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

#include "m2kcomponentbuilder.h"
#include "m2kcontext.h"

#include "core/pooledcmdexecutor.h"

#include <libm2k/contextbuilder.hpp>
#include <libm2k/m2k.hpp>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_M2K_CTX_FACTORY, "M2kContextFactory")

using namespace scopy;
using namespace scopy::adalm2000;

component::Context *M2kContextFactory::create(const QString &uri, const int maxThreads)
{
	libm2k::context::M2k *m2k = nullptr;
	try {
		m2k = libm2k::context::m2kOpen(uri.toStdString().c_str());
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

	auto *executor = new PooledCmdExecutor(maxThreads, ctx);
	ctx->setExecutor(executor);

	M2kComponentBuilder builder;
	if(!builder.build(ctx, executor)) {
		qWarning(CAT_M2K_CTX_FACTORY) << "component build failed for" << uri;
		delete ctx;
		return nullptr;
	}
	return ctx;
}
