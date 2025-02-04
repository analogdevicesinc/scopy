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

#include "iiocpp/iioscan.h"
#include <QApplication>

using namespace scopy;

IIOScan *IIOScan::pinstance_{nullptr};

IIOScan::IIOScan(QObject *parent)
	: QObject(parent)
{
}

IIOScan::~IIOScan() = default;

IIOScan *IIOScan::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new IIOScan(QApplication::instance());
	}
	return pinstance_;
}

IIOResult<struct iio_scan *> IIOScan::scan(const iio_context_params *params, const char *backends)
{
	struct iio_scan *scan = iio_scan(params, backends);
	int err = iio_err(scan);

	if(err) {
		return IIOResult<struct iio_scan *>(err);
	}
	return IIOResult<struct iio_scan *>(scan);
}

void IIOScan::destroy(struct iio_scan *ctx) { iio_scan_destroy(ctx); }

size_t IIOScan::get_results_count(const struct iio_scan *ctx) { return iio_scan_get_results_count(ctx); }

const char *IIOScan::get_description(const struct iio_scan *ctx, size_t idx)
{
	return iio_scan_get_description(ctx, idx);
}

const char *IIOScan::get_uri(const struct iio_scan *ctx, size_t idx) { return iio_scan_get_uri(ctx, idx); }

unsigned int IIOScan::get_builtin_backends_count(void) { return iio_get_builtin_backends_count(); }

const char *IIOScan::get_builtin_backend(unsigned int idx) { return iio_get_builtin_backend(idx); }

bool IIOScan::has_backend(const iio_context_params *params, const char *backend)
{
	return iio_has_backend(params, backend);
}
