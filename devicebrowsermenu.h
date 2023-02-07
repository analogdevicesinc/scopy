#ifndef DEVICEBROWSERMENU_H
#define DEVICEBROWSERMENU_H

#include <QWidget>
#include <QHBoxLayout>
#include <QAbstractButton>

class DeviceBrowserMenuItem : public QWidget {

};
namespace adiscope {
class DeviceBrowserMenu : public QWidget
{
	Q_OBJECT
public:
	explicit DeviceBrowserMenu(QWidget *parent = nullptr);
	~DeviceBrowserMenu();
	QAbstractButton *getDeviceWidgetFor(QString id);
	void addDevice(QString id, QAbstractButton *w, int position = -1);
	void removeDevice(QString id);
	QButtonGroup *getButtonGroup() const;

Q_SIGNALS:
	void requestDeviceSelect(QString);

private:
	QHBoxLayout *layout;
	QList<QAbstractButton*> list;
	QButtonGroup *bg;

	const char* devBrowserId = "DeviceBrowserId";

};
}

#endif // DEVICEBROWSERMENU_H
