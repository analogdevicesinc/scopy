#include "searchbarwidget.hpp"

#include <qboxlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include "dynamicWidget.h"

SearchBarWidget::SearchBarWidget(QWidget *parent)
	: QWidget{parent}
{
	searchBar = new QLineEdit();
	searchBar->setPlaceholderText("Search for regist by address");
	searchButton = new QPushButton("Search");
    scopy::setDynamicProperty(searchButton, "blue_button", true);
	layout = new QHBoxLayout();

	QObject::connect(searchButton, &QPushButton::pressed, this, [=](){
	      Q_EMIT requestSearch(searchBar->text().toLower());
	});
	layout->addWidget(searchBar,4);
	layout->addWidget(searchButton,1);

	setLayout(layout);
}

SearchBarWidget::~SearchBarWidget()
{
	delete layout;
	delete searchBar;
	delete searchButton;
}
