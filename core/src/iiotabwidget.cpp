#include "iiotabwidget.h"

#include "iioutil/scopy-iioutil_config.h"
#include "menusectionwidget.h"
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

	QWidget *contentWidget = new QWidget(this);
	QVBoxLayout *contentLay = new QVBoxLayout(this);
	contentLay->setSpacing(10);
	contentLay->setMargin(0);
	contentWidget->setLayout(contentLay);

	QStringList backendsList = computeBackendsList();

	MenuSectionCollapseWidget *scanSection =
		new MenuSectionCollapseWidget("SCAN", MenuCollapseSection::MHCW_ARROW, contentWidget);

	QWidget *scanWidget = new QWidget(scanSection);
	QGridLayout *scanGrid = new QGridLayout(scanWidget);
	scanGrid->setMargin(0);
	scanWidget->setLayout(scanGrid);
	QLabel *filterLabel = new QLabel("Filter", scanWidget);
	StyleHelper::MenuSmallLabel(filterLabel);

	m_filterWidget = createFilterWidget(scanWidget);
	scanGrid->addWidget(filterLabel, 0, 0);
	scanGrid->addWidget(m_filterWidget, 0, 1);
	setupFilterWidget(backendsList);

	QLabel *ctxLabel = new QLabel("Context", scanWidget);
	StyleHelper::MenuSmallLabel(ctxLabel);
	QWidget *avlContextWidget = createAvlCtxWidget(scanWidget);
	m_btnScan->setVisible(!backendsList.isEmpty());
	m_ctxUriLabel = new QLabel(scanWidget);
	m_ctxUriLabel->setVisible(false);
	scanGrid->addWidget(ctxLabel, 1, 0);
	scanGrid->addWidget(avlContextWidget, 1, 1);
	scanGrid->addWidget(m_ctxUriLabel, 2, 1);

	scanSection->add(scanWidget);
	contentLay->addWidget(scanSection);

	MenuSectionCollapseWidget *serialSection =
		new MenuSectionCollapseWidget("SERIAL", MenuCollapseSection::MHCW_ARROW, contentWidget);

	QWidget *serialSettWidget = createSerialSettWidget(serialSection);
	bool serialCompatible = isSerialCompatible();
	serialSettWidget->setEnabled(serialCompatible);
	serialSection->add(serialSettWidget);

	contentLay->addWidget(serialSection);

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
	rstUriMsgLabel();
	if(!result) {
		m_uriMsgLabel->setVisible(true);
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
	QGridLayout *layout = new QGridLayout(w);
	w->setLayout(layout);
	StyleHelper::RoundedCornersWidget(w, "uriAddPage");

	QLabel *uriLabel = new QLabel("URI", w);
	StyleHelper::MenuSmallLabel(uriLabel);

	m_uriEdit = new MenuLineEdit(w);
	m_uriEdit->edit()->setPlaceholderText("The device you are connecting to");
	m_uriEdit->edit()->setFocusPolicy(Qt::ClickFocus);
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
