#pragma once

#include "iioutil/handles.h"

#include <QObject>
#include <QVector>

namespace scopy::iio {

class IScanOps;

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
	IScanOps *m_scanOps;
};

} // namespace scopy::iio
