#ifndef DEVICEPAGE_H
#define DEVICEPAGE_H

#include <QWidget>
#include <scopycore_export.h>

namespace Ui {
class DevicePage;
}

class SCOPYCORE_EXPORT DevicePage : public QWidget
{
	Q_OBJECT

public:
	explicit DevicePage(QWidget *parent = nullptr);
	~DevicePage();

private:
	Ui::DevicePage *ui;
};

#endif // DEVICEPAGE_H
