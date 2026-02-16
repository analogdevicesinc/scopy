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

#include "adcinstrument.h"
#include <pluginbase/resourcemanager.h>
#include <gui/smallOnOffSwitch.h>
#include <gui/style_properties.h>
#include <QDesktopServices>
#include <QLoggingCategory>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_ADCINSTRUMENT, "ADCInstrument")

using namespace scopy;
using namespace scopy::adc;

ADCInstrument::ADCInstrument(ToolMenuEntry *tme, QWidget *parent)
	: QWidget(parent)
	, m_tme(tme)
{
	setupToolLayout();
}

ADCInstrument::~ADCInstrument() {}

void ADCInstrument::setupToolLayout()
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout *lay = new QHBoxLayout(this);
	lay->setMargin(0);
	setLayout(lay);
	tool = new ToolTemplate(this);
	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->bottomContainer()->setVisible(true);
	tool->topContainer()->setVisible(true);
	tool->leftContainer()->setVisible(false);
	tool->rightContainer()->setVisible(false);
	tool->topCentral()->setVisible(true);
	tool->bottomCentral()->setVisible(false);
	lay->addWidget(tool);
	tool->setTopContainerHeight(100);
	tool->setBottomContainerHeight(90);

	// Create splitter-based layout for left/center/right
	m_splitter = new QSplitter(Qt::Horizontal, this);

	// Left panel
	m_leftPanel = new QWidget(m_splitter);
	QVBoxLayout *leftLayout = new QVBoxLayout(m_leftPanel);
	leftLayout->setMargin(0);
	leftLayout->setSpacing(0);
	m_leftStack = new MapStackedWidget(m_leftPanel);
	leftLayout->addWidget(m_leftStack);

	// Central widget (placeholder - actual content added by controllers)
	m_centralWidget = new QWidget(m_splitter);
	m_centralWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QVBoxLayout *centralLayout = new QVBoxLayout(m_centralWidget);
	centralLayout->setMargin(0);
	centralLayout->setSpacing(0);

	// Right panel
	m_rightPanel = new QWidget(m_splitter);
	QVBoxLayout *rightLayout = new QVBoxLayout(m_rightPanel);
	rightLayout->setMargin(0);
	rightLayout->setSpacing(0);
	rightStack = new MapStackedWidget(m_rightPanel);
	rightLayout->addWidget(rightStack);

	m_splitter->addWidget(m_leftPanel);
	m_splitter->addWidget(m_centralWidget);
	m_splitter->addWidget(m_rightPanel);
	m_splitter->setStretchFactor(0, 0);
	m_splitter->setStretchFactor(1, 1);
	m_splitter->setStretchFactor(2, 0);
	m_splitter->setSizes(QList<int>() << 210 << 600 << 310);

	tool->addWidgetToCentralContainerHelper(m_splitter);

	rightMenuBtnGrp = new SemiExclusiveButtonGroup(this);
	tool->topContainerMenuControl()->setVisible(false);

	tool->openBottomContainerHelper(false);
	tool->openTopContainerHelper(false);

	m_settingsBtn = new MenuControlButton(this);
	m_settingsBtn->setName("General Settings");
	m_settingsBtn->setOpenMenuChecksThis(true);
	m_settingsBtn->setDoubleClickToOpenMenu(false);
	m_settingsBtn->checkBox()->setVisible(false);

	InfoBtn *infoBtn = new InfoBtn(this);
	m_printBtn = new PrintBtn(this);
	printPlotManager = new PrintPlotManager(this);
	addBtn = new AddBtn(this);
	removeBtn = new RemoveBtn(this);

	connect(infoBtn, &QAbstractButton::clicked, this, [=]() {
		QDesktopServices::openUrl(QUrl("https://analogdevicesinc.github.io/scopy/plugins/adc/adc.html"));
	});

	m_sync = new QPushButton("Sync");
	m_sync->setFixedWidth(150);
	m_sync->setCheckable(true);
	Style::setStyle(m_sync, style::properties::button::blueGrayButton);
	Style::setStyle(m_sync, style::properties::label::menuMedium);

	m_complex = new QPushButton("Complex");
	m_complex->setCheckable(true);
	m_complex->setFixedWidth(150);
	Style::setStyle(m_complex, style::properties::button::blueGrayButton);
	Style::setStyle(m_complex, style::properties::label::menuMedium);

	m_runBtn = new RunBtn(this);
	m_singleBtn = new SingleShotBtn(this);

	hoverMenuGroup = new SemiExclusiveButtonGroup(this);
	m_cursor = new MenuControlButton(this);
	setupCursorButtonHelper(m_cursor);

	m_analyze = new MenuControlButton(this);
	setupAnalyzeButtonHelper(m_analyze);
	m_analyze->setVisible(false);

	tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(m_singleBtn, TTA_RIGHT);

	tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(m_printBtn, TTA_LEFT);

	tool->addWidgetToTopContainerHelper(addBtn, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(removeBtn, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(m_sync, TTA_LEFT);

	tool->addWidgetToBottomContainerHelper(m_complex, TTA_LEFT);
	tool->addWidgetToBottomContainerHelper(m_analyze, TTA_RIGHT);
	tool->addWidgetToBottomContainerHelper(m_cursor, TTA_RIGHT);

	rightMenuBtnGrp->addButton(m_settingsBtn->button());

	setupChannelMenu();
	setupRunSingleButtonHelper();

	channelGroup = new QButtonGroup(this);
	channelGroup->addButton(m_settingsBtn);

	connect(m_settingsBtn->button(), &QAbstractButton::toggled, this, [=](bool b) {
		if(b)
			rightStack->show(settingsMenuId);
	});
	connect(m_settingsBtn, &QAbstractButton::clicked, this, [=](bool b) {
		if(b)
			rightStack->show(settingsMenuId);
	});

	connect(m_runBtn, &QAbstractButton::toggled, this, [=](bool b) {
		m_runBtn->setEnabled(false);
		if(b) {
			Q_EMIT requestStart();
		} else {
			Q_EMIT requestStop();
		}
	});

	connect(m_tme, &ToolMenuEntry::runToggled, m_runBtn, &QAbstractButton::toggle);

	connect(addBtn, &QAbstractButton::clicked, this, [=]() { Q_EMIT requestNewInstrument(TIME); });

	connect(removeBtn, &QAbstractButton::clicked, this, &ADCInstrument::requestDeleteInstrument);
}

void ADCInstrument::setupRunSingleButtonHelper() {}

void ADCInstrument::setupChannelMenu()
{
	m_vcm = new VerticalChannelManager(this);
	m_vcm->addTop(m_settingsBtn);
	m_leftStack->add(verticalChannelManagerId, m_vcm);
}

void ADCInstrument::addDevice(CollapsableMenuControlButton *b, ToolComponent *dev)
{
	auto devBtn = b;
	m_vcm->add(b);
	QWidget *dev_widget = dynamic_cast<QWidget *>(dev);
	Q_ASSERT(dev_widget);

	channelGroup->addButton(b->getControlBtn());
	QString id = dev->name() + QString::number(uuid++);
	rightStack->add(id, dev_widget);

	connect(b->getControlBtn(), &QPushButton::clicked /* Or ::toggled*/, this, [=](bool b) {
		if(b) {
			rightStack->show(id);
		}
	});
}

void ADCInstrument::switchToChannelMenu(QString id, bool force)
{
	Q_UNUSED(force);
	rightStack->show(id);
}

void ADCInstrument::addChannel(MenuControlButton *btn, ChannelComponent *ch, CompositeWidget *c)
{
	c->add(btn);
	channelGroup->addButton(btn);

	QString id = ch->name() + QString::number(uuid++);
	QWidget *ch_widget = dynamic_cast<QWidget *>(ch);
	Q_ASSERT(ch_widget);

	rightStack->add(id, ch_widget);

	connect(btn->button(), &QPushButton::pressed, this, [=]() { Q_EMIT btn->clicked(true); });
	connect(btn, &QAbstractButton::clicked, this, [=](bool b) {
		if(b) {
			switchToChannelMenu(id, true);
		}
	});

	connect(ch, &ChannelComponent::requestChannelMenu, this, [=](bool f) { switchToChannelMenu(id, f); });

	/*setupChannelSnapshot(ch);
	setupChannelMeasurement(ch);
	setupChannelDelete(ch);*/
}
// #endif

QPushButton *ADCInstrument::sync() const { return m_sync; }

void ADCInstrument::stopped()
{
	m_tme->setRunning(false);
	QSignalBlocker sb(m_runBtn);
	m_singleBtn->setChecked(false);
	m_runBtn->setEnabled(true);
	m_runBtn->setChecked(false);
	m_runBtn->setText("Start");
}

void ADCInstrument::started()
{
	m_tme->setRunning(true);
	QSignalBlocker sb(m_runBtn);
	m_runBtn->setChecked(true);
	m_runBtn->setEnabled(true);
	m_runBtn->setText("Stop");
}

VerticalChannelManager *ADCInstrument::vcm() const { return m_vcm; }

ToolTemplate *ADCInstrument::getToolTemplate() { return tool; }

MapStackedWidget *ADCInstrument::getRightStack() { return rightStack; }

QWidget *ADCInstrument::getCentralWidget() { return m_centralWidget; }

QButtonGroup *ADCInstrument::getHoverMenuBtnGroup() { return hoverMenuGroup; }

void ADCInstrument::setupCursorButtonHelper(MenuControlButton *cursor)
{
	cursor->setName("Cursors");
	cursor->setOpenMenuChecksThis(true);
	cursor->setDoubleClickToOpenMenu(true);
	cursor->checkBox()->setVisible(false);
	cursor->setCheckBoxStyle(MenuControlButton::CS_SQUARE);
	hoverMenuGroup->addButton(cursor->button());
}

void ADCInstrument::setupAnalyzeButtonHelper(MenuControlButton *analyze)
{
	analyze->setName("Genalyzer");
	analyze->setOpenMenuChecksThis(true);
	analyze->setDoubleClickToOpenMenu(true);
	analyze->checkBox()->setVisible(false);
	analyze->setCheckBoxStyle(MenuControlButton::CS_SQUARE);
	hoverMenuGroup->addButton(analyze->button());
}
