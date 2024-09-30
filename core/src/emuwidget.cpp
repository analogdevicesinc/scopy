#include "emuwidget.h"

#include "common/scopyconfig.h"
#include "pluginbase/preferences.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QLoggingCategory>
#include <stylehelper.h>

#include <filesystem>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

Q_LOGGING_CATEGORY(CAT_EMU_ADD_PAGE, "EmuAddPage")
using namespace scopy;

EmuWidget::EmuWidget(QWidget *parent)
	: QWidget(parent)
	, m_enableDemo(false)
	, m_emuProcess(nullptr)
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setSpacing(10);
	setLayout(layout);
	QWidget *vWidget = new QWidget(this);
	QVBoxLayout *vLay = new QVBoxLayout(vWidget);
	vLay->setSpacing(10);
	vLay->setMargin(0);
	vWidget->setLayout(vLay);

	getJsonConfiguration();
	getEmuOptions();

	m_emuWidget = new QWidget(vWidget);
	QGridLayout *emuWidgetLay = new QGridLayout(m_emuWidget);
	emuWidgetLay->setSpacing(10);
	emuWidgetLay->setMargin(0);

	QLabel *demoLabel = new QLabel("Demo option:", m_emuWidget);
	StyleHelper::MenuSmallLabel(demoLabel);
	QWidget *demoOptWidget = createDemoOptWidget(m_emuWidget);
	emuWidgetLay->addWidget(demoLabel, 0, 0);
	emuWidgetLay->addWidget(demoOptWidget, 0, 1);

	QLabel *xmlLabel = new QLabel("XML path:", m_emuWidget);
	StyleHelper::MenuSmallLabel(xmlLabel);
	QWidget *xmlPathWidget = createXmlPathWidget(m_emuWidget);
	emuWidgetLay->addWidget(xmlLabel, 1, 0);
	emuWidgetLay->addWidget(xmlPathWidget, 1, 1);

	QLabel *rxTxLabel = new QLabel("RX/TX Devices:", m_emuWidget);
	StyleHelper::MenuSmallLabel(rxTxLabel);
	QWidget *rxTxDevWidget = createRxTxDevWidget(m_emuWidget);
	emuWidgetLay->addWidget(rxTxLabel, 2, 0);
	emuWidgetLay->addWidget(rxTxDevWidget, 2, 1);

	QLabel *uriLabel = new QLabel("Uri:", m_emuWidget);
	StyleHelper::MenuSmallLabel(uriLabel);
	QWidget *uriWidget = createUriWidget(m_emuWidget);
	emuWidgetLay->addWidget(uriLabel, 3, 0);
	emuWidgetLay->addWidget(uriWidget, 3, 1);

	QWidget *btnWidget = createEnBtnWidget(vWidget);

	vLay->addWidget(m_emuWidget);
	vLay->addWidget(btnWidget);
	vLay->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	layout->addWidget(vWidget);
	layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

	enGenericOptWidget(xmlPathWidget, rxTxDevWidget, m_demoOptCb->currentText());
	connect(m_demoOptCb, &QComboBox::currentTextChanged, this,
		[this, xmlPathWidget, rxTxDevWidget](QString option) {
			enGenericOptWidget(xmlPathWidget, rxTxDevWidget, option);
		});
	connect(m_enDemoBtn, &QPushButton::clicked, this, &EmuWidget::onEnableDemoClicked);

	m_emuProcess = new QProcess(this);
	init();
}

EmuWidget::~EmuWidget()
{
	if(m_emuProcess) {
		killEmuProcess();
	}
}

void EmuWidget::init()
{
	Preferences *p = Preferences::GetInstance();
	p->init("iio_emu_path", QCoreApplication::applicationDirPath());

	QString systemEmuCall = "iio-emu";
	if(startIioEmuProcess(systemEmuCall)) {
		m_emuPath = systemEmuCall;
	} else {
		m_emuPath = findEmuPath();
	}
	if(m_emuPath.isEmpty()) {
		setStatusMessage("Can't find iio-emu in the system!");
	} else {
		setStatusMessage("");
	}
	this->setEnabled(!m_emuPath.isEmpty());
}

void EmuWidget::enGenericOptWidget(QWidget *xmlPathWidget, QWidget *rxTxDevWidget, QString crtOpt)
{
	// when a new option is selected clear all fields
	m_xmlPathEdit->edit()->setText("");
	m_rxTxDevEdit->edit()->setText("");
	m_uriEdit->edit()->setText("");

	bool isNotAdalm2000 = !crtOpt.contains("adalm2000");

	xmlPathWidget->setEnabled(isNotAdalm2000);
	rxTxDevWidget->setEnabled(isNotAdalm2000);
	m_enDemoBtn->setFocus();

	configureOption(crtOpt);
}

void EmuWidget::setStatusMessage(QString msg)
{
	m_uriMsgLabel->clear();
	m_uriMsgLabel->setText(msg);
}

void EmuWidget::onEnableDemoClicked()
{
	m_enDemoBtn->startAnimation();
	if(!m_enableDemo) {
		QStringList arg = createArgList();
		bool started = startIioEmuProcess(m_emuPath, arg);
		if(!started) {
			stopEnableBtn("Enable Demo");
			return;
		}
		m_emuWidget->setEnabled(false);
		stopEnableBtn("Disable Demo");
		if(m_uriEdit->edit()->text().isEmpty()) {
			m_uriEdit->edit()->setText("ip:127.0.0.1");
		}
		m_enableDemo = !m_enableDemo;
		Q_EMIT emuDeviceAvailable(m_uriEdit->edit()->text());
	} else {
		killEmuProcess();
	}
}

QStringList EmuWidget::createArgList()
{
	QString option = m_demoOptCb->currentText();
	QStringList arguments;
	arguments.append(m_emuType);

	if(option.compare("adalm2000") != 0) {
		arguments.append(m_xmlPathEdit->edit()->text());
		arguments.append(m_rxTxDevEdit->edit()->text());
	}

	return arguments;
}

QString EmuWidget::findEmuPath()
{
	Preferences *p = Preferences::GetInstance();
	QString program = p->get("iio_emu_path").toString() + "/iio-emu";
#ifdef WIN32
	program += ".exe";
#endif

	QFileInfo fi(program);
	if(!fi.exists()) {
		program = "";
	}
	return program;
}

void EmuWidget::stopEnableBtn(QString btnText)
{
	m_enDemoBtn->stopAnimation();
	m_enDemoBtn->setText(btnText);
}

bool EmuWidget::startIioEmuProcess(QString processPath, QStringList arg)
{
	m_emuProcess->setProgram(processPath);
	m_emuProcess->setArguments(arg);
	m_emuProcess->start();

	auto started = m_emuProcess->waitForStarted();
	if(!started) {
		setStatusMessage("Server failed to start!");
		qDebug(CAT_EMU_ADD_PAGE) << "Process failed to start";
	} else {
		qDebug(CAT_EMU_ADD_PAGE) << "Process " << m_emuPath << "started";
	}
	return started;
}

void EmuWidget::killEmuProcess()
{
	m_emuProcess->kill();
	m_emuWidget->setEnabled(true);
	stopEnableBtn("Enable Demo");
	m_enableDemo = !m_enableDemo;
}

void EmuWidget::getEmuOptions()
{
	// Populate emu devices from json
	QJsonDocument d = QJsonDocument::fromJson(m_jsonConfigVal.toUtf8());
	QJsonArray valuesList = d.array();

	for(auto object : valuesList) {
		QString device = object.toObject().value(QString("device")).toString();
		m_availableOptions.append(device);
	}
}

void EmuWidget::configureOption(QString option)
{
	QJsonDocument d = QJsonDocument::fromJson(m_jsonConfigVal.toUtf8());
	QJsonArray valuesList = d.array();

	for(auto jsonArrayItem : valuesList) {
		QJsonObject jsonObject = jsonArrayItem.toObject();
		QString device = jsonObject.value(QString("device")).toString();
		if(device == option) {

			// Check the local folder first
			QString currentPath = QCoreApplication::applicationDirPath() + "/plugins/emu_xml/";
			if(!QDir(currentPath).exists()) {
				currentPath = config::defaultPluginFolderPath() + "/emu_xml/";
			}

			qDebug(CAT_EMU_ADD_PAGE) << "Emu xmls path: " << currentPath;

			if(jsonObject.contains("xml_path")) {
				QString xmlPath = jsonObject.value(QString("xml_path")).toString();
				m_xmlPathEdit->edit()->setText(currentPath + xmlPath);
			}

			if(jsonObject.contains("rx_tx_device")) {
				QString rxTxDevice = jsonObject.value(QString("rx_tx_device")).toString();
				rxTxDevice += "@";
				rxTxDevice += currentPath;
				rxTxDevice += jsonObject.value(QString("rx_tx_bin_path")).toString();
				m_rxTxDevEdit->edit()->setText(rxTxDevice);
			}

			if(jsonObject.contains("uri")) {
				QString uri = jsonObject.value(QString("uri")).toString();
				m_uriEdit->edit()->setText(uri);
			}

			if(jsonObject.contains("emu-type")) {
				m_emuType = jsonObject.value(QString("emu-type")).toString();
			} else {
				m_emuType = "generic";
			}

			break;
		}
	}
}

void EmuWidget::getJsonConfiguration()
{
	// Check the local folder first
	QString currentPath = QCoreApplication::applicationDirPath() + "/plugins";
	if(!QDir(currentPath).exists()) {
		currentPath = config::defaultPluginFolderPath();
	}

	QString filePath = currentPath + "/resources/scopy_emu_options_config.json";
	QFile file;

	file.setFileName(filePath);

	if(file.exists()) {
		file.open(QIODevice::ReadOnly | QIODevice::Text);
		m_jsonConfigVal = file.readAll();
		file.close();
	} else {
		qWarning(CAT_EMU_ADD_PAGE) << "Emu configuration file is missing ";
		m_availableOptions.append("generic");
	}
}

void EmuWidget::browseFile(QLineEdit *lineEditPath)
{
	QString filePath =
		QFileDialog::getOpenFileName(this, "Open a file", "directoryToOpen",
					     "All (*);;XML Files (*.xml);;Text Files (*.txt);;BIN Files (*.bin)");
	lineEditPath->setText(filePath);
	m_enDemoBtn->setFocus();
}

QWidget *EmuWidget::createDemoOptWidget(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QHBoxLayout *layout = new QHBoxLayout(w);
	layout->setMargin(0);
	layout->setSpacing(10);
	w->setLayout(layout);

	m_demoOptCb = new QComboBox(w);
	for(const QString &opt : m_availableOptions) {
		m_demoOptCb->addItem(opt);
	}
	StyleHelper::MenuComboBox(m_demoOptCb, "demo_combo");
	layout->addWidget(m_demoOptCb);
	return w;
}

QWidget *EmuWidget::createXmlPathWidget(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QHBoxLayout *layout = new QHBoxLayout(w);
	layout->setMargin(0);
	layout->setSpacing(10);
	w->setLayout(layout);

	m_xmlPathEdit = new MenuLineEdit(w);

	QPushButton *xmlPathBtn = new QPushButton("...", w);
	StyleHelper::BrowseButton(xmlPathBtn);
	connect(xmlPathBtn, &QPushButton::clicked, this, [=]() { browseFile(m_xmlPathEdit->edit()); });

	layout->addWidget(m_xmlPathEdit);
	layout->addWidget(xmlPathBtn);
	return w;
}

QWidget *EmuWidget::createRxTxDevWidget(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QHBoxLayout *layout = new QHBoxLayout(w);
	layout->setMargin(0);
	layout->setSpacing(10);
	w->setLayout(layout);

	m_rxTxDevEdit = new MenuLineEdit(w);
	m_rxTxDevEdit->edit()->setPlaceholderText("iio:device0@/absolutePathTo/data.bin");

	QPushButton *rxTxDevBtn = new QPushButton("...", w);
	StyleHelper::BrowseButton(rxTxDevBtn);

	connect(rxTxDevBtn, &QPushButton::clicked, this, [=]() { browseFile(m_rxTxDevEdit->edit()); });

	layout->addWidget(m_rxTxDevEdit);
	layout->addWidget(rxTxDevBtn);
	return w;
}

QWidget *EmuWidget::createUriWidget(QWidget *parent)
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
	m_uriEdit->edit()->setPlaceholderText("ip:127.0.0.1");
	m_uriMsgLabel = new QLabel(msgUriWidget);
	msgUriWidget->layout()->addWidget(m_uriEdit);
	msgUriWidget->layout()->addWidget(m_uriMsgLabel);

	layout->addWidget(msgUriWidget);
	return w;
}

QWidget *EmuWidget::createEnBtnWidget(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	w->setLayout(new QHBoxLayout(w));
	w->layout()->setMargin(0);
	w->layout()->setAlignment(Qt::AlignRight);

	m_enDemoBtn = new AnimationPushButton(parent);
	m_enDemoBtn->setText("Enable demo");
	StyleHelper::BlueButton(m_enDemoBtn);
	m_enDemoBtn->setFixedWidth(128);
	QMovie *loadingIcon(new QMovie(this));
	loadingIcon->setFileName(":/gui/loading.gif");
	m_enDemoBtn->setAnimation(loadingIcon);
	m_enDemoBtn->setAutoDefault(true);

	w->layout()->addWidget(m_enDemoBtn);
	return w;
}

void EmuWidget::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	m_enDemoBtn->setFocus();
}

#include "moc_emuwidget.cpp"
