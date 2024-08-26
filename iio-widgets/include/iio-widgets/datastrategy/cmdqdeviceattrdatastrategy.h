#ifndef CMDQDEVICEATTRDATASTRATEGY_H
#define CMDQDEVICEATTRDATASTRATEGY_H

#include <QWidget>
#include <iioutil/commandqueue.h>
#include "iiowidgetdata.h"
#include "datastrategyinterface.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT CmdQDeviceAttrDataStrategy : public QObject, public DataStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::DataStrategyInterface)
public:
	explicit CmdQDeviceAttrDataStrategy(IIOWidgetFactoryRecipe recipe, QObject *parent = nullptr);
	CmdQDeviceAttrDataStrategy();

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

private Q_SLOTS:
	void attributeReadFinished(Command *cmd);
	void optionalAttrReadFinished(Command *cmd);

private:
	CommandQueue *m_cmdQueue;
	QString m_dataRead;
	QString m_optionalDataRead;
};
} // namespace scopy

#endif // CMDQDEVICEATTRDATASTRATEGY_H
