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

#include "ad9084.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QFileDialog>
#include <QTextStream>

#include <menusectionwidget.h>
#include <menucollapsesection.h>
#include <style.h>
#include <preferenceshelper.h>
#include <iio-widgets/iiowidgetbuilder.h>

#define NUM_MAX_CHANNEL 8

using namespace scopy;
using namespace scopy::ad9084;

Ad9084::Ad9084(struct iio_device *dev, QWidget *parent)
	: QWidget(parent)
	, m_device(dev)
{
	QHBoxLayout *lay = new QHBoxLayout();
	this->setLayout(lay);
	lay->setMargin(0);

	tool = new ToolTemplate(this);
	tool->topContainer()->setVisible(true);
	tool->topContainerMenuControl()->setVisible(false);
	tool->bottomContainer()->setVisible(false);
	tool->rightContainer()->setVisible(true);
	tool->leftContainer()->setVisible(false);
	tool->topCentral()->setVisible(false);
	tool->centralContainer()->setVisible(true);
	tool->setRightContainerWidth(300);
	lay->addWidget(tool);

	settingsBtn = new GearBtn(this);
	settingsBtn->setCheckable(true);
	settingsBtn->setChecked(true);

	connect(settingsBtn, &QPushButton::toggled, this, [this](bool b) {
		tool->openRightContainerHelper(b);
		tool->requestMenu(settingsMenuId);
	});

	m_hSplitter = new QSplitter(Qt::Horizontal, this);

	// Setup device name header
	m_rxChain = new QPushButton(this);
	m_rxChain->setText("RX");
	m_rxChain->setCheckable(true);
	m_rxChain->setChecked(true);
	Style::setStyle(m_rxChain, style::properties::button::squareIconButton);

	m_txChain = new QPushButton(this);
	m_txChain->setText("TX");
	m_txChain->setCheckable(true);
	m_txChain->setChecked(true);
	Style::setStyle(m_txChain, style::properties::button::squareIconButton);

	m_deviceName = new QPushButton(this);
	m_deviceName->setText(iio_device_get_name(m_device));
	Style::setStyle(m_deviceName, style::properties::button::basicButtonBig);
	m_deviceName->setCheckable(false);
	m_deviceName->setChecked(true);

	m_refreshBtn = new AnimationPushButton(this);
	m_refreshBtn->setIcon(
		Style::getPixmap(":/gui/icons/refresh.svg", Style::getColor(json::theme::content_inverse)));
	m_refreshBtn->setIconSize(QSize(25, 25));
	m_refreshBtn->setText("Refresh");
	m_refreshBtn->setAutoDefault(true);
	Style::setStyle(m_refreshBtn, style::properties::button::basicButton);
	QMovie *movie = new QMovie(":/gui/loading.gif");
	m_refreshBtn->setAnimation(movie, 20000);

	// RX chain scrollable container
	QLabel *globalRxLbl = new QLabel("RX Chain");
	QScrollArea *rxScroll = new QScrollArea(m_hSplitter);
	MenuSectionWidget *globalRxSection = new MenuSectionWidget(this);
	QVBoxLayout *layRxScroll = new QVBoxLayout(globalRxSection);
	globalRxSection->contentLayout()->setSpacing(10);

	globalRxSection->setLayout(layRxScroll);
	rxScroll->setWidgetResizable(true);
	rxScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	rxScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	rxScroll->setWidget(globalRxSection);

	// TX chain scrollable container
	QLabel *globalTxLbl = new QLabel("TX Chain");
	QScrollArea *txScroll = new QScrollArea(m_hSplitter);
	MenuSectionWidget *globalTxSection = new MenuSectionWidget(this);
	QVBoxLayout *layTxScroll = new QVBoxLayout(globalTxSection);
	globalTxSection->contentLayout()->setSpacing(10);

	globalTxSection->setLayout(layTxScroll);
	txScroll->setWidgetResizable(true);
	txScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	txScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	txScroll->setWidget(globalTxSection);

	// Add title labels for RX and TX chains
	globalRxSection->contentLayout()->addWidget(globalRxLbl);
	globalTxSection->contentLayout()->addWidget(globalTxLbl);

	Style::setStyle(globalRxLbl, style::properties::label::menuBig);
	Style::setStyle(globalTxLbl, style::properties::label::menuBig);
	Style::setBackgroundColor(txScroll, json::theme::background_subtle);
	Style::setBackgroundColor(rxScroll, json::theme::background_subtle);

	m_hSplitter->addWidget(rxScroll);
	m_hSplitter->addWidget(txScroll);
	m_hSplitter->setStretchFactor(0, 1);
	m_hSplitter->setStretchFactor(1, 1);
	m_hSplitter->setSizes({1, 1});

	connect(m_rxChain, &QPushButton::toggled, this, [=, this](bool toggled) { rxScroll->setVisible(toggled); });
	connect(m_txChain, &QPushButton::toggled, this, [=, this](bool toggled) { txScroll->setVisible(toggled); });
	connect(m_refreshBtn, &QPushButton::clicked, this, [this]() {
		m_refreshBtn->startAnimation();

		QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
		connect(
			watcher, &QFutureWatcher<void>::finished, this,
			[this, watcher]() {
				m_refreshBtn->stopAnimation();
				watcher->deleteLater();
			},
			Qt::QueuedConnection);

		QFuture<void> future = QtConcurrent::run([this]() { Q_EMIT triggerRead(); });

		watcher->setFuture(future);
	});

	// Create right side menu
	QWidget *rightSideMenu = createMenu();

	tool->addWidgetToTopContainerHelper(m_deviceName, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(m_rxChain, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(m_txChain, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(m_refreshBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(settingsBtn, TTA_RIGHT);
	tool->addWidgetToCentralContainerHelper(m_hSplitter);
	tool->rightStack()->add(settingsMenuId, rightSideMenu);

	setupChannels();
	for(auto w : qAsConst(m_channelsRx)) {
		globalRxSection->contentLayout()->addWidget(w);
	}
	for(auto w : qAsConst(m_channelsTx)) {
		globalTxSection->contentLayout()->addWidget(w);
	}
	Q_EMIT triggerRead();
}

Ad9084::~Ad9084() {}

void Ad9084::setupChannels()
{
	for(unsigned int i = 0; i < NUM_MAX_CHANNEL; i++) {
		// RX channels
		QString chnName = QString("voltage%1_i").arg(i);
		struct iio_channel *rxchn = iio_device_find_channel(m_device, chnName.toUtf8(), false);
		if(!rxchn) {
			chnName = QString("voltage%1").arg(i);
			rxchn = iio_device_find_channel(m_device, chnName.toUtf8(), false);
		}

		if(!rxchn) {
			continue;
		}

		bool usefulChn = !!iio_channel_find_attr(rxchn, "channel_nco_frequency");
		if(!usefulChn) {
			continue;
		}

		Ad9084Channel *rxchnWidget = new Ad9084Channel(rxchn, i, this);
		connect(this, &Ad9084::triggerRead, rxchnWidget, &Ad9084Channel::readChannel, Qt::QueuedConnection);
		m_channelsRx.push_back(rxchnWidget);

		// TX channels
		chnName = QString("voltage%1_i").arg(i);
		struct iio_channel *txchn = iio_device_find_channel(m_device, chnName.toUtf8(), true);
		if(!txchn) {
			chnName = QString("voltage%1").arg(i);
			txchn = iio_device_find_channel(m_device, chnName.toUtf8(), true);
		}

		Ad9084Channel *txchnWidget = new Ad9084Channel(txchn, i, this);
		connect(this, &Ad9084::triggerRead, txchnWidget, &Ad9084Channel::readChannel, Qt::QueuedConnection);
		m_channelsTx.push_back(txchnWidget);

		rxchnWidget->init();
		txchnWidget->init();
	}
}

QWidget *Ad9084::createMenu()
{
	// tbd will be replaced with a FileBrowserWidget (PR open)
	QWidget *menu = new QWidget(this);
	QVBoxLayout *vLay = new QVBoxLayout();
	vLay->setMargin(0);
	menu->setLayout(vLay);

	auto pfirContainer = new MenuSectionCollapseWidget("PFIR CONFIG", MenuCollapseSection::MHCW_NONE,
							   MenuCollapseSection::MHW_BASEWIDGET, menu);
	QHBoxLayout *pfirlay = new QHBoxLayout(this);
	pfirlay->setMargin(0);
	pfirlay->setSpacing(10);

	m_pfirLineEdit = new QLineEdit(this);

	QPushButton *pfirBtn = new QPushButton(this);
	StyleHelper::BrowseButton(pfirBtn);

	pfirlay->addWidget(m_pfirLineEdit);
	pfirlay->addWidget(pfirBtn);

	connect(pfirBtn, &QPushButton::pressed, this, &Ad9084::choosePfir);
	connect(m_pfirLineEdit, &QLineEdit::editingFinished, this, &Ad9084::loadPfir);
	Style::setStyle(this, style::properties::widget::border_interactive);
	pfirContainer->contentLayout()->addLayout(pfirlay);

	auto cfirContainer = new MenuSectionCollapseWidget("CFIR CONFIG", MenuCollapseSection::MHCW_NONE,
							   MenuCollapseSection::MHW_BASEWIDGET, menu);
	QHBoxLayout *cfirlay = new QHBoxLayout(this);
	cfirlay->setMargin(0);
	cfirlay->setSpacing(10);

	m_cfirLineEdit = new QLineEdit(this);

	QPushButton *cfirBtn = new QPushButton(this);
	StyleHelper::BrowseButton(cfirBtn);

	cfirlay->addWidget(m_cfirLineEdit);
	cfirlay->addWidget(cfirBtn);

	connect(cfirBtn, &QPushButton::pressed, this, &Ad9084::chooseCfir);
	connect(m_cfirLineEdit, &QLineEdit::editingFinished, this, &Ad9084::loadCfir);
	Style::setStyle(this, style::properties::widget::border_interactive);

	cfirContainer->contentLayout()->addLayout(cfirlay);

	vLay->addWidget(pfirContainer);
	vLay->addWidget(cfirContainer);
	vLay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
	return menu;
}

QString Ad9084::readFile(QString file)
{
	QString content = "";

	QFile f(file);

	if(!f.open(QFile::ReadOnly | QFile::Text)) {
		return content;
	}

	QTextStream in(&f);

	content = in.readAll();
	f.close();
	return content;
}

void Ad9084::loadCfir()
{
	QString path = m_cfirLineEdit->text();
	if(path.isEmpty()) {
		return;
	}
	QString content = readFile(path);
	size_t ret = iio_device_attr_write_raw(m_device, "cfir_config", content.toStdString().c_str(), content.size());
	if(ret < 0)
		qWarning() << "Failed to load CFIR CONFIG file to CFIR_CONFIG attr";
}

void Ad9084::loadPfir()
{
	QString path = m_pfirLineEdit->text();
	if(path.isEmpty()) {
		return;
	}
	QString content = readFile(path);
	size_t ret = iio_device_attr_write_raw(m_device, "pfilt_config", content.toStdString().c_str(), content.size());
	if(ret < 0)
		qWarning() << "Failed to load PFIR CONFIG file to PFILT_CONFIG attr";
}

QString Ad9084::chooseFile()
{
	QString selectedFilter;

	bool useNativeDialogs = Preferences::get("general_use_native_dialogs").toBool();
	QString tmpFilename = QFileDialog::getOpenFileName(
		this, tr("Import"), "", tr("All Files(*)"), &selectedFilter,
		(useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));
	return tmpFilename;
}

void Ad9084::chooseCfir()
{
	QString path = chooseFile();
	m_cfirLineEdit->setText(path);
	Q_EMIT m_cfirLineEdit->editingFinished();
}

void Ad9084::choosePfir()
{
	QString path = chooseFile();
	m_pfirLineEdit->setText(path);
	Q_EMIT m_pfirLineEdit->editingFinished();
}
