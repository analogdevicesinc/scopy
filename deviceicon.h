#ifndef DEVICEICON_H
#define DEVICEICON_H

#include <QWidget>
#include <QAbstractButton>

namespace Ui {
	class DeviceButton;
};

class DeviceIcon : public QAbstractButton
{
	Q_OBJECT
public:
	explicit DeviceIcon(QString name, QString description, QWidget *icon, QWidget *parent);
	~DeviceIcon();
	virtual void paintEvent(QPaintEvent *e) override;
private:
	Ui::DeviceButton *ui;

};

#endif // DEVICEICON_H
