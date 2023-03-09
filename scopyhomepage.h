#ifndef SCOPYHOMEPAGE_H
#define SCOPYHOMEPAGE_H

#include "scopyhomeaddpage.h"
#include <QWidget>
#include <QPushButton>

namespace Ui {
class ScopyHomePage;
}

namespace adiscope {
class ScopyHomePage : public QWidget
{
	Q_OBJECT

public:
	explicit ScopyHomePage(QWidget *parent = nullptr);
	~ScopyHomePage();
	QPushButton* scanControlBtn();
Q_SIGNALS:

	void requestAddDevice(QString id);
	void requestRemoveDevice(QString id);
	void requestDevice(QString id);

public Q_SLOTS:
	void addDevice(QString id, QString name, QString description, QWidget *icon, QWidget *page);
	void removeDevice(QString id);
	void connectDevice(QString);
	void disconnectDevice(QString);

private:
	Ui::ScopyHomePage *ui;
	ScopyHomeAddPage *add;
};
}

#endif // SCOPYHOMEPAGE_H
