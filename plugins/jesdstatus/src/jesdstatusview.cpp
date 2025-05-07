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
 */

#include "jesdstatusview.h"
#include <style.h>
#include <gui/widgets/menusectionwidget.h>
#include <QVBoxLayout>

using namespace scopy;
using namespace jesdstatus;

JesdStatusView::JesdStatusView(iio_device *dev, QWidget *parent)
	: QWidget(parent)
	, m_parserThread(new QThread())
{
	m_parser = new JesdStatusParser(dev, this);
	m_parser->moveToThread(m_parserThread);

	connect(m_parserThread, &QThread::started, m_parser, &JesdStatusParser::update);
	connect(m_parser, &JesdStatusParser::finished, this, &JesdStatusView::updateUi, Qt::QueuedConnection);
	connect(m_parser, &JesdStatusParser::finished, m_parserThread, &QThread::quit);

	m_colorMap.insert(C_NORM, Style::getAttribute(json::theme::content_default));
	m_colorMap.insert(C_GOOD, Style::getAttribute(json::theme::content_success));
	m_colorMap.insert(C_ERR, Style::getAttribute(json::theme::content_error));
	m_colorMap.insert(C_CRIT, Style::getAttribute(json::theme::content_error));
	m_colorMap.insert(C_OPT, Style::getAttribute(json::theme::content_busy));

	QVBoxLayout *lay = new QVBoxLayout();
	lay->setMargin(0);
	lay->setSpacing(10);
	this->setLayout(lay);

	// Setup STATUS CONTAINER
	QScrollArea *statusScroll = new QScrollArea(this);
	QWidget *statusHeader = new QWidget(statusScroll);
	QWidget *statusContainer = new QWidget(statusScroll);
	QVBoxLayout *statusScrollLayV = new QVBoxLayout();
	QHBoxLayout *statusScrollLayH = new QHBoxLayout();
	statusHeader->setLayout(statusScrollLayV);
	statusContainer->setLayout(statusScrollLayH);
	statusScroll->setWidgetResizable(true);
	statusScroll->setWidget(statusHeader);
	statusScrollLayH->setMargin(0);
	statusScroll->setSizePolicy(statusScroll->sizePolicy().horizontalPolicy(), QSizePolicy::Maximum);

	// Setup status label
	QLabel *statusContainerLbl = new QLabel("STATUS", statusContainer);
	statusScrollLayV->addWidget(statusContainerLbl);
	statusScrollLayV->addWidget(statusContainer);
	statusScrollLayV->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));

	// Setup status style
	Style::setStyle(statusContainerLbl, style::properties::label::menuBig);
	Style::setBackgroundColor(statusHeader, json::theme::background_primary);
	Style::setStyle(statusHeader, style::properties::widget::border_interactive);

	initStatusValues(statusContainer);
	lay->addWidget(statusScroll);

	if(m_parser->getLaneCount() != 0) {
		// Setup LANE STATUS CONTAINER
		QScrollArea *laneScroll = new QScrollArea(this);
		QWidget *laneHeader = new QWidget(laneScroll);
		QWidget *laneContainer = new QWidget(this);
		QVBoxLayout *laneScrollLayV = new QVBoxLayout();
		QHBoxLayout *laneScrollLayH = new QHBoxLayout();
		laneHeader->setLayout(laneScrollLayV);
		laneContainer->setLayout(laneScrollLayH);
		laneScroll->setWidgetResizable(true);
		laneScroll->setWidget(laneHeader);
		laneScrollLayH->setMargin(0);
		//		laneScroll->setSizePolicy(laneScroll->sizePolicy().horizontalPolicy(),
		//QSizePolicy::Maximum);

		// Setup lane status label
		QLabel *laneContainerLbl = new QLabel("LANE STATUS", laneContainer);
		laneScrollLayV->addWidget(laneContainerLbl);
		laneScrollLayV->addWidget(laneContainer);
		laneScrollLayV->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));

		// Setup lane status style
		Style::setStyle(laneContainerLbl, style::properties::label::menuBig);
		Style::setBackgroundColor(laneHeader, json::theme::background_primary);
		Style::setStyle(laneHeader, style::properties::widget::border_interactive);

		initLaneStatusValues(laneContainer);
		lay->addWidget(laneScroll);
	}

	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
}

JesdStatusView::~JesdStatusView()
{
	m_laneLabels.clear();
	m_statusLabels.clear();
}

void JesdStatusView::initStatusValues(QWidget *statusContainer)
{
	auto statusScrollLay = statusContainer->layout();
	// INIT Status LABELS
	MenuSectionWidget *statusLabels = new MenuSectionWidget(statusContainer);
	MenuSectionWidget *statusValues = new MenuSectionWidget(statusContainer);
	statusScrollLay->addWidget(statusLabels);
	statusScrollLay->addWidget(statusValues);
	statusLabels->contentLayout()->setMargin(0);
	statusValues->contentLayout()->setMargin(0);

	appendToStatusLabels(
		"Link is", [this]() { return m_parser->getLinkState(); }, statusLabels, statusValues);
	appendToStatusLabels(
		"Link Status", [this]() { return m_parser->getLinkStatus(); }, statusLabels, statusValues);
	appendToStatusLabels(
		"Measured Link Clock (MHz)", [this]() { return m_parser->getMeasuredLinkClock(); }, statusLabels,
		statusValues);
	appendToStatusLabels(
		"Reported Link Clock (MHz)", [this]() { return m_parser->getReportedLinkClock(); }, statusLabels,
		statusValues);
	appendToStatusLabels(
		"Measured Device Clock (MHz)", [this]() { return m_parser->getMeasuredDeviceClock(); }, statusLabels,
		statusValues);
	appendToStatusLabels(
		"Reported Device Clock (MHz)", [this]() { return m_parser->getReportedDeviceClock(); }, statusLabels,
		statusValues);
	appendToStatusLabels(
		"Desired Device Clock (MHz)", [this]() { return m_parser->getDesiredDeviceClock(); }, statusLabels,
		statusValues);
	appendToStatusLabels(
		"Lane rate (MHz)", [this]() { return m_parser->getLaneRate(); }, statusLabels, statusValues);
	if(m_parser->getEncoder() == JESD204_64B66B) {
		appendToStatusLabels(
			"Lane rate / 66 (MHz)", [this]() { return m_parser->getLaneRateDiv(); }, statusLabels,
			statusValues);
		appendToStatusLabels(
			"LEMC rate (MHz)", [this]() { return m_parser->getLmfcRate(); }, statusLabels, statusValues);
	} else {
		appendToStatusLabels(
			"Lane rate / 40 (MHz)", [this]() { return m_parser->getLaneRateDiv(); }, statusLabels,
			statusValues);
		appendToStatusLabels(
			"LMFC rate (MHz)", [this]() { return m_parser->getLmfcRate(); }, statusLabels, statusValues);
	}
	appendToStatusLabels(
		"SYSREF captured", [this]() { return m_parser->getSysrefCaptured(); }, statusLabels, statusValues);
	appendToStatusLabels(
		"SYSREF alignment error", [this]() { return m_parser->getSysrefAlignmentError(); }, statusLabels,
		statusValues);
	if(m_parser->getEncoder() != JESD204_64B66B) {
		appendToStatusLabels(
			"SYNC~", [this]() { return m_parser->getSyncState(); }, statusLabels, statusValues);
	}
}

void JesdStatusView::initLaneStatusValues(QWidget *laneContainer)
{
	auto laneScrollLay = laneContainer->layout();

	// INIT LANE status LABELS
	MenuSectionWidget *laneLabels = new MenuSectionWidget(laneContainer);
	laneLabels->contentLayout()->setMargin(0);

	QWidget *laneValues = new QWidget(laneContainer);
	QHBoxLayout *laneValuesLay = new QHBoxLayout();
	laneValuesLay->setMargin(0);
	laneValues->setLayout(laneValuesLay);

	laneScrollLay->addWidget(laneLabels);
	laneScrollLay->addWidget(laneValues);

	laneLabels->contentLayout()->addWidget(new QLabel("Lane#", laneLabels));
	laneLabels->contentLayout()->addWidget(new QLabel("Errors", laneLabels));
	if(m_parser->getEncoder() == JESD204_64B66B) {
		laneLabels->contentLayout()->addWidget(new QLabel("Extended multiblock alignment", laneLabels));
	} else {
		laneLabels->contentLayout()->addWidget(new QLabel("Latency (Multiframes/Octets)", laneLabels));
		laneLabels->contentLayout()->addWidget(new QLabel("CGS State", laneLabels));
		laneLabels->contentLayout()->addWidget(new QLabel("Initial Frame Sync", laneLabels));
		laneLabels->contentLayout()->addWidget(new QLabel("Initial Lane Alignment Sequence", laneLabels));
	}

	for(int i = 0; i < m_parser->getLaneCount(); i++) {
		MenuSectionWidget *lane = new MenuSectionWidget(laneValues);
		laneValuesLay->addWidget(lane);
		m_laneLabels.insert(i, {});

		lane->contentLayout()->addWidget(new QLabel(QString::number(i), lane));
		appendToLaneValues(
			i, [this, i](unsigned int) { return m_parser->getErrors(i); }, lane);
		if(m_parser->getEncoder() == JESD204_64B66B) {
			appendToLaneValues(
				i, [this, i](unsigned int) { return m_parser->getExtMultiBlockAlignment(i); }, lane);
		} else {
			appendToLaneValues(
				i, [this, i](unsigned int) { return m_parser->getLaneLatency(i); }, lane);
			appendToLaneValues(
				i, [this, i](unsigned int) { return m_parser->getCgsState(i); }, lane);
			appendToLaneValues(
				i, [this, i](unsigned int) { return m_parser->getInitFrameSync(i); }, lane);
			appendToLaneValues(
				i, [this, i](unsigned int) { return m_parser->getInitLaneAlignSeq(i); }, lane);
		}
	}
}

void JesdStatusView::appendToLaneValues(unsigned int laneIdx,
					std::function<QPair<QString, VISUAL_STATUS>(unsigned int)> cb,
					MenuSectionWidget *valueContainer)
{
	auto lblValue = new QLabel("", valueContainer);
	laneStatusCallback pair;
	pair.first = lblValue;
	pair.second = cb;
	m_laneLabels[laneIdx].push_back(pair);
	valueContainer->contentLayout()->addWidget(lblValue);
}

void JesdStatusView::appendToStatusLabels(QString lbl, std::function<QPair<QString, VISUAL_STATUS>()> cb,
					  MenuSectionWidget *labelContainer, MenuSectionWidget *valueContainer)
{
	auto label = new QLabel(lbl, labelContainer);
	auto lblValue = new QLabel("", valueContainer);
	QPair<QLabel *, std::function<QPair<QString, VISUAL_STATUS>()>> pairValue;
	pairValue.first = lblValue;
	pairValue.second = cb;
	m_statusLabels.insert(label, pairValue);
	labelContainer->contentLayout()->addWidget(label);
	valueContainer->contentLayout()->addWidget(lblValue);
}

void JesdStatusView::update()
{
	if(!m_parserThread->isRunning()) {
		m_parserThread->start();
	}
}

void JesdStatusView::updateUi()
{
	updateStatus();
	updateLaneStatus();
}

void JesdStatusView::updateStatus()
{
	for(auto &pair : qAsConst(m_statusLabels)) {
		QLabel *vLbl = pair.first;
		auto callback = pair.second;
		auto elem = callback();
		vLbl->setText(elem.first);
		vLbl->setStyleSheet(QString("QLabel {color: %1 }").arg(m_colorMap.value(elem.second)));
	}
}

void JesdStatusView::updateLaneStatus()
{
	auto lanes = m_laneLabels.keys();
	for(unsigned int i : lanes) {
		for(auto &pair : m_laneLabels.value(i)) {
			QLabel *vLbl = pair.first;
			auto callback = pair.second;
			auto elem = callback(i);
			vLbl->setText(elem.first);
			vLbl->setStyleSheet(QString("QLabel {color: %1 }").arg(m_colorMap.value(elem.second)));
		}
	}
}
