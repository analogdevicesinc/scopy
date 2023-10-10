#include "iiotabwidget.h"

#include "iioutil/scopy-iioutil_config.h"
#include "qtconcurrentrun.h"

#include "ui_iiotabwidget.h"

#include <iio.h>

#include <QCheckBox>
#include <QLoggingCategory>

#include <iioutil/iioscantask.h>

Q_LOGGING_CATEGORY(CAT_IIO_ADD_PAGE, "IIOTabWidget")

using namespace scopy;

IioTabWidget::IioTabWidget(QWidget *parent)
	: QWidget(parent)
	, m_ui(new Ui::IioTabWidget())
{
	m_ui->setupUi(this);
	init();
	verifyIioBackend();

	connect(m_ui->btnVerify, &QPushButton::clicked, this, &IioTabWidget::verifyBtnClicked, Qt::QueuedConnection);
	// scan
	fwScan = new QFutureWatcher<int>(this);
	connect(fwScan, &QFutureWatcher<int>::started, m_ui->btnScan, &AnimationPushButton::startAnimation,
		Qt::QueuedConnection);
	connect(fwScan, &QFutureWatcher<int>::finished, this, &IioTabWidget::scanFinished, Qt::QueuedConnection);
	connect(m_ui->btnScan, SIGNAL(clicked()), this, SLOT(futureScan()), Qt::QueuedConnection);

	connect(m_ui->comboBoxContexts, &QComboBox::textActivated, this,
		[=]() { Q_EMIT uriChanged(m_ui->comboBoxContexts->currentText()); });

	// serial widget connections
	connect(m_ui->comboBoxSerialPort, &QComboBox::textActivated, this,
		[=]() { Q_EMIT uriChanged(getSerialPath()); });
	connect(m_ui->comboBoxBaudRate, &QComboBox::textActivated, this, [=]() { Q_EMIT uriChanged(getSerialPath()); });
	connect(m_ui->editSerialFrameConfig, &QLineEdit::returnPressed, this,
		[=]() { Q_EMIT uriChanged(getSerialPath()); });
	connect(this, &IioTabWidget::uriChanged, this, &IioTabWidget::updateUri);
	connect(m_ui->editUri, &QLineEdit::returnPressed, this, [=]() { Q_EMIT m_ui->btnVerify->clicked(); });
	connect(m_ui->editUri, &QLineEdit::textChanged, this,
		[=](QString uri) { m_ui->btnVerify->setEnabled(!uri.isEmpty()); });
}

IioTabWidget::~IioTabWidget() { delete m_ui; }

void IioTabWidget::init()
{
	bool hasLibSerialPort = false;
#ifdef WITH_LIBSERIALPORT
	hasLibSerialPort = true;
#endif
	bool serialBackEnd = iio_has_backend("serial");

	QMovie *veifyIcon(new QMovie(this));
	veifyIcon->setFileName(":/gui/loading.gif");
	m_ui->btnVerify->setAnimation(veifyIcon);

	QMovie *scanIcon(new QMovie(this));
	scanIcon->setFileName(":/gui/loading.gif");
	m_ui->btnScan->setAnimation(scanIcon);

	QRegExp re("[5-9]{1}(n|o|e|m|s){1}[1-2]{1}(x|r|d){0,1}$");
	QRegExpValidator *validator = new QRegExpValidator(re, this);

	m_ui->editSerialFrameConfig->setValidator(validator);
	m_ui->serialSettingsWidget->setEnabled(hasLibSerialPort && serialBackEnd);
	m_ui->btnVerify->setProperty("blue_button", QVariant(true));
	m_ui->btnVerify->setEnabled(false);
	m_ui->btnScan->setProperty("blue_button", QVariant(true));
	m_ui->btnScan->setIcon(QIcon(":/gui/icons/refresh.svg"));
	m_ui->btnScan->setIconSize(QSize(25, 25));
	addScanFeedbackMsg("No scanned contexts... Press the refresh button!");
	m_ui->btnScan->setAutoDefault(true);
	m_ui->btnVerify->setAutoDefault(true);
	for(int baudRate : availableBaudRates) {
		m_ui->comboBoxBaudRate->addItem(QString::number(baudRate));
	}
}

void IioTabWidget::verifyIioBackend()
{
	bool scan = false;
	int backEndsCount = iio_get_backends_count();
	bool hasLibSerialPort = false;
#ifdef WITH_LIBSERIALPORT
	hasLibSerialPort = true;
#endif
	for(int i = 0; i < backEndsCount; i++) {
		QString backEnd(iio_get_backend(i));
		if(backEnd.compare("xml") == 0 || (!hasLibSerialPort && backEnd.compare("serial") == 0)) {
			continue;
		}
		createBackEndCheckBox(backEnd);
		scan = true;
	}
	m_ui->btnScan->setVisible(scan);
}

void IioTabWidget::createBackEndCheckBox(QString backEnd)
{
	QCheckBox *cb = new QCheckBox();
	cb->setText(backEnd);
	connect(cb, &QCheckBox::toggled, this, [=](bool en) {
		if(en) {
			scanParamsList.push_back(backEnd + ":");
		} else {
			scanParamsList.removeOne(backEnd + ":");
		}
		if(backEnd.compare("serial") == 0) {
			m_ui->serialSettingsWidget->setEnabled(en);
		}
		if(scanParamsList.empty()) {
			m_ui->btnScan->setEnabled(false);
		} else if(!m_ui->btnScan->isEnabled()) {
			m_ui->btnScan->setEnabled(true);
		}
	});
	cb->setChecked(true);
	m_ui->filterCheckBoxes->layout()->addWidget(cb);
}

void IioTabWidget::verifyBtnClicked()
{
	QRegExp ipRegex("^(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-"
			"4][0-9]|25[0-5])$");
	QString uri(m_ui->editUri->text());
	bool isIp = uri.contains(ipRegex);
	if(isIp && !m_ui->editUri->text().contains("ip:")) {
		m_ui->editUri->blockSignals(true);
		m_ui->editUri->setText("ip:" + uri);
		m_ui->editUri->blockSignals(false);
	}
	m_ui->btnScan->setDisabled(true);
	m_ui->btnVerify->startAnimation();
	Q_EMIT startVerify(m_ui->editUri->text(), "iio");
}

void IioTabWidget::onVerifyFinished(bool result)
{
	m_ui->uriMessageLabel->clear();
	if(!result) {
		m_ui->uriMessageLabel->setText("\"" + m_ui->editUri->text() + "\" not a valid context!");
	}
	m_ui->btnVerify->stopAnimation();
	m_ui->btnScan->setEnabled(true);
}

void IioTabWidget::futureScan()
{
	scanList.clear();
	QString scanParams = scanParamsList.join("").remove("serial:");
	QFuture<int> f = QtConcurrent::run(std::bind(&IIOScanTask::scan, &scanList, scanParams));
	fwScan->setFuture(f);
}

void IioTabWidget::scanFinished()
{
	int retCode = fwScan->result();
	m_ui->btnScan->stopAnimation();
	m_ui->comboBoxContexts->clear();
	m_ui->uriMessageLabel->clear();
	if(retCode < 0) {
		addScanFeedbackMsg("Scan command failed!");
		qWarning(CAT_IIO_ADD_PAGE) << "iio_scan_context_get_info_list error " << retCode;
		return;
	}
	if(scanList.isEmpty()) {
		addScanFeedbackMsg("No scanned contexts available!");
		return;
	}
	if(!m_ui->comboBoxContexts->isEnabled()) {
		m_ui->comboBoxContexts->setEnabled(true);
	}
	for(const auto &ctx : qAsConst(scanList)) {
		m_ui->comboBoxContexts->addItem(ctx);
	}
	findAvailableSerialPorts();
	updateUri(m_ui->comboBoxContexts->currentText());
}

void IioTabWidget::findAvailableSerialPorts()
{
	if(scanParamsList.contains("serial:")) {
		QVector<QString> portsName = IIOScanTask::getSerialPortsName();
		m_ui->comboBoxSerialPort->clear();
		if(!portsName.empty()) {
			for(const QString &port : portsName) {
				m_ui->comboBoxSerialPort->addItem(port);
			}
		}
	}
}

QString IioTabWidget::getSerialPath()
{
	QString serialPath = "serial:";
	serialPath.append(m_ui->comboBoxSerialPort->currentText());
	serialPath.append("," + m_ui->comboBoxBaudRate->currentText());
	serialPath.append("," + m_ui->editSerialFrameConfig->text());
	return serialPath;
}

void IioTabWidget::updateUri(QString uri)
{
	m_ui->editUri->clear();
	m_ui->editUri->setText(uri);
	if(!uri.isEmpty()) {
		m_ui->btnVerify->setFocus();
	}
}

void IioTabWidget::addScanFeedbackMsg(QString message)
{
	m_ui->comboBoxContexts->clear();
	m_ui->comboBoxContexts->addItem(message);
	m_ui->comboBoxContexts->setEnabled(false);
	updateUri("");
}

void IioTabWidget::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	m_ui->btnScan->setFocus();
}

#include "moc_iiotabwidget.cpp"
