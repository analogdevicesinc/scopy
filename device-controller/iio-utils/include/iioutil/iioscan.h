#pragma once

#include "iioutil/handles.h"

#include <QObject>
#include <QVector>

namespace scopy::iio {

class IScanOps;
class IBackend;

class IIOScan : public QObject {
	Q_OBJECT
public:
	explicit IIOScan(LibiioVersion version = LibiioVersion::V0, QObject *parent = nullptr);
	~IIOScan();

	QVector<ScanResult> scan(const QString &backends = QString());
	void scanAsync(const QString &backends = QString());

Q_SIGNALS:
	void scanCompleted(const QVector<scopy::iio::ScanResult> &results);

private:
	IBackend *m_backend = nullptr; // non-owning; loader keeps backends resident for the process
	IScanOps *m_scanOps = nullptr;
};

} // namespace scopy::iio
