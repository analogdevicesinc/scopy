/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "styletool.h"

#include <QBoxLayout>
#include <QLineEdit>
#include <QScrollArea>
#include <QTabWidget>
#include <QTextBrowser>
#include <style.h>

using namespace scopy;

StyleTool::StyleTool(QWidget *parent)
{
	setLayout(new QHBoxLayout(this));

	QTabWidget *tab = new QTabWidget();
	tab->insertTab(0, buildStylePage(), "Properties");
	tab->insertTab(0, buildPageColors(), "Color");
	layout()->addWidget(tab);
}

QWidget *StyleTool::buildStylePage()
{
	// Create the main container widget
	QWidget *containerWidget = new QWidget;

	// Create the scroll area and set the container widget as its content
	QScrollArea *scrollArea = new QScrollArea;
	scrollArea->setWidgetResizable(true); // Allow resizing

	// Create a vertical layout for the container widget
	QVBoxLayout *layout = new QVBoxLayout;

	// Helper lambda to add a widget with a label to the layout
	auto addLabeledWidget = [&](const QString &labelText, QWidget *widget) {
		QWidget *hContainer = new QWidget; // Horizontal container
		QHBoxLayout *hLayout = new QHBoxLayout(hContainer);
		hLayout->setContentsMargins(0, 0, 0, 0); // Remove margins for tight layout

		QLabel *label = new QLabel(labelText);
		hLayout->addWidget(label);
		hLayout->addWidget(widget);

		if(dynamic_cast<QPushButton *>(widget))
			dynamic_cast<QPushButton *>(widget)->setCheckable(true);

		layout->addWidget(hContainer); // Add the horizontal container to the main layout
	};

	int i = 0;

	// Add styled buttons
	addLabeledWidget("Basic Button", new QPushButton("Basic Button"));
	addLabeledWidget("Single Button", new QPushButton("Single"));
	addLabeledWidget("Spinbox Button", new QPushButton("Spinbox Button"));
	addLabeledWidget("Run Button", new QPushButton("Run"));
	addLabeledWidget("Border Button", new QPushButton("Border Button"));
	addLabeledWidget("Square Icon Button", new QPushButton("Square Icon Button"));
	addLabeledWidget("Square Icon Browser Button", new QPushButton("Square Icon Browser Button"));
	addLabeledWidget("Stop Button", new QPushButton(""));
	addLabeledWidget("Subtle Button", new QPushButton("Subtle Button"));
	addLabeledWidget("Blue Gray Button", new QPushButton("Blue Gray Button"));
	addLabeledWidget("Gray Button", new QPushButton("Gray Button"));

	// Style the buttons
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::button::basicButton, true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::button::singleButton, true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::button::spinboxButton, true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::button::runButton, true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::button::borderButton, true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::button::squareIconButton, true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::button::squareIconBrowserButton, true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::button::stopButton, true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::button::subtleButton, true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::button::blueGrayButton, true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::button::grayButton, true, true);

	// Add styled widgets (using QWidget instead of QPushButton)
	addLabeledWidget("Basic Component", new QWidget);
	addLabeledWidget("Text Browser", new QTextBrowser);
	addLabeledWidget("Search Bar", new QLineEdit);
	addLabeledWidget("Solid Border", new QWidget);
	addLabeledWidget("Device Icon", new QWidget);
	addLabeledWidget("Not Interactive", new QWidget);
	addLabeledWidget("Border", new QWidget);

	// Style the widgets
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::widget::basicComponent, true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::widget::textBrowser, true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::widget::searchbar, true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::widget::solidBorder, true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::widget::deviceIcon, true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::widget::notInteractive, true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(), style::properties::widget::border,
			true, true);

	// Add styled checkboxes
	addLabeledWidget("Collapse Checkbox", new QCheckBox("Collapse Checkbox"));
	addLabeledWidget("Circle Checkbox", new QCheckBox("Circle Checkbox"));
	addLabeledWidget("Square Checkbox", new QCheckBox("Square Checkbox"));

	// Style the checkboxes
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::checkbox::collapseCB, true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::checkbox::circleCB, true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::checkbox::squareCB, true, true);

	// Add styled labels
	addLabeledWidget("Menu Small Label", new QLabel("Menu Small Label"));
	addLabeledWidget("Subtle Label", new QLabel("Subtle Label"));
	addLabeledWidget("Menu Medium Label", new QLabel("Menu Medium Label"));

	// Style the labels
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::label::menuSmall, true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(), style::properties::label::subtle,
			true, true);
	Style::setStyle(layout->itemAt(i++)->widget()->layout()->itemAt(1)->widget(),
			style::properties::label::menuMedium, true, true);


	// Set the layout to the container widget
	containerWidget->setLayout(layout);

	// Set the container widget as the content of the scroll area
	scrollArea->setWidget(containerWidget);

	// Create a final widget to hold the scroll area
	QWidget *finalWidget = new QWidget;
	QVBoxLayout *finalLayout = new QVBoxLayout(finalWidget);
	finalLayout->addWidget(scrollArea);

	return finalWidget;
}

QWidget *StyleTool::buildPageColors()
{
	// Create the main container widget
	QWidget *containerWidget = new QWidget;

	// Create the scroll area and set the container widget as its content
	QScrollArea *scrollArea = new QScrollArea;
	scrollArea->setWidgetResizable(true); // Allow resizing

	// Create a vertical layout for the container widget
	QVBoxLayout *layout = new QVBoxLayout;

	// Array of color properties and their labels
	const std::vector<std::pair<const char *, QString>> colors = {
		{json::theme::interactive_subtle_idle, "Interactive Subtle Idle"},
		{json::theme::interactive_subtle_hover, "Interactive Subtle Hover"},
		{json::theme::interactive_subtle_pressed, "Interactive Subtle Pressed"},
		{json::theme::interactive_subtle_disabled, "Interactive Subtle Disabled"},
		{json::theme::interactive_focus, "Interactive Focus"},
		{json::theme::interactive_primary_idle, "Interactive Primary Idle"},
		{json::theme::interactive_primary_hover, "Interactive Primary Hover"},
		{json::theme::interactive_primary_pressed, "Interactive Primary Pressed"},
		{json::theme::interactive_primary_disabled, "Interactive Primary Disabled"},
		{json::theme::interactive_secondary_idle, "Interactive Secondary Idle"},
		{json::theme::interactive_secondary_hover, "Interactive Secondary Hover"},
		{json::theme::interactive_secondary_pressed, "Interactive Secondary Pressed"},
		{json::theme::interactive_secondary_disabled, "Interactive Secondary Disabled"},
		{json::theme::content_default, "Content Default"},
		{json::theme::content_subtle, "Content Subtle"},
		{json::theme::content_silent, "Content Silent"},
		{json::theme::content_inverse, "Content Inverse"},
		{json::theme::danger_default, "Danger Default"},
		{json::theme::interactive_accent_idle, "Interactive Accent Idle"},
		{json::theme::background_subtle, "Background Subtle"},
		{json::theme::background_primary, "Background Primary"},
		{json::theme::background_secondary, "Background Secondary"},
		{json::theme::background_plot, "Background Plot"},
		{json::theme::interactive_accent_pressed, "Interactive Accent Pressed"},
		{json::theme::content_success, "Content Success"},
		{json::theme::content_error, "Content Error"},
		{json::theme::content_busy, "Content Busy"},
		{json::theme::color_default_hexcode, "Default Hex Code"},
		{json::theme::color_inverse_hexcode, "Inverse Hex Code"}};

	// Add color squares with labels to the layout
	for(const auto &colorPair : colors) {
		const char *colorProperty = colorPair.first;
		const QString &colorName = colorPair.second;

		QWidget *hContainer = new QWidget; // Horizontal container
		QHBoxLayout *hLayout = new QHBoxLayout(hContainer);
		hLayout->setContentsMargins(0, 0, 0, 0); // Remove margins for tight layout

		// Create a color square
		QFrame *colorSquare = new QFrame;
		colorSquare->setFixedSize(40, 40); // Increased size of the square
		colorSquare->setStyleSheet("background-color: " + Style::getColor(colorProperty).name() + ";");

		QLabel *label = new QLabel(colorName);
		label->setMargin(0); // Remove default margins for the label
		hLayout->addWidget(colorSquare);
		hLayout->addWidget(label);
		hLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

		layout->addWidget(hContainer); // Add the horizontal container to the main layout
	}

	// Set the layout to the container widget
	containerWidget->setLayout(layout);

	// Set the container widget as the content of the scroll area
	scrollArea->setWidget(containerWidget);

	// Create a final widget to hold the scroll area
	QWidget *finalWidget = new QWidget;
	QVBoxLayout *finalLayout = new QVBoxLayout(finalWidget);
	finalLayout->addWidget(scrollArea);

	return finalWidget;
}
