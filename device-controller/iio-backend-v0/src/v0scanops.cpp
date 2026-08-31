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

#include "v0scanops.h"
#include <iio.h>

namespace scopy::iio {

QVector<ScanResult> V0ScanOps::scan(const QString &backends)
{
	QVector<ScanResult> results;
	const char *back = backends.isEmpty() ? nullptr : backends.toUtf8().constData();

	iio_scan_block *blk = iio_create_scan_block(back, 0);
	if(!blk) {
		return results;
	}

	ssize_t count = iio_scan_block_scan(blk);
	if(count > 0) {
		for(ssize_t i = 0; i < count; ++i) {
			iio_context_info *info = iio_scan_block_get_info(blk, static_cast<unsigned int>(i));
			if(!info) {
				continue;
			}
			ScanResult r;
			const char *uri = iio_context_info_get_uri(info);
			const char *desc = iio_context_info_get_description(info);
			r.uri = uri ? QString::fromUtf8(uri) : QString();
			r.description = desc ? QString::fromUtf8(desc) : QString();
			results.append(r);
		}
	}

	iio_scan_block_destroy(blk);
	return results;
}

} // namespace scopy::iio
