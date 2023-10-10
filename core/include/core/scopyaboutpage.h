#ifndef SCOPYABOUTPAGE_H
#define SCOPYABOUTPAGE_H

#include "scopy-core_export.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QTextBrowser>

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
} // namespace scopy

#endif // SCOPYABOUTPAGE_H
