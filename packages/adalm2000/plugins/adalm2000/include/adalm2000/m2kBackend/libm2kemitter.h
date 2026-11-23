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

#ifndef LIBM2KEMITTER_H
#define LIBM2KEMITTER_H

#include "scopy-adalm2000_export.h"

#include <core/codeemitter.h>

#include <memory>

namespace scopy::adalm2000 {

// Code for the libm2k calls recorded by M2K_CALL (m2ktrace.h), in C++ (the libm2k API) or Python
// (the `libm2k` module). Pure text: the script opens its own context (m2kOpen) on the logged uri.
// Each libm2k object is one variable, obtained once from the context (ain, aout, ps, dig, trig);
// enums are emitted by name, values read are printed, and data that was not logged (pushed
// samples) becomes a zero-filled buffer of the logged size, with a comment.
class SCOPY_ADALM2000_EXPORT Libm2kEmitter : public CodeEmitter
{
public:
	static constexpr const char *LIBRARY = "libm2k";

	explicit Libm2kEmitter(const QString &language);
	~Libm2kEmitter() override;

	// The C++ and Python emitters.
	static QList<std::shared_ptr<CodeEmitter>> create();

	QString library() const override;
	QString language() const override;
	QString description() const override;
	QStringList headers(EmitContext &ctx) const override;
	QString prologue(EmitContext &ctx) const override;
	QString emitCall(const CallRecord &call, EmitContext &ctx) const override;
	QString epilogue(EmitContext &ctx) const override;

private:
	QString m_language;
};

} // namespace scopy::adalm2000

#endif // LIBM2KEMITTER_H
