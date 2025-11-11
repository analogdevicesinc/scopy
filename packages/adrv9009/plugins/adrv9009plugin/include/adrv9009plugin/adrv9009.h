#ifndef ADRV9009_H
#define ADRV9009_H

#include "scopy-adrv9009plugin_export.h"
#include <QWidget>
#include <iio.h>

namespace scopy::adrv9009 {
class SCOPY_ADRV9009PLUGIN_EXPORT Adrv9009 : public QWidget
{
	Q_OBJECT
public:
	Adrv9009(iio_context *ctx, QWidget *parent = nullptr);
	~Adrv9009();

private:
	iio_context *m_ctx = nullptr;
	iio_device *m_iio_dev = nullptr;
};
} // namespace scopy::adrv9009
#endif // ADRV9009_H
