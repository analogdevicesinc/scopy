/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef IIOCONTEXT_H
#define IIOCONTEXT_H

#include "iioresult.h"
#include "scopy-iioutil_export.h"
#include <iio/iio.h>
#include <QObject>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IIOContext : public QObject
{
	Q_OBJECT
protected:
	IIOContext(QObject *parent = nullptr);
	~IIOContext();

public:
	IIOContext(const IIOContext &) = delete;
	IIOContext &operator=(const IIOContext &) = delete;

	static IIOContext *GetInstance();

	static IIOResult<iio_context *> create_context(const iio_context_params *params, const char *uri);
	static void destroy(iio_context *ctx);
	static unsigned int get_version_major(const iio_context *ctx);
	static unsigned int get_version_minor(const iio_context *ctx);
	static const char *get_version_tag(const iio_context *ctx);
	static IIOResult<char *> get_xml(const iio_context *ctx);
	static const char *get_name(const iio_context *ctx);
	static const char *get_description(const iio_context *ctx);
	static unsigned int get_attrs_count(const iio_context *ctx);
	static IIOResult<const iio_attr *> get_attr(const iio_context *ctx, unsigned int index);
	static IIOResult<const iio_attr *> find_attr(const iio_context *ctx, const char *name);
	static unsigned int get_devices_count(const iio_context *ctx);
	static IIOResult<iio_device *> get_device(const iio_context *ctx, unsigned int index);
	static IIOResult<iio_device *> find_device(const iio_context *ctx, const char *name);
	static int set_timeout(iio_context *ctx, unsigned int timeout_ms);
	static const iio_context_params *get_params(const iio_context *ctx);
	static void set_data(iio_context *ctx, void *data);
	static void *get_data(const iio_context *ctx);

private:
	static IIOContext *pinstance_;
};

} // namespace scopy

#endif // IIOCONTEXT_H
