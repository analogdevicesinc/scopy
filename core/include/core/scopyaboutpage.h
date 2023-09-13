#ifndef SCOPYABOUTPAGE_H
#define SCOPYABOUTPAGE_H

#include <QBoxLayout>
#include <QTabWidget>
#include <QTextBrowser>
#include <QPushButton>

#include "scopy-core_export.h"

namespace scopy {
class SCOPY_CORE_EXPORT ScopyAboutPage : public QWidget
{
	Q_OBJECT
public:
	ScopyAboutPage(QWidget *parent = nullptr);
	~ScopyAboutPage();
	void addHorizontalTab(QWidget *w, QString text);
	QWidget *buildPage(QString src);
private:
	void initUI();
	void initNavigationWidget(QTextBrowser *browser);
	QTabWidget *tabWidget;
	QVBoxLayout *layout;
};
}

#endif // SCOPYABOUTPAGE_H
