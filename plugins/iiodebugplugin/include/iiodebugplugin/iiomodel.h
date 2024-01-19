#ifndef SCOPY_IIOMODEL_H
#define SCOPY_IIOMODEL_H

#include <QObject>
#include <QStandardItemModel>
#include <iio.h>

namespace scopy::iiodebugplugin {
class IIOModel : public QObject
{
	Q_OBJECT
public:
	explicit IIOModel(struct iio_context *context, QObject *parent = nullptr);

	QStandardItemModel *getModel();
	QStringList getEntries();

private:
	void demoSetup();
	void iioTreeSetup();

	QStandardItemModel *m_model;
	struct iio_context *m_ctx;
	QStringList m_entries;
};
} // namespace scopy::iiodebugplugin

#endif // SCOPY_IIOMODEL_H
