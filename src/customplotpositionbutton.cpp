#include "customplotpositionbutton.h"

#include "dynamicWidget.hpp"

#include "ui_customplotpositionbutton.h"

using namespace adiscope;

CustomPlotPositionButton::CustomPlotPositionButton(QWidget *parent)
	: ColoredQWidget(parent), ui(new Ui::CustomPlotPositionButton) {
	ui->setupUi(this);

	setDynamicProperty(ui->leftTop, "selected", true);
	setDynamicProperty(ui->rightTop, "selected", false);
	setDynamicProperty(ui->rightBot, "selected", false);
	setDynamicProperty(ui->leftBot, "selected", false);

	ui->leftTop->setChecked(true);

	btns = new QButtonGroup(this);

	connect(ui->leftTop, &QPushButton::toggled, [=](bool on) {
		setDynamicProperty(ui->leftTop, "selected", on);
		if (on)
			Q_EMIT positionChanged(topLeft);
	});
	connect(ui->rightTop, &QPushButton::toggled, [=](bool on) {
		setDynamicProperty(ui->rightTop, "selected", on);
		if (on)
			Q_EMIT positionChanged(topRight);
	});
	connect(ui->rightBot, &QPushButton::toggled, [=](bool on) {
		setDynamicProperty(ui->rightBot, "selected", on);
		if (on)
			Q_EMIT positionChanged(bottomRight);
	});
	connect(ui->leftBot, &QPushButton::toggled, [=](bool on) {
		setDynamicProperty(ui->leftBot, "selected", on);
		if (on)
			Q_EMIT positionChanged(bottomLeft);
	});

	btns->addButton(ui->leftBot);
	btns->addButton(ui->leftTop);
	btns->addButton(ui->rightBot);
	btns->addButton(ui->rightTop);
}

CustomPlotPositionButton::~CustomPlotPositionButton() { delete ui; }

void CustomPlotPositionButton::setPosition(ReadoutsPosition position) {
	switch (position) {
	case topLeft:
	default:
		ui->leftTop->setChecked(true);
		break;
	case topRight:
		ui->rightTop->setChecked(true);
		break;
	case bottomLeft:
		ui->leftBot->setChecked(true);
		break;
	case bottomRight:
		ui->rightBot->setChecked(true);
		break;
	}
}
