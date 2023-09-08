#ifndef SCOPYABOUTPAGE_H
#define SCOPYABOUTPAGE_H

#include <QTabWidget>
#include <QTextBrowser>
#include <QPushButton>

#include "scopy-core_export.h"

namespace scopy {
class SCOPY_CORE_EXPORT ScopyAboutPage : public QTabWidget
{
	Q_OBJECT
public:
	ScopyAboutPage(QWidget *parent = nullptr);
	~ScopyAboutPage();
	void addHorizontalTab(QWidget *w, QString text);
	QWidget *buildPage(QString src);
private:
	QWidget* createNavigationWidget(QTextBrowser *browser);
	QPushButton *createNavigationButton(QIcon icon);
};
}

#endif // SCOPYABOUTPAGE_H
