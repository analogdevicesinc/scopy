#ifndef DEVICEICONIMPL_H
#define DEVICEICONIMPL_H

#include <QWidget>
#include <QAbstractButton>
#include <deviceicon.h>
#include "scopycore_export.h"

namespace Ui {
	class DeviceButton;
};

namespace adiscope {
class SCOPYCORE_EXPORT DeviceIconImpl : public DeviceIcon
{
	Q_OBJECT
public:
	explicit DeviceIconImpl(QString name, QString description, QWidget *icon, QWidget *parent);
	~DeviceIconImpl();
	virtual void paintEvent(QPaintEvent *e) override;
public Q_SLOTS:
	void setConnected(bool) override;

private:
	Ui::DeviceButton *ui;


};
}

#endif // DEVICEICONIMPL_H
