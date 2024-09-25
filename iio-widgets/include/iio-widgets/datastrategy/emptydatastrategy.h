#ifndef EMPTYDATASTRATEGY_H
#define EMPTYDATASTRATEGY_H

#include <QObject>
#include "datastrategy/datastrategyinterface.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT EmptyDataStrategy : public QObject, public DataStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::DataStrategyInterface)
public:
	explicit EmptyDataStrategy(QObject *parent = nullptr);

	QString data() override;
	QString optionalData() override;

public Q_SLOTS:
	int write(QString data) override;
	QPair<QString, QString> read() override;
	void writeAsync(QString data) override;
	void readAsync() override;

Q_SIGNALS:
	void sendData(QString data, QString dataOptions) override;
	void aboutToWrite(QString oldData, QString newData) override;
	void emitStatus(QDateTime timestamp, QString oldData, QString newData, int returnCode, bool isReadOp) override;

private:
	QString m_data;
	QString m_previousData;
};
} // namespace scopy

#endif // EMPTYDATASTRATEGY_H
