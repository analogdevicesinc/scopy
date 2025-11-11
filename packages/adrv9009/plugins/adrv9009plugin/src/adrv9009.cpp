#include "adrv9009.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_ADRV9009, "Adrv9009")
using namespace scopy::adrv9009;

Adrv9009::Adrv9009(iio_context *ctx, QWidget *parent)
	: QWidget(parent)
	, m_ctx(ctx)
{
	if(!m_ctx) {
		qWarning(CAT_ADRV9009) << "No IIO context provided";
		return;
	}

	// Find ADRV9009 PHY device
	m_iio_dev = iio_context_find_device(m_ctx, "adrv9009-phy");
	if(!m_iio_dev) {
		qWarning(CAT_ADRV9009) << "ADRV9009 PHY device not found in context";
		return;
	}

	qDebug(CAT_ADRV9009) << "ADRV9009 tool initialized with device:" << iio_device_get_name(m_iio_dev);
}

Adrv9009::~Adrv9009() {}
