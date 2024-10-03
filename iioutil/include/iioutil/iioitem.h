#ifndef IIOITEM_H
#define IIOITEM_H

#include "scopy-iioutil_export.h"
#include <QStandardItem>
#include <QList>
#include <iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IIOItem : public QStandardItem
{
public:
	// User defined types should be > 1000, the rest are Qt reserved
	// In order to ensure that the user cannot enter a type and a
	// different iio struct, we will separate the types in 3 enums
	enum IIOTypeCtx
	{
		CONTEXT = 1001,
		CONTEXT_ATTR = 1002,
	};

	enum IIOTypeDev
	{
		DEVICE = 1003,
		DEVICE_ATTR = 1004,
	};

	enum IIOTypeChnl
	{
		CHANNEL = 1005,
		CHANNEL_ATTR = 1006,
	};

	IIOItem(QString name, QString id, IIOTypeCtx type, iio_context *ctx, IIOItem *parent = nullptr);
	IIOItem(QString name, QString id, IIOTypeDev type, iio_device *dev, IIOItem *parent = nullptr);
	IIOItem(QString name, QString id, IIOTypeChnl type, iio_channel *chnl, IIOItem *parent = nullptr);
	virtual ~IIOItem();
	int type() const override;

	// Navigation functions
	int childCount() const;
	void addChild(IIOItem *item);
	void removeChild(IIOItem *item);
	IIOItem *child(int index) const;
	IIOItem *child(QString name) const;
	IIOItem *parent() const;
	void setParent(IIOItem *parent);

	// Data functions
	QString name() const;
	QString id() const;

	iio_context *ctx() const;
	void setCtx(iio_context *newCtx);

	iio_device *dev() const;
	void setDev(iio_device *newDev);

	iio_channel *chnl() const;
	void setChnl(iio_channel *newChnl);

private:
	int m_type;
	IIOItem *m_parent;
	QList<IIOItem *> m_children;

	struct iio_context *m_ctx;
	struct iio_device *m_dev;
	struct iio_channel *m_chnl;

	QString m_name;
	QString m_id;
};
} // namespace scopy

#endif // IIOITEM_H
