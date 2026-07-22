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
