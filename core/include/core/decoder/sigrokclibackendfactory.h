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
#include "annotationsymbols.h"
#include "idecoderbackendfactory.h"
#include "sigrokclibackend.h"

namespace scopy {
namespace decoder {

class SigrokCliCatalog;
class DecoderLogger;

// Factory for SigrokCliBackend. Threads a shared catalog and annotation
// extractor registry (built-in uart/spi/i2c) to every created backend.
// Additional extractors can be registered before calling create().
class SCOPY_CORE_EXPORT SigrokCliBackendFactory : public IDecoderBackendFactory
{
public:
	explicit SigrokCliBackendFactory(SigrokCliCatalog *catalog)
		: m_catalog(catalog)
	{
		m_extractors.registerBuiltins();
	}

	void setLogger(DecoderLogger *lg) { m_logger = lg; }

	AnnotationExtractorRegistry &extractorRegistry() { return m_extractors; }
	const AnnotationExtractorRegistry &extractorRegistry() const { return m_extractors; }

	std::unique_ptr<IDecoderBackend> create() override
	{
		auto b = std::make_unique<SigrokCliBackend>(m_catalog);
		b->setLogger(m_logger);
		b->setExtractorRegistry(&m_extractors);
		return b;
	}

private:
	SigrokCliCatalog *m_catalog{nullptr};
	DecoderLogger *m_logger{nullptr};
	AnnotationExtractorRegistry m_extractors;
};

} // namespace decoder
} // namespace scopy
