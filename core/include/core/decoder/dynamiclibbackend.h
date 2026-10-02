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

#pragma once

#include "scopy-core_export.h"
#include "idecoderbackend.h"

#include <QString>
#include <memory>

class QLibrary;

namespace scopy {
namespace decoder {

class DecoderLogger;

// Backend that loads a vendor decoder shared library exposing the C ABI
// in decoder_c_api.h.
class SCOPY_CORE_EXPORT DynamicLibBackend : public IDecoderBackend
{
public:
	explicit DynamicLibBackend(const QString &libraryPath);
	~DynamicLibBackend() override;

	bool decode(const DecoderConfig &cfg, const uint8_t *data, std::size_t nSamples,
		    std::vector<AnnotationC> &out) override;

	// Backed by optional scopy_decoder_accepts_annotations /
	// scopy_decoder_decode_ann C symbols; falls back to unsupported.
	bool acceptsAnnotationInput(const DecoderConfig &cfg) const override;
	bool decodeAnnotations(const DecoderConfig &cfg, const std::vector<AnnotationC> &in,
			       std::vector<AnnotationC> &out) override;

	std::string lastError() const override { return m_lastError; }

	void setLogger(DecoderLogger *lg) { m_logger = lg; }

private:
	// Lazy-load m_lib and resolve symbols. True iff scopy_decoder_decode resolved.
	bool ensureLoaded() const;

	const char *m_kDynLibId;
	QString m_libraryPath;
	mutable std::unique_ptr<QLibrary> m_lib;
	mutable bool m_loadAttempted{false};
	mutable void *m_symDecode{nullptr};
	mutable void *m_symAcceptsAnn{nullptr};
	mutable void *m_symDecodeAnn{nullptr};
	mutable std::string m_lastError;
	DecoderLogger *m_logger{nullptr};
};

} // namespace decoder
} // namespace scopy
