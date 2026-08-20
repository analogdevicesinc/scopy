#include "iioutil/iioscan.h"
#include "iioutil/iscanops.h"
#include "iioutil/ibackend.h"
#include "iioutil/iiobackendloader.h"
#include <QThreadPool>

namespace scopy::iio {

IIOScan::IIOScan(LibiioVersion version, QObject *parent)
    : QObject(parent)
{
    // The loader keeps the backend resident for the whole process; non-owning pointers.
    m_backend = IIOBackendLoader::instance()->backend(version);
    m_scanOps = m_backend ? m_backend->scanOps() : nullptr;
}

IIOScan::~IIOScan() {}

QVector<ScanResult> IIOScan::scan(const QString &backends) { return m_scanOps->scan(backends); }

void IIOScan::scanAsync(const QString &backends)
{
    QThreadPool::globalInstance()->start([this, backends]() {
		QVector<ScanResult> results = m_scanOps->scan(backends);
		Q_EMIT scanCompleted(results);
    });
}

} // namespace scopy::iio
