#include "searchbarwidget.hpp"
#include "utils.hpp"
#include "utils.h"

#include "regmapstylehelper.hpp"

#include <qboxlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <style.h>
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

	QString iconPath = ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/search.svg";
	searchButton->setIcon(Style::getPixmap(iconPath, Style::getColor(json::theme::content_default)));

	connect(searchButton, &QPushButton::toggled, this, [=](bool toggle) {
		const char *color = toggle ? json::theme::content_inverse : json::theme::content_default;
		searchButton->setIcon(Style::getPixmap(iconPath, Style::getColor(color)));
	});
	StyleHelper::SquareToggleButtonWithIcon(searchButton, "search_btn", false);

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
