#ifndef DEVICEICONIMPL_H
#define DEVICEICONIMPL_H

#include <QWidget>
#include <QAbstractButton>
#include <deviceicon.h>
#include "device.h"
#include "scopycore_export.h"

namespace Ui {
	class DeviceButton;
};

namespace scopy {
class SCOPYCORE_EXPORT DeviceIconImpl : public DeviceIcon
{
	Q_OBJECT
public:
	explicit DeviceIconImpl(Device *d, QWidget *parent);
	~DeviceIconImpl();
	virtual void paintEvent(QPaintEvent *e) override;
public Q_SLOTS:
	void setConnected(bool) override;

private:
	Ui::DeviceButton *ui;


};
}

#endif // DEVICEICONIMPL_H
