/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "settingsmenu.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <menuheader.h>
#include <style.h>
#include <menusectionwidget.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_EXT_SETTINGS, "ExtSettingsMenu");

using namespace scopy::extprocplugin;

SettingsMenu::SettingsMenu(QObject *parent)
	: QObject(parent)
	, m_plotW(nullptr)
	, m_acqW(nullptr)
{
	setupUI();
}

void SettingsMenu::setupUI()
{

	createAcqMenu();
	createPlotMenu();

	// Connections
	connect(m_selectPlotCb, &QComboBox::currentTextChanged, this, &SettingsMenu::plotSettings);
	connect(m_analysisCb->combo(), &QComboBox::currentTextChanged, this, &SettingsMenu::analysisChanged);
	connect(m_bufferMenu, &BufferMenu::bufferParamsChanged, this, &SettingsMenu::bufferParamsChanged);
	connect(m_analysisMenu, &AnalysisMenu::applyPressed, this, &SettingsMenu::onAnalysisApply);
}

void SettingsMenu::createAcqMenu()
{
	m_acqW = createMenuW("Acquisition Setup");

	QWidget *acqWidget = new QWidget();
	QVBoxLayout *acqLay = new QVBoxLayout(acqWidget);
	acqLay->setMargin(0);

	// Buffer menu
	MenuSectionCollapseWidget *bufferAcq = new MenuSectionCollapseWidget(
		"Acquisition", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, acqWidget);
	m_bufferMenu = new BufferMenu();
	bufferAcq->add(m_bufferMenu);

	// Analysis cb
	MenuSectionCollapseWidget *analysisCb = new MenuSectionCollapseWidget(
		"Select Analysis", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, acqWidget);
	m_analysisCb = new MenuCombo("Analysis");
	analysisCb->add(m_analysisCb);

	// Analysis menu
	MenuSectionCollapseWidget *analysisMenu = new MenuSectionCollapseWidget(
		"Analysis Settings", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, acqWidget);
	m_analysisMenu = new AnalysisMenu();
	analysisMenu->add(m_analysisMenu);

	acqLay->addWidget(bufferAcq);
	acqLay->addWidget(analysisCb);
	acqLay->addWidget(analysisMenu);
	acqLay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	QScrollArea *scrollArea = createScrollArea(acqWidget);
	m_acqW->layout()->addWidget(scrollArea);
}

void SettingsMenu::createPlotMenu()
{
	m_plotW = createMenuW("Plot Settings");

	QWidget *plotWidget = new QWidget();
	QVBoxLayout *plotLay = new QVBoxLayout(plotWidget);
	// The right margin is used to avoid the scroll bar overlapping
	QMargins rightMargin(0, 0, 2, 0);
	plotLay->setContentsMargins(rightMargin);

	// Plot settings
	MenuSectionCollapseWidget *plotSettings = new MenuSectionCollapseWidget(
		"Select Plot", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, plotWidget);
	m_selectPlotCb = new QComboBox(plotSettings);
	plotSettings->add(m_selectPlotCb);

	m_plotSettings = new QWidget(plotWidget);
	m_plotSettings->setLayout(new QVBoxLayout());
	m_plotSettings->layout()->setMargin(0);

	plotLay->addWidget(plotSettings);
	plotLay->addWidget(m_plotSettings);
	plotLay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	QScrollArea *scrollArea = createScrollArea(plotWidget);
	m_plotW->layout()->addWidget(scrollArea);
}

QScrollArea *SettingsMenu::createScrollArea(QWidget *contentWidget)
{
	QScrollArea *scrollArea = new QScrollArea();
	scrollArea->setWidget(contentWidget);
	scrollArea->setWidgetResizable(true);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setFrameShape(QFrame::NoFrame);

	return scrollArea;
}

QWidget *SettingsMenu::createMenuW(const QString &title, QWidget *parent)
{
	QWidget *menu = new QWidget(parent);
	menu->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QVBoxLayout *lay = new QVBoxLayout(menu);
	lay->setMargin(0);
	MenuHeaderWidget *header =
		new MenuHeaderWidget(title, QPen(Style::getAttribute(json::theme::interactive_primary_idle)), menu);
	lay->addWidget(header);
	return menu;
}

QWidget *SettingsMenu::plotW() const { return m_plotW; }

QWidget *SettingsMenu::acqW() const { return m_acqW; }

void SettingsMenu::setAvailableChannels(const QMap<QString, QList<ChannelInfo>> &channels)
{
	m_bufferMenu->setAvailableChannels(channels);
}

void SettingsMenu::setAnalysisTypes(const QStringList &types)
{
	m_analysisCb->combo()->clear();
	m_analysisCb->combo()->addItems(types);
}

void SettingsMenu::setPlotTitle(const QStringList &title)
{
	m_selectPlotCb->clear();
	m_selectPlotCb->addItems(title);
}

void SettingsMenu::setAnalysisParams(const QString &type, const QVariantMap &params)
{
	if(type == m_analysisCb->combo()->currentText()) {
		m_analysisMenu->createMenu(params);
	}
}

void SettingsMenu::validateAnalysisParams(const QString &type, const QVariantMap &config)
{
	QString currentType = m_analysisCb->combo()->currentText();
	QVariantMap currentConfig = m_analysisMenu->getAnalysisConfig();
	if(type != currentType) {
		qWarning(CAT_EXT_SETTINGS) << "The current type is:" << currentType << "not" << type;
	}

	for(auto it = config.begin(); it != config.end(); ++it) {
		if(!currentConfig.contains(it.key())) {
			continue;
		}
		if(currentConfig[it.key()] != it.value()) {
			qWarning(CAT_EXT_SETTINGS) << "Different values for the field:" << it.key();
		}
	}
}

QString SettingsMenu::getCrtAnalysisType() { return m_analysisCb->combo()->currentText(); }

void SettingsMenu::onSettingsMenu(QWidget *w)
{
	QVBoxLayout *lay = dynamic_cast<QVBoxLayout *>(m_plotSettings->layout());
	QLayoutItem *item;
	while((item = lay->takeAt(0)) != nullptr) {
		if(item->widget()) {
			item->widget()->setParent(nullptr);
		}
		delete item;
	}
	if(!w) {
		return;
	}
	lay->addWidget(w);
}

void SettingsMenu::changeSettings(const QString &plotTitle) { m_selectPlotCb->setCurrentText(plotTitle); }

void SettingsMenu::onAnalysisApply()
{
	QString analysisType = m_analysisCb->combo()->currentText();
	QVariantMap config = m_analysisMenu->getAnalysisConfig();
	Q_EMIT analysisConfig(analysisType, config);
}
