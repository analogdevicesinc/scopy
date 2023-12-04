#include "searchbarwidget.hpp"
#include "utils.hpp"
#include "utils.h"

#include "regmapstylehelper.hpp"

#include <qboxlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <stylehelper.h>
#include "regmapstylehelper.hpp"

using namespace scopy;
using namespace regmap;

SearchBarWidget::SearchBarWidget(QWidget *parent)
	: QWidget{parent}
{
	layout = new QHBoxLayout();
	Utils::removeLayoutMargins(layout);
	setLayout(layout);

	searchBar = new QLineEdit();
	searchBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	searchBar->setPlaceholderText("Search for register ");
	searchButton = new QPushButton(this);
	QIcon icon1;
	icon1.addPixmap(Util::ChangeSVGColor(":/gui/icons/scopy-default/icons/search.svg", "white", 1));
	StyleHelper::SquareToggleButtonWithIcon(searchButton, "search_btn", false);
	searchButton->setIcon(icon1);

	QObject::connect(searchBar, &QLineEdit::returnPressed, searchButton, &QPushButton::pressed);

	QObject::connect(searchButton, &QPushButton::pressed, this,
			 [=]() { Q_EMIT requestSearch(searchBar->text().toLower()); });
	layout->addWidget(searchBar);
	layout->addWidget(searchButton);

	applyStyle();
}

SearchBarWidget::~SearchBarWidget()
{
	delete layout;
	delete searchBar;
	delete searchButton;
}

void SearchBarWidget::setEnabled(bool enabled)
{
	this->searchBar->setEnabled(enabled);
	this->searchButton->setVisible(enabled);
}

void SearchBarWidget::applyStyle() { RegmapStyleHelper::searchBarStyle(this); }
