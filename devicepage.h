#ifndef DEVICEPAGE_H
#define DEVICEPAGE_H

#include <QWidget>

namespace Ui {
class DevicePage;
}

class DevicePage : public QWidget
{
	Q_OBJECT

public:
	explicit DevicePage(QWidget *parent = nullptr);
	~DevicePage();

private:
	Ui::DevicePage *ui;
};

#endif // DEVICEPAGE_H
