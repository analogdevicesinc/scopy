#include "scopyaboutpage.h"

#include "widgets/hoverwidget.h"
#include "widgets/pagenavigationwidget.h"

#include <QLabel>
#include <QTabBar>
#include <QTabWidget>
#include <QVBoxLayout>

#include <stylehelper.h>

using namespace scopy;
ScopyAboutPage::ScopyAboutPage(QWidget *parent)
	: QWidget(parent)
	, tabWidget(new QTabWidget(this))
	, layout(new QVBoxLayout(this))
{
	initUI();
	addHorizontalTab(buildPage(QString("qrc:/about.html")), "Scopy");
}

void ScopyAboutPage::initUI()
{
	layout->setMargin(0);
	layout->setSpacing(0);
	this->setLayout(layout);
	layout->addWidget(tabWidget);

	StyleHelper::TabWidgetEastMenu(tabWidget, "aboutPageTable");
}

QWidget *ScopyAboutPage::buildPage(QString src)
{
	QWidget *page = new QWidget(this);
	QVBoxLayout *lay = new QVBoxLayout(page);
	QTextBrowser *browser = new QTextBrowser(page);

	lay->addWidget(browser);
	lay->setMargin(0);
	initNavigationWidget(browser);

	if(QFile::exists(QString(src).replace("qrc:/", ":/"))) {
		browser->setSource(src);
	} else {
		browser->setMarkdown(src);
	}

	return page;
}

void ScopyAboutPage::initNavigationWidget(QTextBrowser *browser)
{
	PageNavigationWidget *navWidget = new PageNavigationWidget(true, false, this);
	QPushButton *homeButton = navWidget->getHomeBtn();
	connect(homeButton, SIGNAL(clicked()), browser, SLOT(home()));

	QPushButton *backwardButton = navWidget->getBackwardBtn();
	backwardButton->setEnabled(false);
	connect(backwardButton, SIGNAL(clicked()), browser, SLOT(backward()));
	connect(browser, &QTextBrowser::backwardAvailable, backwardButton,
		[=](bool available) { backwardButton->setEnabled(available); });

	QPushButton *forwardButton = navWidget->getForwardBtn();
	forwardButton->setEnabled(false);
	connect(forwardButton, SIGNAL(clicked()), browser, SLOT(forward()));
	connect(browser, &QTextBrowser::forwardAvailable, forwardButton,
		[=](bool available) { forwardButton->setEnabled(available); });

	HoverWidget *hover = new HoverWidget(navWidget, browser, browser);
	hover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	hover->setContentPos(HoverPosition::HP_BOTTOMLEFT);
	hover->setAnchorOffset(QPoint(-10, 0));
	hover->show();
}

void ScopyAboutPage::addHorizontalTab(QWidget *w, QString text)
{
	// Hackish - so we don't override paint event
	tabWidget->addTab(w, "");
	QLabel *lbl1 = new QLabel();
	lbl1->setText(text);
	StyleHelper::TabWidgetLabel(lbl1, "tabWidgetLabel");
	QTabBar *tabbar = tabWidget->tabBar();
	tabbar->setTabButton(tabbar->count() - 1, QTabBar::RightSide, lbl1);
}

ScopyAboutPage::~ScopyAboutPage() {}

#include "moc_scopyaboutpage.cpp"
