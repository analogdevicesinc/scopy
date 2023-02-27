#include "scopyaboutpage.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTabBar>

using namespace adiscope;
ScopyAboutPage::ScopyAboutPage(QWidget *parent) : QTabWidget(parent)
{
	setTabPosition(TabPosition::West);
	addHorizontalTab(buildPage(QUrl("qrc:/about.html")),"Scopy");
}

QWidget* ScopyAboutPage::buildPage(QString src) {
	QTextBrowser *browser = new QTextBrowser(this);
	browser->setMarkdown(src);
	return browser;

}

QWidget* ScopyAboutPage::buildPage(QUrl src, QTextDocument::ResourceType r) {
	QWidget *page = new QWidget(this);
	QVBoxLayout *lay = new QVBoxLayout(page);

	QTextBrowser *browser = new QTextBrowser(page);
	lay->addWidget(browser, r);
	browser->setSource(src);	
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
	tabbar->setTabButton(tabbar->count() - 1, QTabBar::LeftSide, lbl1);

}

ScopyAboutPage::~ScopyAboutPage() {

}
