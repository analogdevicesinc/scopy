#ifndef SCOPYHOMEPAGE_H
#define SCOPYHOMEPAGE_H

#include "scopyhomeaddpage.h"
#include <QWidget>
#include <QPushButton>
#include "scopycore_export.h"
#include "device.h"

namespace Ui {
class ScopyHomePage;
}

namespace scopy {
class SCOPYCORE_EXPORT ScopyHomePage : public QWidget
{
	Q_OBJECT

public:
	explicit ScopyHomePage(QWidget *parent = nullptr);
	~ScopyHomePage();
	QPushButton* scanControlBtn();
Q_SIGNALS:

	void requestAddDevice(QString cat, QString id);
	void requestRemoveDevice(QString id);
	void requestDevice(QString id);
	void deviceAddedToUi(QString id);

public Q_SLOTS:
	void addDevice(QString id, Device*);
	void removeDevice(QString id);
	void viewDevice(QString id);
	void connectDevice(QString);
	void disconnectDevice(QString);

private:
	Ui::ScopyHomePage *ui;
	ScopyHomeAddPage *add;
};
}

#endif // SCOPYHOMEPAGE_H
