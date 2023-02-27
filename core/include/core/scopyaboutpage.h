#ifndef SCOPYABOUTPAGE_H
#define SCOPYABOUTPAGE_H

#include <QTabWidget>
#include <QTextBrowser>

#include "scopycore_export.h"

namespace adiscope {
class SCOPYCORE_EXPORT ScopyAboutPage : public QTabWidget
{
	Q_OBJECT
public:
	ScopyAboutPage(QWidget *parent = nullptr);
	~ScopyAboutPage();
	void addHorizontalTab(QWidget *w, QString text);
	QWidget *buildPage(QUrl src, QTextDocument::ResourceType r = QTextDocument::UnknownResource);
	QWidget *buildPage(QString src);
};
}

#endif // SCOPYABOUTPAGE_H
