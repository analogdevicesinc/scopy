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

Q_LOGGING_CATEGORY(CAT_QIQ_SETTINGS, "QIQSettingsMenu");

using namespace scopy::qiqplugin;

SettingsMenu::SettingsMenu(QWidget *parent)
	: QWidget(parent)
{
	setupUI();
}

void SettingsMenu::setupUI()
{
	QWidget *contentWidget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout(contentWidget);
	layout->setMargin(0);

	MenuHeaderWidget *header = new MenuHeaderWidget(
		"Settings", QPen(Style::getAttribute(json::theme::interactive_primary_idle)), this);

	// Buffer menu
	MenuSectionCollapseWidget *bufferAcq = new MenuSectionCollapseWidget(
		"Acquisition", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, contentWidget);
	m_bufferMenu = new BufferMenu();
	bufferAcq->add(m_bufferMenu);

	// Analysis cb
	MenuSectionCollapseWidget *analysisCb = new MenuSectionCollapseWidget(
		"Select analysis", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, contentWidget);
	m_analysisCb = new MenuCombo("Analysis");
	analysisCb->add(m_analysisCb);

	// Analysis menu
	MenuSectionCollapseWidget *analysisMenu =
		new MenuSectionCollapseWidget("Analysis settings", MenuCollapseSection::MHCW_NONE,
					      MenuCollapseSection::MHW_BASEWIDGET, contentWidget);
	m_analysisMenu = new AnalysisMenu();
	analysisMenu->add(m_analysisMenu);

	// Plot settings
	MenuSectionCollapseWidget *plotSettings =
		new MenuSectionCollapseWidget("Select plot settings", MenuCollapseSection::MHCW_NONE,
					      MenuCollapseSection::MHW_BASEWIDGET, contentWidget);
	m_selectPlotCb = new QComboBox(plotSettings);
	plotSettings->add(m_selectPlotCb);

	m_plotSettings = new QWidget(contentWidget);
	m_plotSettings->setLayout(new QVBoxLayout());
	m_plotSettings->layout()->setMargin(0);

	layout->addWidget(bufferAcq);
	layout->addWidget(analysisCb);
	layout->addWidget(analysisMenu);
	layout->addWidget(plotSettings);
	layout->addWidget(m_plotSettings);
	layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidget(contentWidget);
	scrollArea->setWidgetResizable(true);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setFrameShape(QFrame::NoFrame);

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->addWidget(header);
	mainLayout->addWidget(scrollArea);

	// Connections
	connect(m_selectPlotCb, &QComboBox::currentTextChanged, this, &SettingsMenu::plotSettings);
	connect(m_analysisCb->combo(), &QComboBox::currentTextChanged, this, &SettingsMenu::analysisChanged);
	connect(m_bufferMenu, &BufferMenu::bufferParamsChanged, this, &SettingsMenu::bufferParamsChanged);
	connect(m_analysisMenu, &AnalysisMenu::applyPressed, this, &SettingsMenu::onAnalysisApply);
}

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
		qWarning(CAT_QIQ_SETTINGS) << "The current type is:" << currentType << "not" << type;
	}

	for(auto it = config.begin(); it != config.end(); ++it) {
		if(!currentConfig.contains(it.key())) {
			continue;
		}
		if(currentConfig[it.key()] != it.value()) {
			qWarning(CAT_QIQ_SETTINGS) << "Different values for the field:" << it.key();
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

void SettingsMenu::onAnalysisApply()
{
	QString analysisType = m_analysisCb->combo()->currentText();
	QVariantMap config = m_analysisMenu->getAnalysisConfig();
	Q_EMIT analysisConfig(analysisType, config);
}
