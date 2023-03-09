#ifndef SCOPYHOMEADDPAGE_H
#define SCOPYHOMEADDPAGE_H

#include <QWidget>

namespace Ui {
class ScopyHomeAddPage;
}

class ScopyHomeAddPage : public QWidget
{
	Q_OBJECT

public:
	explicit ScopyHomeAddPage(QWidget *parent = nullptr);
	~ScopyHomeAddPage();

Q_SIGNALS:
	void requestAddDevice(QString);
	void requestDevice(QString);

private Q_SLOTS:
	void add();
	void verify();

private:
	Ui::ScopyHomeAddPage *ui;
};

#endif // SCOPYHOMEADDPAGE_H
