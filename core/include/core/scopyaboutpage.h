#ifndef SCOPYABOUTPAGE_H
#define SCOPYABOUTPAGE_H

#include <QTabWidget>
#include <QTextBrowser>

#include "scopycore_export.h"

namespace scopy {
class SCOPYCORE_EXPORT ScopyAboutPage : public QTabWidget
{
	Q_OBJECT
public:
	ScopyAboutPage(QWidget *parent = nullptr);
	~ScopyAboutPage();
	void addHorizontalTab(QWidget *w, QString text);
	QWidget *buildPage(QString src, QTextDocument::ResourceType r = QTextDocument::MarkdownResource);
};
}

#endif // SCOPYABOUTPAGE_H
