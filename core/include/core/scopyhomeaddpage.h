#ifndef SCOPYHOMEADDPAGE_H
#define SCOPYHOMEADDPAGE_H

#include <QWidget>
#include "scopycore_export.h"
#include <QFuture>

namespace Ui {
class ScopyHomeAddPage;
}

namespace scopy {
class SCOPYCORE_EXPORT ScopyHomeAddPage : public QWidget
{
	Q_OBJECT

public:
	explicit ScopyHomeAddPage(QWidget *parent = nullptr);
	~ScopyHomeAddPage();

Q_SIGNALS:
	void requestAddDevice(QString, QString);
	void requestDevice(QString);

private Q_SLOTS:
	void add();
	void futureverify();
	bool verify();
	void deviceAddedToUi(QString);

private:
	Ui::ScopyHomeAddPage *ui;
	QString pendingUri;
	QFutureWatcher<bool> *fw;
};
}
#endif // SCOPYHOMEADDPAGE_H
