#include "widgets/pagenavigationwidget.h"

#include <QBoxLayout>
#include <QDebug>
#include <QLoggingCategory>
#include <style.h>

#include <stylehelper.h>

using namespace scopy;
Q_LOGGING_CATEGORY(CAT_NAVIGATIONWIDGET, "PageNavigationWidget")

PageNavigationWidget::PageNavigationWidget(bool hasHome, bool hasOpen, QWidget *parent)
	: QWidget(parent)
{
	initUI(parent);

	setHomeBtnVisible(hasHome);
	setOpenBtnVisible(hasOpen);
}

PageNavigationWidget::~PageNavigationWidget() {}

void PageNavigationWidget::initUI(QWidget *parent)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QHBoxLayout *buttonsLayout = new QHBoxLayout();
	setLayout(buttonsLayout);
	buttonsLayout->setSpacing(10);

	homeButton = new QPushButton(this);
	StyleHelper::BlueIconButton(homeButton, Style::getPixmap(":/gui/icons/home.svg",  Style::getColor(json::theme::focus_item)), "homeButton");
	buttonsLayout->addWidget(homeButton);
	homeButton->hide();

	backwardButton = new QPushButton(this);
	StyleHelper::BlueIconButton(backwardButton, Style::getPixmap(":/gui/icons/handle_left_arrow.svg",  Style::getColor(json::theme::focus_item)),
				    "backwardButton");
	buttonsLayout->addWidget(backwardButton);

	forwardButton = new QPushButton(this);
	StyleHelper::BlueIconButton(forwardButton, Style::getPixmap(":/gui/icons/handle_right_arrow.svg",  Style::getColor(json::theme::focus_item)),
				    "forwardButton");
	buttonsLayout->addWidget(forwardButton);

	openButton = new QPushButton(this);
	StyleHelper::BlueIconButton(openButton, Style::getPixmap(":/gui/icons/sba_up_btn_pressed.svg",  Style::getColor(json::theme::focus_item)), "openButton");
	buttonsLayout->addWidget(openButton);
	openButton->hide();

	buttonsLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
}

void PageNavigationWidget::setHomeBtnVisible(bool en)
{
	homeButton->setVisible(en);
	setMinimumSize(sizeHint());
}

void PageNavigationWidget::setOpenBtnVisible(bool en)
{
	openButton->setVisible(en);
	setMinimumSize(sizeHint());
}

QPushButton *PageNavigationWidget::getHomeBtn() { return homeButton; }

QPushButton *PageNavigationWidget::getBackwardBtn() { return backwardButton; }

QPushButton *PageNavigationWidget::getForwardBtn() { return forwardButton; }

QPushButton *PageNavigationWidget::getOpenBtn() { return openButton; }

#include "moc_pagenavigationwidget.cpp"
