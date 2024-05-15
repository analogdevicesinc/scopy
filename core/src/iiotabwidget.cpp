#include "iiotabwidget.h"

#include "iioutil/scopy-iioutil_config.h"
#include "qtconcurrentrun.h"

#include <iio.h>

#include <QCheckBox>
#include <QLoggingCategory>
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

	QWidget *gridWidget = new QWidget(this);
	QGridLayout *gridLay = new QGridLayout(this);
	gridLay->setSpacing(10);
	gridLay->setMargin(0);
	gridWidget->setLayout(gridLay);

	QStringList backendsList = computeBackendsList();

	QLabel *filterLabel = new QLabel("Filter:", gridWidget);
	StyleHelper::MenuSmallLabel(filterLabel);
	m_filterWidget = createFilterWidget(gridWidget);
	setupFilterWidget(backendsList);
	gridLay->addWidget(filterLabel, 0, 0);
	gridLay->addWidget(m_filterWidget, 0, 1);

	QLabel *ctxLabel = new QLabel("Context:", gridWidget);
	StyleHelper::MenuSmallLabel(ctxLabel);
	QWidget *avlContextWidget = createAvlCtxWidget(gridWidget);
	m_btnScan->setVisible(!backendsList.isEmpty());
	gridLay->addWidget(ctxLabel, 1, 0);
	gridLay->addWidget(avlContextWidget, 1, 1);

	QLabel *serialLabel = new QLabel("Serial:", gridWidget);
	StyleHelper::MenuSmallLabel(serialLabel);
	QWidget *serialSettWiedget = createSerialSettWidget(gridWidget);
	bool serialCompatible = isSerialCompatible();
	serialSettWiedget->setEnabled(serialCompatible);
	gridLay->addWidget(serialLabel, 2, 0);
	gridLay->addWidget(serialSettWiedget, 2, 1);

	QLabel *uriLabel = new QLabel("URI:", gridWidget);
	StyleHelper::MenuSmallLabel(uriLabel);
	QWidget *uriWidget = createUriWidget(gridWidget);
	gridLay->addWidget(uriLabel, 3, 0);
	gridLay->addWidget(uriWidget, 3, 1);

	QWidget *btnVerifyWidget = createVerifyBtnWidget(gridWidget);
	gridLay->addWidget(btnVerifyWidget, 4, 1, Qt::AlignRight);

	gridLay->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 5, 0);

	layout->addWidget(gridWidget);
	layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	addScanFeedbackMsg("No scanned contexts... Press the refresh button!");

	m_fwScan = new QFutureWatcher<int>(this);
	m_fwSerialScan = new QFutureWatcher<QVector<QString>>(this);
	setupConnections();
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

	connect(m_avlCtxCb, &QComboBox::textActivated, this, [=]() { Q_EMIT uriChanged(m_avlCtxCb->currentText()); });
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
	connect(m_serialFrameEdit->edit(), &QLineEdit::returnPressed, this,
		[=]() { Q_EMIT uriChanged(getSerialPath()); });
	connect(this, &IioTabWidget::uriChanged, this, &IioTabWidget::updateUri);
	connect(m_uriEdit->edit(), &QLineEdit::returnPressed, this, [=]() { Q_EMIT m_btnVerify->clicked(); });
	connect(m_uriEdit->edit(), &QLineEdit::textChanged, this,
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
	QString uri(m_uriEdit->edit()->text());
	bool isIp = uri.contains(ipRegex);
	if(isIp && !m_uriEdit->edit()->text().contains("ip:")) {
		m_uriEdit->edit()->blockSignals(true);
		m_uriEdit->edit()->setText("ip:" + uri);
		m_uriEdit->edit()->blockSignals(false);
	}
	m_btnScan->setDisabled(true);
	m_btnSerialScan->setDisabled(true);
	m_btnVerify->startAnimation();
	Q_EMIT startVerify(m_uriEdit->edit()->text(), "iio");
}

void IioTabWidget::onVerifyFinished(bool result)
{
	m_uriMsgLabel->clear();
	if(!result) {
		m_uriMsgLabel->setText("\"" + m_uriEdit->edit()->text() + "\" not a valid context!");
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
	m_uriMsgLabel->clear();
	if(retCode < 0) {
		addScanFeedbackMsg("Scan command failed!");
		qWarning(CAT_IIO_ADD_PAGE) << "iio_scan_context_get_info_list error " << retCode;
		return;
	}
	if(m_scanList.isEmpty()) {
		addScanFeedbackMsg("No scanned contexts available!");
		return;
	}
	if(!m_avlCtxCb->isEnabled()) {
		m_avlCtxCb->setEnabled(true);
	}
	for(const auto &ctx : qAsConst(m_scanList)) {
		m_avlCtxCb->addItem(ctx);
	}
	updateUri(m_avlCtxCb->currentText());
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
	serialPath.append("," + m_serialFrameEdit->edit()->text());
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
	m_uriEdit->edit()->clear();
	m_uriEdit->edit()->setText(uri);
	if(!uri.isEmpty()) {
		m_btnVerify->setFocus();
	}
}

void IioTabWidget::addScanFeedbackMsg(QString message)
{
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
	StyleHelper::MenuComboBox(m_avlCtxCb, "ctx_combo");

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

	m_serialPortCb = new MenuCombo("Port name", w);
	m_baudRateCb = new MenuCombo("Baud rate", w);
	for(int baudRate : m_availableBaudRates) {
		m_baudRateCb->combo()->addItem(QString::number(baudRate));
	}

	QWidget *lineEditWidget = new QWidget(w);
	lineEditWidget->setLayout(new QVBoxLayout(lineEditWidget));
	lineEditWidget->layout()->setMargin(0);
	lineEditWidget->layout()->setSpacing(3);
	QLabel *serialFrameLabel = new QLabel("Port config", lineEditWidget);
	StyleHelper::MenuComboLabel(serialFrameLabel);

	QRegExp re("[5-9]{1}(n|o|e|m|s){1}[1-2]{1}(x|r|d){0,1}$");
	QRegExpValidator *validator = new QRegExpValidator(re, this);
	m_serialFrameEdit = new MenuLineEdit(lineEditWidget);
	m_serialFrameEdit->edit()->setValidator(validator);
	m_serialFrameEdit->edit()->setText("8n1");
	m_serialFrameEdit->edit()->setFocusPolicy(Qt::ClickFocus);

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
	QHBoxLayout *layout = new QHBoxLayout(w);
	layout->setMargin(0);
	layout->setSpacing(10);
	w->setLayout(layout);

	QWidget *msgUriWidget = new QWidget(w);
	msgUriWidget->setLayout(new QVBoxLayout(msgUriWidget));
	msgUriWidget->layout()->setMargin(0);
	msgUriWidget->layout()->setSpacing(0);
	m_uriEdit = new MenuLineEdit(msgUriWidget);
	m_uriEdit->edit()->setFocusPolicy(Qt::ClickFocus);
	m_uriMsgLabel = new QLabel(msgUriWidget);
	msgUriWidget->layout()->addWidget(m_uriEdit);
	msgUriWidget->layout()->addWidget(m_uriMsgLabel);

	layout->addWidget(msgUriWidget);
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
	StyleHelper::BlueButton(m_btnVerify);
	m_btnVerify->setText("Verify");
	m_btnVerify->setIconSize(QSize(30, 30));
	m_btnVerify->setFixedWidth(128);
	m_btnVerify->setEnabled(false);
	m_btnVerify->setAutoDefault(true);

	layout->addWidget(m_btnVerify);
	return w;
}

#include "moc_iiotabwidget.cpp"
