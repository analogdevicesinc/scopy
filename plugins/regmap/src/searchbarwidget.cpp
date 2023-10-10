#include "searchbarwidget.hpp"

#include "regmapstylehelper.hpp"

#include <qboxlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>

using namespace scopy;
using namespace regmap;

SearchBarWidget::SearchBarWidget(QWidget *parent)
	: QWidget{parent}
{
	layout = new QHBoxLayout();
	setLayout(layout);

	searchBar = new QLineEdit();
	searchBar->setPlaceholderText("Search for regist by address");
	searchButton = new QPushButton("Search");

	QObject::connect(searchButton, &QPushButton::pressed, this,
			 [=]() { Q_EMIT requestSearch(searchBar->text().toLower()); });
	layout->addWidget(searchBar, 4);
	layout->addWidget(searchButton, 1);

	applyStyle();
}

SearchBarWidget::~SearchBarWidget()
{
	delete layout;
	delete searchBar;
	delete searchButton;
}

void SearchBarWidget::applyStyle() { RegmapStyleHelper::BlueButton(searchButton, ""); }
