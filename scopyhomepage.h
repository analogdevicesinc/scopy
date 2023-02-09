#ifndef SCOPYHOMEPAGE_H
#define SCOPYHOMEPAGE_H

#include <QWidget>

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
Q_SIGNALS:

	void requestAddDevice(QString id);

public Q_SLOTS:
	void addDevice(QString id, QString name, QString description, QWidget *icon, QWidget *page);
	void removeDevice(QString id);


private:
	Ui::ScopyHomePage *ui;
};
}

#endif // SCOPYHOMEPAGE_H
