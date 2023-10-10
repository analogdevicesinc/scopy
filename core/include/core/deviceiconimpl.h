#ifndef DEVICEICONIMPL_H
#define DEVICEICONIMPL_H

#include "device.h"
#include "gui/utils.h"
#include "scopy-core_export.h"

#include <QAbstractButton>
#include <QPaintEvent>
#include <QWidget>

#include <deviceicon.h>

namespace Ui {
class DeviceButton;
};

namespace scopy {
class SCOPY_CORE_EXPORT DeviceIconImpl : public DeviceIcon
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	explicit DeviceIconImpl(Device *d, QWidget *parent);
	~DeviceIconImpl();
public Q_SLOTS:
	void setConnected(bool) override;

private:
	Ui::DeviceButton *ui;
};
} // namespace scopy

#endif // DEVICEICONIMPL_H
