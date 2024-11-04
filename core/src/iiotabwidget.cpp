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

#include "iiotabwidget.h"

#include "iioutil/scopy-iioutil_config.h"
#include "menusectionwidget.h"
#include "qtconcurrentrun.h"

#include <iio.h>
#include <QLoggingCategory>
#include <style.h>
#include <stylehelper.h>
#include <iioutil/iioscantask.h>

Q_LOGGING_CATEGORY(CAT_IIO_ADD_PAGE, "IIOTabWidget")

using namespace scopy;

IioTabWidget::IioTabWidget(QWidget *parent)
	: QWidget(parent)
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setSpacing(10);
	setLayout(layout);

	QWidget *contentWidget = new QWidget(this);
	QVBoxLayout *contentLay = new QVBoxLayout(this);
	contentLay->setSpacing(10);
	contentLay->setMargin(0);
	contentWidget->setLayout(contentLay);

	QStringList backendsList = computeBackendsList();

	MenuSectionCollapseWidget *scanSection = new MenuSectionCollapseWidget(
		"SCAN", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, contentWidget);
	scanSection->menuSection()->layout()->setMargin(0);

	QWidget *scanWidget = new QWidget(scanSection);
	QGridLayout *scanGrid = new QGridLayout(scanWidget);
	scanGrid->setMargin(0);
	scanWidget->setLayout(scanGrid);

	m_filterWidget = createFilterWidget(scanWidget);
	scanGrid->addWidget(new QLabel("Filter", scanWidget), 0, 0);
	scanGrid->addWidget(m_filterWidget, 0, 1);
	setupFilterWidget(backendsList);

	QWidget *avlContextWidget = createAvlCtxWidget(scanWidget);
	m_btnScan->setVisible(!backendsList.isEmpty());
	m_ctxUriLabel = new QLabel(scanWidget);
	m_ctxUriLabel->setVisible(false);
	scanGrid->addWidget(new QLabel("Context", scanWidget), 1, 0);
	scanGrid->addWidget(avlContextWidget, 1, 1);
	scanGrid->addWidget(m_ctxUriLabel, 2, 1);
	scanSection->add(scanWidget);

	QWidget *scanContainer = new QWidget(parent);
	scanContainer->setLayout(new QGridLayout(scanContainer));
	Style::setStyle(scanContainer, style::properties::widget::border_interactive);
	scanContainer->layout()->addWidget(scanSection);

	contentLay->addWidget(scanContainer);

	MenuSectionCollapseWidget *serialSection = new MenuSectionCollapseWidget(
		"SERIAL PORT", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, contentWidget);
	serialSection->menuSection()->layout()->setMargin(0);

	QWidget *serialSettWidget = createSerialSettWidget(serialSection);
	bool serialCompatible = isSerialCompatible();
	serialSettWidget->setEnabled(serialCompatible);
	serialSection->add(serialSettWidget);

	QWidget *serialContainer = new QWidget(parent);
	serialContainer->setLayout(new QGridLayout(serialContainer));
	Style::setStyle(serialContainer, style::properties::widget::border_interactive);
	serialContainer->layout()->addWidget(serialSection);

	contentLay->addWidget(serialContainer);

	QWidget *uriWidget = createUriWidget(contentWidget);
	contentLay->addWidget(uriWidget);
	contentLay->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	layout->addWidget(contentWidget);
	layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	addScanFeedbackMsg("No scanned devices... Press the refresh button!");

	m_fwScan = new QFutureWatcher<int>(this);
	m_fwSerialScan = new QFutureWatcher<QVector<QString>>(this);
	setupConnections();
	if(serialCompatible)
		futureSerialScan();
}

IioTabWidget::~IioTabWidget() {}

void IioTabWidget::setupConnections()
{
	connect(m_btnVerify, &QPushButton::clicked, this, &IioTabWidget::verifyBtnClicked, Qt::QueuedConnection);
	// scanfilterLayout
	connect(m_fwScan, &QFutureWatcher<int>::started, m_btnScan, &AnimationPushButton::startAnimation,
		Qt::QueuedConnection);
	connect(m_fwScan, &QFutureWatcher<int>::finished, this, &IioTabWidget::scanFinished, Qt::QueuedConnection);
	connect(m_btnScan, SIGNAL(clicked()), this, SLOT(futureScan()), Qt::QueuedConnection);

	connect(m_avlCtxCb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
		if(idx >= 0 && idx < m_scanList.size()) {
			m_ctxUriLabel->setText(m_scanList[idx].second);
			Q_EMIT uriChanged(m_scanList[idx].second);
		}
	});

	// serial scan
	connect(m_fwSerialScan, &QFutureWatcher<int>::started, m_btnSerialScan, &AnimationPushButton::startAnimation,
		Qt::QueuedConnection);
	connect(m_fwSerialScan, &QFutureWatcher<int>::finished, this, &IioTabWidget::serialScanFinished,
		Qt::QueuedConnection);
	connect(m_btnSerialScan, SIGNAL(clicked()), this, SLOT(futureSerialScan()), Qt::QueuedConnection);
	// serial widget connections
	connect(m_serialPortCb->combo(), &QComboBox::textActivated, this,
		[=]() { Q_EMIT uriChanged(getSerialPath()); });
	connect(m_baudRateCb->combo(), &QComboBox::textActivated, this, [=]() { Q_EMIT uriChanged(getSerialPath()); });
	connect(m_serialFrameEdit, &QLineEdit::returnPressed, this, [=]() { Q_EMIT uriChanged(getSerialPath()); });
	connect(this, &IioTabWidget::uriChanged, this, &IioTabWidget::updateUri);
	connect(m_uriEdit, &QLineEdit::returnPressed, this, [=]() { Q_EMIT m_btnVerify->clicked(); });
	connect(m_uriEdit, &QLineEdit::textChanged, this,
		[=](QString uri) { m_btnVerify->setEnabled(!uri.isEmpty()); });
}

QStringList IioTabWidget::computeBackendsList()
{
	QStringList list;
	int backEndsCount = iio_get_backends_count();
	for(int i = 0; i < backEndsCount; i++) {
		QString backEnd(iio_get_backend(i));
		if(backEnd.compare("xml") == 0 || backEnd.compare("serial") == 0) {
			continue;
		}
		list.append(backEnd);
	}
	return list;
}

QCheckBox *IioTabWidget::createBackendCheckBox(QString backEnd, QWidget *parent)
{
	QCheckBox *cb = new QCheckBox(backEnd, m_filterWidget);
	cb->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	connect(cb, &QCheckBox::toggled, this, [=](bool en) {
		if(en) {
			m_scanParamsList.push_back(backEnd + ":");
		} else {
			m_scanParamsList.removeOne(backEnd + ":");
		}
		futureScan();
		m_btnScan->setFocus();
	});
	return cb;
}

void IioTabWidget::setupFilterWidget(QStringList backednsList)
{
	QHBoxLayout *filterLayout = dynamic_cast<QHBoxLayout *>(m_filterWidget->layout());
	for(const QString &backend : backednsList) {
		QCheckBox *cb = createBackendCheckBox(backend, m_filterWidget);
		filterLayout->addWidget(cb, 0, Qt::AlignLeft);
	}
	filterLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
}

void IioTabWidget::verifyBtnClicked()
{
	QRegExp ipRegex("^(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-"
			"4][0-9]|25[0-5])$");
	QString uri(m_uriEdit->text());
	bool isIp = uri.contains(ipRegex);
	if(isIp && !m_uriEdit->text().contains("ip:")) {
		m_uriEdit->blockSignals(true);
		m_uriEdit->setText("ip:" + uri);
		m_uriEdit->blockSignals(false);
	}
	m_btnScan->setDisabled(true);
	m_btnSerialScan->setDisabled(true);
	m_btnVerify->startAnimation();
	Q_EMIT startVerify(m_uriEdit->text(), "iio");
}

void IioTabWidget::onVerifyFinished(bool result)
{
	rstUriMsgLabel();
	if(!result) {
		m_uriMsgLabel->setVisible(true);
		m_uriMsgLabel->setText("\"" + m_uriEdit->text() + "\" not a valid context!");
	}
	m_btnVerify->stopAnimation();
	m_btnScan->setEnabled(true);
	m_btnSerialScan->setEnabled(true);
}

void IioTabWidget::futureScan()
{
	m_scanList.clear();
	QString scanParams = m_scanParamsList.join("");
	QFuture<int> f = QtConcurrent::run(std::bind(&IIOScanTask::scan, &m_scanList, scanParams));
	m_fwScan->setFuture(f);
}

void IioTabWidget::futureSerialScan()
{
	QFuture<QVector<QString>> f = QtConcurrent::run(std::bind(&IIOScanTask::getSerialPortsName));
	m_fwSerialScan->setFuture(f);
}

void IioTabWidget::scanFinished()
{
	int retCode = m_fwScan->result();
	m_btnScan->stopAnimation();
	m_avlCtxCb->clear();
	m_ctxUriLabel->clear();
	rstUriMsgLabel();
	if(retCode < 0) {
		m_ctxUriLabel->setVisible(false);
		addScanFeedbackMsg("Scan command failed!");
		qWarning(CAT_IIO_ADD_PAGE) << "iio_scan_context_get_info_list error " << retCode;
		return;
	}
	if(m_scanList.isEmpty()) {
		m_ctxUriLabel->setVisible(false);
		addScanFeedbackMsg("No scanned devices available!");
		return;
	}
	if(!m_avlCtxCb->isEnabled()) {
		m_avlCtxCb->setEnabled(true);
	}
	for(const auto &ctx : qAsConst(m_scanList)) {
		QString cbEntry = ctx.first + " [" + ctx.second + "]";
		m_avlCtxCb->addItem(cbEntry);
	}
	int crtIdx = m_avlCtxCb->currentIndex();
	m_ctxUriLabel->setVisible(true);
	m_ctxUriLabel->setText(m_scanList[crtIdx].second);
	updateUri(m_scanList[crtIdx].second);
}

void IioTabWidget::serialScanFinished()
{
	QVector<QString> portsName = m_fwSerialScan->result();
	m_btnSerialScan->stopAnimation();
	m_serialPortCb->combo()->clear();
	if(!portsName.empty()) {
		for(const QString &port : portsName) {
			m_serialPortCb->combo()->addItem(port);
		}
	}
}

QString IioTabWidget::getSerialPath()
{
	QString serialPath = "serial:";
	serialPath.append(m_serialPortCb->combo()->currentText());
	serialPath.append("," + m_baudRateCb->combo()->currentText());
	serialPath.append("," + m_serialFrameEdit->text());
	return serialPath;
}

bool IioTabWidget::isSerialCompatible()
{
	bool hasLibSerialPort = false;
#ifdef WITH_LIBSERIALPORT
	hasLibSerialPort = true;
#endif
	bool hasSerialBackend = iio_has_backend("serial");
	return hasLibSerialPort && hasSerialBackend;
}

void IioTabWidget::updateUri(QString uri)
{
	m_uriEdit->clear();
	m_uriEdit->setText(uri);
	if(!uri.isEmpty()) {
		m_btnVerify->setFocus();
	}
}

void IioTabWidget::addScanFeedbackMsg(QString message)
{
	m_ctxUriLabel->clear();
	m_avlCtxCb->clear();
	m_avlCtxCb->addItem(message);
	m_avlCtxCb->setEnabled(false);
	updateUri("");
}

void IioTabWidget::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	m_btnScan->setFocus();
}

void IioTabWidget::setupBtnLdIcon(AnimationPushButton *btn)
{
	QMovie *icon(new QMovie(this));
	icon->setFileName(":/gui/loading.gif");
	btn->setAnimation(icon);
}

void IioTabWidget::rstUriMsgLabel()
{
	m_uriMsgLabel->setVisible(false);
	m_uriMsgLabel->clear();
}

QWidget *IioTabWidget::createFilterWidget(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QHBoxLayout *layout = new QHBoxLayout(w);
	layout->setMargin(0);
	layout->setSpacing(10);
	w->setLayout(layout);
	return w;
}

QWidget *IioTabWidget::createAvlCtxWidget(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QHBoxLayout *layout = new QHBoxLayout(w);
	layout->setMargin(0);
	layout->setSpacing(10);
	w->setLayout(layout);

	m_avlCtxCb = new QComboBox(w);

	m_btnScan = new AnimationPushButton(w);
	setupBtnLdIcon(m_btnScan);
	StyleHelper::RefreshButton(m_btnScan);
	m_btnScan->setAutoDefault(true);

	layout->addWidget(m_avlCtxCb);
	layout->addWidget(m_btnScan);
	return w;
}

QWidget *IioTabWidget::createSerialSettWidget(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QHBoxLayout *layout = new QHBoxLayout(w);
	layout->setMargin(0);
	layout->setSpacing(10);
	w->setLayout(layout);

	m_serialPortCb = new MenuCombo("Name", w);
	m_baudRateCb = new MenuCombo("Baud Rate", w);
	for(int baudRate : m_availableBaudRates) {
		m_baudRateCb->combo()->addItem(QString::number(baudRate));
	}

	QWidget *lineEditWidget = new QWidget(w);
	lineEditWidget->setLayout(new QVBoxLayout(lineEditWidget));
	lineEditWidget->layout()->setMargin(0);
	lineEditWidget->layout()->setSpacing(0);
	QLabel *serialFrameLabel = new QLabel("Config", lineEditWidget);

	QRegExp re("[5-9]{1}(n|o|e|m|s){1}[1-2]{1}(x|r|d){0,1}$");
	QRegExpValidator *validator = new QRegExpValidator(re, this);
	m_serialFrameEdit = new QLineEdit(lineEditWidget);
	m_serialFrameEdit->setValidator(validator);
	m_serialFrameEdit->setText("8n1");
	m_serialFrameEdit->setFocusPolicy(Qt::ClickFocus);

	lineEditWidget->layout()->addWidget(serialFrameLabel);
	lineEditWidget->layout()->addWidget(m_serialFrameEdit);
	lineEditWidget->setFixedWidth(serialFrameLabel->width());

	m_btnSerialScan = new AnimationPushButton(w);
	setupBtnLdIcon(m_btnSerialScan);
	StyleHelper::RefreshButton(m_btnSerialScan);

	layout->addWidget(m_serialPortCb);
	layout->addWidget(m_baudRateCb);
	layout->addWidget(lineEditWidget);
	layout->addWidget(m_btnSerialScan);
	return w;
}

QWidget *IioTabWidget::createUriWidget(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QGridLayout *layout = new QGridLayout(w);
	w->setLayout(layout);
	Style::setStyle(w, style::properties::widget::border_interactive);

	QLabel *uriLabel = new QLabel("URI", w);
	StyleHelper::MenuLargeLabel(uriLabel);

	m_uriEdit = new QLineEdit(w);
	m_uriEdit->setPlaceholderText("The device you are connecting to");
	m_uriEdit->setFocusPolicy(Qt::ClickFocus);
	m_uriMsgLabel = new QLabel(w);
	m_uriMsgLabel->setVisible(false);

	QWidget *btnVerifyWidget = createVerifyBtnWidget(w);

	layout->addWidget(uriLabel, 0, 0);
	layout->addWidget(m_uriEdit, 0, 1);
	layout->addWidget(btnVerifyWidget, 0, 2, Qt::AlignRight);
	layout->addWidget(m_uriMsgLabel, 1, 1);

	return w;
}

QWidget *IioTabWidget::createVerifyBtnWidget(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QHBoxLayout *layout = new QHBoxLayout(w);
	layout->setMargin(0);
	layout->setAlignment(Qt::AlignRight);
	w->setLayout(layout);

	m_btnVerify = new AnimationPushButton(w);
	setupBtnLdIcon(m_btnVerify);
	StyleHelper::BasicButton(m_btnVerify);
	m_btnVerify->setText("Verify");
	m_btnVerify->setIconSize(QSize(30, 30));
	m_btnVerify->setFixedWidth(128);
	m_btnVerify->setEnabled(false);
	m_btnVerify->setAutoDefault(true);

	layout->addWidget(m_btnVerify);
	return w;
}

#include "moc_iiotabwidget.cpp"
