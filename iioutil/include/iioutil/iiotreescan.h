#ifndef IIOTREESCAN_H
#define IIOTREESCAN_H

#include <QObject>
#include "iioitem.h"
#include <iio.h>
#include "scopy-iioutil_export.h"

#include <mutex>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IIOTreeScan : public QObject
{
	Q_OBJECT
public:
	explicit IIOTreeScan(struct iio_context *ctx, QObject *parent = nullptr);
	~IIOTreeScan();
	IIOItem *getRoot();

private:
	IIOItem *m_rootItem;
};
} // namespace scopy

#endif // IIOTREESCAN_H
