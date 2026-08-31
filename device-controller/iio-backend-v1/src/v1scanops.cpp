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

#include "v1scanops.h"
#include <iio/iio.h>

namespace scopy::iio {

QVector<ScanResult> V1ScanOps::scan(const QString &backends)
{
	QVector<ScanResult> results;
	const char *back = backends.isEmpty() ? nullptr : backends.toUtf8().constData();

	struct iio_scan *scan = iio_scan(nullptr, back);
	if(iio_err(scan)) {
		return results;
	}

	ssize_t count = iio_scan_get_results_count(scan);
	if(count > 0) {
		for(ssize_t i = 0; i < count; ++i) {
			ScanResult r;
			const char *uri = iio_scan_get_uri(scan, i);
			const char *desc = iio_scan_get_description(scan, i);
			r.uri = uri ? QString::fromUtf8(uri) : QString();
			r.description = desc ? QString::fromUtf8(desc) : QString();
			results.append(r);
		}
	}

	iio_scan_destroy(scan);

	return results;
}

} // namespace scopy::iio
