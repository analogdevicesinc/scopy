/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "backendstabwidget.h"

#include "menusectionwidget.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLoggingCategory>
#include <QMovie>
#include <QVBoxLayout>
#include <QtConcurrent>
#include <style.h>
#include <stylehelper.h>

#include <component/controller.h>

Q_LOGGING_CATEGORY(CAT_BACKENDS_ADD_PAGE, "BackendsTabWidget")

using namespace scopy;

namespace {
QVector<component::ScanResult> scanBackends(const QList<component::BackendId> &ids)
{
	QVector<component::ScanResult> results;
	component::Controller *c = component::Controller::GetInstance();
	for(const component::BackendId &id : ids) {
		std::shared_ptr<component::ContextBackend> backend = c->backend(id);
		if(backend) {
			results.append(backend->scan());
		}
	}
	return results;
}
} // namespace

BackendsTabWidget::BackendsTabWidget(QWidget *parent)
	: QWidget(parent)
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setSpacing(10);

	QWidget *contentWidget = new QWidget(this);
	QVBoxLayout *contentLay = new QVBoxLayout(contentWidget);
	contentLay->setSpacing(10);
	contentLay->setContentsMargins(0, 0, 0, 0);

	contentLay->addWidget(createScanSection(contentWidget));
	contentLay->addWidget(createUriWidget(contentWidget));
	contentLay->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	layout->addWidget(contentWidget);
	layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

	m_fwScan = new QFutureWatcher<QVector<component::ScanResult>>(this);
	connect(m_fwScan, &QFutureWatcher<QVector<component::ScanResult>>::finished, this,
		&BackendsTabWidget::scanFinished, Qt::QueuedConnection);
	connect(m_btnScan, &QPushButton::clicked, this, &BackendsTabWidget::futureScan, Qt::QueuedConnection);
	connect(m_btnVerify, &QPushButton::clicked, this, &BackendsTabWidget::verifyBtnClicked, Qt::QueuedConnection);
	connect(m_resultsCb, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&BackendsTabWidget::selectResult);
	connect(m_uriEdit, &QLineEdit::returnPressed, this, [this]() { Q_EMIT m_btnVerify->clicked(); });
	connect(m_backendCb, &QComboBox::currentTextChanged, this, &BackendsTabWidget::updateUriHint);
	connect(m_uriEdit, &QLineEdit::textChanged, this,
		[this](QString uri) { m_btnVerify->setEnabled(!uri.isEmpty() && m_backendCb->count() > 0); });

	component::Controller *c = component::Controller::GetInstance();
	connect(c, &component::Controller::backendRegistered, this, &BackendsTabWidget::addBackend);
	connect(c, &component::Controller::backendUnregistered, this, &BackendsTabWidget::removeBackend);
	const QList<component::BackendId> ids = c->backends();
	for(const component::BackendId &id : ids) {
		addBackend(id);
	}

	addScanFeedbackMsg("No scanned devices... Press the refresh button!");
}

BackendsTabWidget::~BackendsTabWidget() {}

QWidget *BackendsTabWidget::createScanSection(QWidget *parent)
{
	MenuSectionCollapseWidget *scanSection = new MenuSectionCollapseWidget(
		"SCAN", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, parent);
	scanSection->menuSection()->layout()->setContentsMargins(0, 0, 0, 0);

	QWidget *scanWidget = new QWidget(scanSection);
	QGridLayout *scanGrid = new QGridLayout(scanWidget);
	scanGrid->setContentsMargins(0, 0, 0, 0);

	m_filterWidget = new QWidget(scanWidget);
	QHBoxLayout *filterLay = new QHBoxLayout(m_filterWidget);
	filterLay->setContentsMargins(0, 0, 0, 0);
	filterLay->setSpacing(10);
	filterLay->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

	QWidget *resultsWidget = new QWidget(scanWidget);
	QHBoxLayout *resultsLay = new QHBoxLayout(resultsWidget);
	resultsLay->setContentsMargins(0, 0, 0, 0);
	resultsLay->setSpacing(10);
	m_resultsCb = new QComboBox(resultsWidget);
	m_btnScan = createLoadingButton(resultsWidget);
	StyleHelper::RefreshButton(m_btnScan);
	m_btnScan->setAutoDefault(true);
	resultsLay->addWidget(m_resultsCb);
	resultsLay->addWidget(m_btnScan);

	scanGrid->addWidget(new QLabel("Backends", scanWidget), 0, 0);
	scanGrid->addWidget(m_filterWidget, 0, 1);
	scanGrid->addWidget(new QLabel("Device", scanWidget), 1, 0);
	scanGrid->addWidget(resultsWidget, 1, 1);
	scanSection->add(scanWidget);

	QWidget *scanContainer = new QWidget(parent);
	scanContainer->setLayout(new QGridLayout(scanContainer));
	Style::setStyle(scanContainer, style::properties::widget::border_interactive);
	scanContainer->layout()->addWidget(scanSection);
	return scanContainer;
}

QWidget *BackendsTabWidget::createUriWidget(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QGridLayout *layout = new QGridLayout(w);
	Style::setStyle(w, style::properties::widget::border_interactive);

	QLabel *uriLabel = new QLabel("URI", w);
	Style::setStyle(uriLabel, style::properties::label::menuMedium);

	m_uriEdit = new QLineEdit(w);
	m_uriEdit->setPlaceholderText("The device you are connecting to");
	m_uriEdit->setFocusPolicy(Qt::ClickFocus);

	m_backendCb = new QComboBox(w);
	m_backendCb->setToolTip("Backend used to open the URI");

	m_btnVerify = createLoadingButton(w);
	StyleHelper::BasicButton(m_btnVerify);
	m_btnVerify->setText("Verify");
	m_btnVerify->setIconSize(QSize(30, 30));
	m_btnVerify->setFixedWidth(Style::getDimension(json::global::unit_6));
	m_btnVerify->setEnabled(false);
	m_btnVerify->setAutoDefault(true);

	m_uriMsgLabel = new QLabel(w);
	m_uriMsgLabel->setVisible(false);

	layout->addWidget(uriLabel, 0, 0);
	layout->addWidget(m_uriEdit, 0, 1);
	layout->addWidget(m_backendCb, 0, 2);
	layout->addWidget(m_btnVerify, 0, 3, Qt::AlignRight);
	layout->addWidget(m_uriMsgLabel, 1, 1);
	return w;
}

AnimationPushButton *BackendsTabWidget::createLoadingButton(QWidget *parent)
{
	AnimationPushButton *btn = new AnimationPushButton(parent);
	QMovie *icon = new QMovie(btn);
	icon->setFileName(":/gui/loading.gif");
	btn->setAnimation(icon);
	return btn;
}

void BackendsTabWidget::addBackend(const QString &id)
{
	if(m_filters.contains(id)) {
		return;
	}
	QCheckBox *cb = new QCheckBox(id, m_filterWidget);
	cb->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	cb->setChecked(true);
	connect(cb, &QCheckBox::toggled, this, [this]() {
		futureScan();
		m_btnScan->setFocus();
	});
	// Keep the trailing spacer last.
	QHBoxLayout *lay = static_cast<QHBoxLayout *>(m_filterWidget->layout());
	lay->insertWidget(lay->count() - 1, cb, 0, Qt::AlignLeft);
	m_filters.insert(id, cb);
	m_backendCb->addItem(id);
	m_btnVerify->setEnabled(!m_uriEdit->text().isEmpty());
}

void BackendsTabWidget::updateUriHint(const QString &id)
{
	std::shared_ptr<component::ContextBackend> backend = component::Controller::GetInstance()->backend(id);
	const QString hint = backend ? backend->uriHint() : QString();
	m_uriEdit->setPlaceholderText(hint.isEmpty() ? "The device you are connecting to" : hint);
}

void BackendsTabWidget::removeBackend(const QString &id)
{
	if(QCheckBox *cb = m_filters.take(id)) {
		delete cb;
	}
	m_backendCb->removeItem(m_backendCb->findText(id));
}

void BackendsTabWidget::futureScan()
{
	if(m_fwScan->isRunning()) {
		return;
	}
	QList<component::BackendId> ids;
	for(auto it = m_filters.cbegin(); it != m_filters.cend(); ++it) {
		if(it.value()->isChecked()) {
			ids.append(it.key());
		}
	}
	m_btnScan->startAnimation();
	m_fwScan->setFuture(QtConcurrent::run(scanBackends, ids));
}

void BackendsTabWidget::scanFinished()
{
	m_btnScan->stopAnimation();
	m_results = m_fwScan->result();
	m_uriMsgLabel->setVisible(false);
	if(m_results.isEmpty()) {
		addScanFeedbackMsg("No scanned devices available!");
		return;
	}
	m_resultsCb->blockSignals(true);
	m_resultsCb->clear();
	for(const component::ScanResult &r : std::as_const(m_results)) {
		m_resultsCb->addItem(r.description + " [" + r.uri + "] (" + r.backend + ")");
	}
	m_resultsCb->setEnabled(true);
	m_resultsCb->setCurrentIndex(0);
	m_resultsCb->blockSignals(false);
	selectResult(0);
}

void BackendsTabWidget::selectResult(int idx)
{
	if(idx < 0 || idx >= m_results.size()) {
		return;
	}
	m_backendCb->setCurrentText(m_results[idx].backend);
	updateUri(m_results[idx].uri);
}

void BackendsTabWidget::addScanFeedbackMsg(const QString &message)
{
	m_results.clear();
	m_resultsCb->clear();
	m_resultsCb->addItem(message);
	m_resultsCb->setEnabled(false);
	updateUri("");
}

void BackendsTabWidget::verifyBtnClicked()
{
	if(m_uriEdit->text().isEmpty() || m_backendCb->currentText().isEmpty()) {
		return;
	}
	m_uriMsgLabel->setVisible(false);
	m_btnScan->setDisabled(true);
	m_btnVerify->startAnimation();
	Q_EMIT startVerify(m_uriEdit->text(), m_backendCb->currentText());
}

void BackendsTabWidget::onVerifyFinished(bool result)
{
	if(!result) {
		m_uriMsgLabel->setVisible(true);
		m_uriMsgLabel->setText("\"" + m_uriEdit->text() + "\" could not be opened with the " +
				       m_backendCb->currentText() + " backend!");
	}
	m_btnVerify->stopAnimation();
	m_btnScan->setEnabled(true);
}

void BackendsTabWidget::updateUri(QString uri)
{
	m_uriEdit->setText(uri);
	if(!uri.isEmpty()) {
		m_btnVerify->setFocus();
	}
}

void BackendsTabWidget::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	m_btnScan->setFocus();
}

#include "moc_backendstabwidget.cpp"
