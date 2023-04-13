#include "scopyaboutpage.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTabBar>

using namespace scopy;
ScopyAboutPage::ScopyAboutPage(QWidget *parent) : QTabWidget(parent)
{
	setTabPosition(TabPosition::East);
	addHorizontalTab(buildPage(QString("qrc:/about.html"), QTextDocument::HtmlResource),"Scopy");
}

QWidget* ScopyAboutPage::buildPage(QString src, QTextDocument::ResourceType r) {
	QWidget *page = new QWidget(this);
	QVBoxLayout *lay = new QVBoxLayout(page);

	QTextBrowser *browser = new QTextBrowser(page);
	lay->addWidget(browser);
	switch(r) {
	case QTextDocument::MarkdownResource:
		browser->setMarkdown(src);
		break;
	case QTextDocument::UnknownResource:
	case QTextDocument::HtmlResource:
	default:
		browser->setSource(src,r);
		break;
	}
	QPushButton *backButton = new QPushButton("Back",page);
	backButton->setProperty("blue_button",true);
	lay->addWidget(backButton);
	connect(backButton,SIGNAL(clicked()),browser,SLOT(backward()));
	return page;
}

void ScopyAboutPage::addHorizontalTab(QWidget *w, QString text) {
	// Hackish - so we don't override paint event
	addTab(w, "");
	QLabel *lbl1 = new QLabel();
	lbl1->setText(text);
	QTabBar *tabbar = tabBar();
	tabbar->setTabButton(tabbar->count() - 1, QTabBar::RightSide, lbl1);

}

ScopyAboutPage::~ScopyAboutPage() {

}

#include "moc_scopyaboutpage.cpp"
