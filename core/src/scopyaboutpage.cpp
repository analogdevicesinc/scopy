#include "scopyaboutpage.h"
#include "widgets/hoverwidget.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QTabBar>

using namespace scopy;
ScopyAboutPage::ScopyAboutPage(QWidget *parent) : QTabWidget(parent)
{
	setTabPosition(TabPosition::East);
	addHorizontalTab(buildPage(QString("qrc:/about.html")),"Scopy");
}

QWidget* ScopyAboutPage::buildPage(QString src) {
	QWidget *page = new QWidget(this);
	QVBoxLayout *lay = new QVBoxLayout(page);

	QTextBrowser *browser = new QTextBrowser(page);

	HoverWidget* hover = new HoverWidget(createNavigationWidget(browser), browser, browser);
	hover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	hover->setContentPos(HoverPosition::HP_BOTTOMLEFT);
	hover->setAnchorOffset(QPoint(-10, 0));
	hover->show();
	lay->addWidget(browser);

	if (QFile::exists(QString(src).replace("qrc:/", ":/"))) {
		browser->setSource(src);
	} else {
		browser->setMarkdown(src);
	}

	return page;
}

QPushButton *ScopyAboutPage::createNavigationButton(QIcon icon)
{
	QPushButton *btn = new QPushButton();
	int size = 24;

	btn->setIcon(icon);
	btn->setIconSize(QSize(size, size));
	btn->setProperty("blue_button", true);
	btn->setFixedHeight(size);
	btn->setFixedWidth(size);

	return btn;
}

QWidget* ScopyAboutPage::createNavigationWidget(QTextBrowser *browser)
{
	QWidget* widget = new QWidget();
	QHBoxLayout *buttonsLayout = new QHBoxLayout();
	widget->setLayout(buttonsLayout);
	buttonsLayout->setSpacing(10);

	QPushButton *homeButton = createNavigationButton(QIcon(":/gui/icons/launcher_home.svg"));
	buttonsLayout->addWidget(homeButton);
	connect(homeButton, SIGNAL(clicked()), browser, SLOT(home()));

	QPushButton *backwardButton = createNavigationButton(QIcon(":/gui/icons/handle_left_arrow.svg"));
	backwardButton->setEnabled(false);
	buttonsLayout->addWidget(backwardButton);
	connect(backwardButton, SIGNAL(clicked()), browser, SLOT(backward()));
	connect(browser, &QTextBrowser::backwardAvailable, backwardButton, [=](bool available){
		backwardButton->setEnabled(available);
	});

	QPushButton *forwardButton = createNavigationButton(QIcon(":/gui/icons/handle_right_arrow.svg"));
	forwardButton->setEnabled(false);
	buttonsLayout->addWidget(forwardButton);
	connect(forwardButton, SIGNAL(clicked()), browser, SLOT(forward()));
	connect(browser, &QTextBrowser::forwardAvailable, forwardButton, [=](bool available){
		forwardButton->setEnabled(available);
	});

	buttonsLayout->addSpacerItem(new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Fixed));
	widget->setMaximumSize(widget->minimumSizeHint());

	return widget;
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
