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

#include "plotmanager/plotcreator.h"
#include "menuplotchannelcurvestylecontrol.h"
#include <extprocutils.h>
#include <menuonoffswitch.h>
#include <menuplotaxisrangecontrol.h>
#include <menusectionwidget.h>
#include <plotaxis.h>
#include <style.h>
#include <stylehelper.h>
#include <pluginbase/preferences.h>

using namespace scopy::gui;
using namespace scopy::extprocplugin;

StandardPlotCreator::StandardPlotCreator(const ExtProcPlotInfo &plotInfo, QObject *parent)
	: PlotCreatorBase(parent)
	, m_plotWidget(nullptr)
	, m_plotSettings(nullptr)
	, m_chnlsTab(nullptr)
	, m_plotAutoscaler(new PlotAutoscaler(this))
	, m_curveControl(nullptr)
{
	m_plotAutoscaler->setXAxisMode(true);
	m_plotInfo = plotInfo;
	m_curveControl = new MenuPlotChannelCurveStyleControl(nullptr);
	init(plotInfo);
	connect(m_plotAutoscaler, &PlotAutoscaler::newMin, this, &StandardPlotCreator::onNewMin);
	connect(m_plotAutoscaler, &PlotAutoscaler::newMax, this, &StandardPlotCreator::onNewMax);
}

QWidget *StandardPlotCreator::plot() const { return m_plotWidget; }

void StandardPlotCreator::init(const ExtProcPlotInfo &plotInfo)
{
	m_plotWidget = new PlotWidget();
	bool useDock = Preferences::get("general_use_docking_if_available").toBool();
	if(!useDock) {
		m_plotWidget->plot()->setTitle(plotInfo.title);
	}

	setupPlotChannels();
	configurePlotAxis();
	applyPlotFlags();
	createPlotSettings();

	QPushButton *settingsPlotHover = new QPushButton("");
	settingsPlotHover->setMaximumSize(16, 16);
	settingsPlotHover->setIcon(
		QIcon(":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/preferences.svg"));

	connect(settingsPlotHover, &QAbstractButton::clicked, this,
		[this, plotInfo]() { Q_EMIT requestSettings(plotInfo.title); });

	QWidget *paddingW = new QWidget();
	paddingW->setVisible(false);

	m_plotWidget->plotButtonManager()->add(settingsPlotHover);
	m_plotWidget->plotButtonManager()->add(paddingW);
	m_plotWidget->plotButtonManager()->setVisible(true);

	m_plotWidget->replot();
}

QWidget *StandardPlotCreator::settingsMenu() { return m_plotSettings; }

void StandardPlotCreator::setupPlotChannels()
{
	int i = 0;
	const QList<ExtProcPlotInfo::PlotInfoCh> infoChnls = m_plotInfo.channels;
	for(const auto &plotCh : infoChnls) {
		QString chId = "ch" + QString::number(i);
		QPen pen(StyleHelper::getChannelColor(i));
		auto ch = new PlotChannel(chId, pen, m_plotWidget->xAxis(), m_plotWidget->yAxis(), m_plotWidget);
		m_plotWidget->addPlotChannel(ch);
		ch->setEnabled(true);
		m_curveControl->addChannels(ch);
		if(m_plotInfo.flags.contains("points")) {
			ch->setThickness(3);
			ch->setStyle(PlotChannel::PlotCurveStyle::PCS_DOTS);
		}
		i++;
	}
}

void StandardPlotCreator::configurePlotAxis()
{
	// X Axis configuration
	m_plotWidget->xAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	m_plotWidget->xAxis()->scaleDraw()->setFloatPrecision(2);
	if(m_plotInfo.flags.contains("points")) {
		m_plotWidget->xAxis()->setInterval(-200, 200);
	}

	// Y Axis configuration
	m_plotWidget->yAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	m_plotWidget->yAxis()->scaleDraw()->setFloatPrecision(2);
	m_plotWidget->yAxis()->setInterval(-200, 200);
}

void StandardPlotCreator::applyPlotFlags()
{
	// Handle labels flag
	bool showLabels = m_plotInfo.flags.contains("labels");
	m_plotWidget->setShowXAxisLabels(showLabels);
	m_plotWidget->setShowYAxisLabels(showLabels);
	m_plotWidget->showAxisLabels();
}

void StandardPlotCreator::createPlotSettings()
{
	m_plotSettings = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_plotSettings);
	lay->setMargin(0);

	QPushButton *xAutoScaleBtn = new QPushButton("Autoscale X", m_plotSettings);
	Style::setStyle(xAutoScaleBtn, style::properties::button::basicButton);
	connect(xAutoScaleBtn, &QPushButton::pressed, this, &StandardPlotCreator::autoscaleX);

	QPushButton *yAutoScaleBtn = new QPushButton("Autoscale Y", m_plotSettings);
	Style::setStyle(yAutoScaleBtn, style::properties::button::basicButton);
	connect(yAutoScaleBtn, &QPushButton::pressed, this, &StandardPlotCreator::autoscaleY);

	lay->addWidget(createYAxisSection());
	lay->addWidget(createDataManagerSection());
	lay->addWidget(createGeneralSettingsSection());
	lay->addWidget(xAutoScaleBtn);
	lay->addWidget(yAutoScaleBtn);
}

QWidget *StandardPlotCreator::createYAxisSection()
{
	MenuSectionCollapseWidget *yAxis = new MenuSectionCollapseWidget(
		"Y-AXIS", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, m_plotSettings);
	MenuPlotAxisRangeControl *yCtrl = new MenuPlotAxisRangeControl(m_plotWidget->yAxis(), yAxis);
	yCtrl->setMin(m_plotWidget->yAxis()->min());
	yCtrl->setMax(m_plotWidget->yAxis()->max());
	yAxis->add(yCtrl);
	return yAxis;
}

QWidget *StandardPlotCreator::createDataManagerSection()
{
	MenuSectionCollapseWidget *dataManagerMenu = new MenuSectionCollapseWidget(
		"DATA MANAGER", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, m_plotSettings);
	initPlotChnlsTab(dataManagerMenu);
	dataManagerMenu->add(m_chnlsTab);

	return dataManagerMenu;
}

QWidget *StandardPlotCreator::createGeneralSettingsSection()
{
	MenuSectionCollapseWidget *plotMenu = new MenuSectionCollapseWidget(
		"SETTINGS", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, m_plotSettings);

	MenuOnOffSwitch *labelsSwitch = new MenuOnOffSwitch("Show plot labels", plotMenu, false);
	labelsSwitch->onOffswitch()->setChecked(m_plotWidget->showXAxisLabels() && m_plotWidget->showYAxisLabels());
	connect(labelsSwitch->onOffswitch(), &QAbstractButton::toggled, this, [this](bool en) {
		if(en) {
			m_plotWidget->showAxisLabels();
		} else {
			m_plotWidget->hideAxisLabels();
		}
	});
	plotMenu->add(labelsSwitch);
	plotMenu->add(m_curveControl);
	return plotMenu;
}

void StandardPlotCreator::onNewMin(double min)
{
	if(m_plotAutoscaler->xAxisMode()) {
		m_plotWidget->xAxis()->setMin(min);
	} else {
		m_plotWidget->yAxis()->setMin(min);
	}
	m_plotAutoscaler->stop();
}

void StandardPlotCreator::onNewMax(double max)
{
	if(m_plotAutoscaler->xAxisMode()) {
		m_plotWidget->xAxis()->setMax(max);
	} else {
		m_plotWidget->yAxis()->setMax(max);
	}
	m_plotAutoscaler->stop();
}

void StandardPlotCreator::initPlotChnlsTab(QWidget *parent)
{
	m_chnlsTab = new QTabWidget(parent);
	m_chnlsTab->setTabPosition(QTabWidget::South);
	m_chnlsTab->setTabsClosable(true);
	Style::setStyle(m_chnlsTab->tabBar(), style::properties::tabwidget::smallTabItem);

	int iconSize = Style::getDimension(json::global::unit_0_5);
	QPushButton *plusBtn = new QPushButton("");
	plusBtn->setIcon(Style::getPixmap(":/gui/icons/plus.svg", Style::getColor(json::theme::content_inverse)));
	Style::setStyle(plusBtn, style::properties::button::spinboxButton);
	plusBtn->setIconSize(QSize(iconSize, iconSize));
	plusBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(plusBtn, &QPushButton::pressed, this, &StandardPlotCreator::addPlotChannel);

	m_chnlsTab->setCornerWidget(plusBtn, Qt::TopRightCorner);

	const QStringList chnls = getChnlsList();
	updateChnlsTab();
	connect(this, &StandardPlotCreator::plotChnlsUpdated, this, &StandardPlotCreator::updateChnlsTab);
	connect(m_chnlsTab->tabBar(), &QTabBar::tabCloseRequested, this, &StandardPlotCreator::rmPlotChannel);
}

void StandardPlotCreator::updateChnlsTab()
{
	if(!m_chnlsTab) {
		return;
	}
	QStringList chnls = getChnlsList();
	for(int i = 0; i < m_chnlsTab->count(); i++) {
		m_chnlsTab->widget(i)->deleteLater();
	}
	m_chnlsTab->clear();
	for(int i = 0; i < chnls.size(); i++) {
		m_chnlsTab->addTab(createTabEntryW(m_chnlsTab, i), chnls[i]);
	}
}

QWidget *StandardPlotCreator::createTabEntryW(QTabWidget *parent, int chnlIdx)
{
	QWidget *w = new QWidget(parent);
	QVBoxLayout *lay = new QVBoxLayout(w);
	lay->setMargin(0);

	QList<ExtProcPlotInfo::PlotInfoCh> infoChnls = m_plotInfo.channels;
	MenuCombo *xDataCb = new MenuCombo("X Values");
	MenuCombo *yDataCb = new MenuCombo("Y Values");
	xDataCb->combo()->addItems(m_dmEntries);
	yDataCb->combo()->addItems(m_dmEntries);

	if(chnlIdx >= 0 && chnlIdx < infoChnls.size()) {
		xDataCb->combo()->setCurrentText(infoChnls[chnlIdx].x);
		yDataCb->combo()->setCurrentText(infoChnls[chnlIdx].y);
	}

	lay->addWidget(xDataCb);
	lay->addWidget(yDataCb);

	connect(this, &StandardPlotCreator::dataManagerEntries, this,
		[xDataCb, yDataCb, chnlIdx, this](const QStringList &entries) {
			m_dmEntries = entries;
			updateOnDmEntries(xDataCb->combo(), yDataCb->combo(), chnlIdx);
		});
	connect(yDataCb->combo(), &QComboBox::currentTextChanged, this, [this, parent](const QString &text) {
		int crtIdx = parent->currentIndex();
		if(crtIdx >= 0 && crtIdx < m_plotInfo.channels.size()) {
			m_plotInfo.channels[crtIdx].y = text;
			autoscaleY();
		}
	});
	connect(xDataCb->combo(), &QComboBox::currentTextChanged, this, [this, parent](const QString &text) {
		int crtIdx = parent->currentIndex();
		if(crtIdx >= 0 && crtIdx < m_plotInfo.channels.size()) {
			m_plotInfo.channels[crtIdx].x = text;
			autoscaleX();
		}
	});

	return w;
}

void StandardPlotCreator::updateOnDmEntries(QComboBox *xCombo, QComboBox *yCombo, int chnlIdx)
{
	xCombo->blockSignals(true);
	yCombo->blockSignals(true);

	xCombo->clear();
	xCombo->addItems(m_dmEntries);

	yCombo->clear();
	yCombo->addItems(m_dmEntries);

	if(chnlIdx >= 0 && chnlIdx < m_plotInfo.channels.size()) {
		xCombo->setCurrentText(m_plotInfo.channels[chnlIdx].x);
		yCombo->setCurrentText(m_plotInfo.channels[chnlIdx].y);
	}
	xCombo->blockSignals(false);
	yCombo->blockSignals(false);
}

void StandardPlotCreator::updatePlotChannels(const ExtProcPlotInfo &plotInfo)
{
	m_plotInfo = plotInfo;
	auto plotChnls = m_plotWidget->getChannels();
	if(m_plotInfo.channels.size() == plotChnls.size()) {
		return;
	}
	clearPlotChannels();
	setupPlotChannels();

	Q_EMIT plotChnlsUpdated(getChnlsList());
}

void StandardPlotCreator::clearPlotChannels()
{
	const auto channels = m_plotWidget->getChannels();
	for(auto ch : channels) {
		m_curveControl->removeChannels(ch);
		m_plotWidget->removePlotChannel(ch);
		ch->deleteLater();
	}
	m_plotWidget->replot();
}

QStringList StandardPlotCreator::getChnlsList()
{
	QStringList plotChnls;
	const QList<PlotChannel *> chnls = m_plotWidget->getChannels();
	for(PlotChannel *ch : chnls) {
		plotChnls.append(ch->name());
	}
	return plotChnls;
}

void StandardPlotCreator::autoscaleX()
{
	m_plotAutoscaler->setXAxisMode(true);
	m_plotAutoscaler->start();
}

void StandardPlotCreator::autoscaleY()
{
	m_plotAutoscaler->setXAxisMode(false);
	m_plotAutoscaler->start();
}

void StandardPlotCreator::addPlotChannel()
{
	QString chData = (m_dmEntries.size() > 0) ? m_dmEntries.first() : DataManagerKeys::TIME;
	ExtProcPlotInfo newPlotInfo = m_plotInfo;
	ExtProcPlotInfo::PlotInfoCh infoCh{chData, chData};
	newPlotInfo.channels.append(infoCh);
	updatePlotChannels(newPlotInfo);
}

void StandardPlotCreator::rmPlotChannel(int chnlIdx)
{
	if(!m_chnlsTab) {
		return;
	}
	ExtProcPlotInfo newPlotInfo = m_plotInfo;
	if(chnlIdx < 0 || chnlIdx >= newPlotInfo.channels.size()) {
		return;
	}
	newPlotInfo.channels.removeAt(chnlIdx);
	updatePlotChannels(newPlotInfo);
}

void StandardPlotCreator::updatePlot(const QMap<QString, QVector<double>> &data)
{
	if(m_first) {
		m_plotAutoscaler->start();
	}
	updatePlotChannels(m_plotInfo);
	const auto &channels = m_plotInfo.channels;
	auto plotChnls = m_plotWidget->getChannels();
	double xFirst = 0, xLast = 0;
	for(int chIdx = 0; chIdx < channels.size() && chIdx < plotChnls.size(); chIdx++) {
		const auto &ch = channels[chIdx];
		auto xData = data.value(ch.x);
		auto yData = data.value(ch.y);

		plotChnls[chIdx]->curve()->setSamples(xData, yData);

		QVector<float> xDataFloat = QVector<float>(xData.begin(), xData.end());
		QVector<float> yDataFloat = QVector<float>(yData.begin(), yData.end());
		m_plotAutoscaler->onNewData(xDataFloat.data(), yDataFloat.data(), xDataFloat.size(), false);
	}
	m_plotWidget->replot();
	m_first = false;
}

void StandardPlotCreator::enableChannelAdd(bool en)
{
	m_chnlsTab->cornerWidget()->setVisible(en);
	m_chnlsTab->setTabsClosable(en);
}

PlotCreatorBase *PlotCreatorFactory::createPlotCreator(const ExtProcPlotInfo &plotInfo)
{
	// This is where the logic needs to be put when multiple plots will be used.
	// Default to standard plot.
	return new StandardPlotCreator(plotInfo);
}

QString PlotCreatorFactory::determinePlotType(const ExtProcPlotInfo &plotInfo)
{
	// Logic to determine plot type based on plotInfo
	if(plotInfo.flags.contains("spectrogram")) {
		return "spectrogram";
	}

	return "plotWidget";
}
