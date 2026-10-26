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

#include "component/componentbuilder.h"
#include "iioutil/handles.h"
#include "component/component_export.h"

#include <QHash>
#include <QString>

namespace scopy::iio {
class IBackend;
class IAttrOps;
} // namespace scopy::iio

namespace scopy::component {
class Attribute;
}

namespace scopy::component::iio {

class IIOAttribute;
class IIOContext;
class IIODevice;

// Builds the IIO component tree of an open iio_context handle under an IIOContext. Written as
// many small single-purpose methods.
class COMPONENT_EXPORT IIOComponentBuilder : public ComponentBuilder
{
public:
	IIOComponentBuilder(scopy::iio::IBackend *backend, scopy::iio::ContextHandle handle)
		: m_backend(backend)
		, m_handle(handle)
	{}

	bool build(Context *ctx, ICmdExecutor *executor) override;

private:
	using AttrMap = QHash<QString, IIOAttribute *>;

	void buildContextAttributes(IIOContext *iioCtx, ICmdExecutor *executor);
	void buildDevices(IIOContext *iioCtx, ICmdExecutor *executor);
	void buildOneDevice(IIOContext *iioCtx, scopy::iio::DeviceHandle dh, ICmdExecutor *executor);

	AttrMap buildDeviceAttributes(IIODevice *dev, ICmdExecutor *executor);
	void buildDebugAttributes(IIODevice *dev, ICmdExecutor *executor);
	void buildChannels(IIODevice *dev, const AttrMap &devAttrs, ICmdExecutor *executor);
	void buildStreams(IIODevice *dev, ICmdExecutor *executor);
	void buildBufferAttributes(QObject *stream, scopy::iio::DeviceHandle dh, unsigned int bufferIndex,
				   ICmdExecutor *executor);
	void buildTrigger(IIODevice *dev, ICmdExecutor *executor);

	IIOAttribute *makeAttribute(QObject *parent, const QString &attrName, scopy::iio::AttrHandle handle,
				    ICmdExecutor *executor, bool forceReadOnly = false);

	void applyAvailable(const AttrMap &scope, const AttrMap *fallback = nullptr);

	static bool isReadOnlyAttr(const QString &attrName);

	scopy::iio::IBackend *m_backend;
	scopy::iio::ContextHandle m_handle;
	scopy::iio::IAttrOps *m_attrOps = nullptr;
};

} // namespace scopy::component::iio
