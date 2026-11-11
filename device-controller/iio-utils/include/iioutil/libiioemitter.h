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

#pragma once

#include "iioutil/controller_iioutils_export.h"
#include "iioutil/handles.h"

#include "core/codeemitter.h"

#include <QList>

#include <memory>

namespace scopy::iio {

// Code for the libiio calls recorded by iio-backend-v0 / iio-backend-v1 (their C names), in C
// (the libiio API itself) or Python (the `iio` module, pylibiio), for the v0 or the v1 API. Pure
// text: no libiio linkage, so both versions build with either SCOPY_LIBIIO_VERSION.
// Objects are addressed by name: a device / channel / attribute is looked up once, when its handle
// is first used, from the record's identifying args (v0) or the command's origin scope (v1 attribute
// records carry only the attribute name). What cannot be resolved becomes a comment.
class CONTROLLER_IIOUTILS_EXPORT LibiioEmitter : public CodeEmitter
{
public:
	static constexpr const char *LIBRARY = "libiio";

	LibiioEmitter(LibiioVersion version, const QString &language);
	~LibiioEmitter() override;

	// The C and Python emitters of one API version.
	static QList<std::shared_ptr<CodeEmitter>> create(LibiioVersion version);

	LibiioVersion version() const { return m_version; }

	QString library() const override;
	QString language() const override;
	QString description() const override;
	QStringList headers(EmitContext &ctx) const override;
	QString prologue(EmitContext &ctx) const override;
	QString emitCall(const CallRecord &call, EmitContext &ctx) const override;
	QString epilogue(EmitContext &ctx) const override;

private:
	LibiioVersion m_version;
	QString m_language;
};

} // namespace scopy::iio
