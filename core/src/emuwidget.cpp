#include "emuwidget.h"

#include "pluginbase/preferences.h"

#include "ui_emuwidget.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QLoggingCategory>

#include <filesystem>

Q_LOGGING_CATEGORY(CAT_EMU_ADD_PAGE, "EmuAddPage")
using namespace scopy;

EmuWidget::EmuWidget(QString path, QWidget *parent)
	: QWidget(parent)
	, m_ui(new Ui::EmuWidget)
	, m_emuPath(path)
	, m_enableDemo(false)
	, m_emuProcess(nullptr)
{
	m_ui->setupUi(this);
	m_emuProcess = new QProcess(this);
	connect(m_ui->comboBoxDemoOption, &QComboBox::currentTextChanged, this, &EmuWidget::onOptionChanged);
	init();

	connect(m_ui->btnEnableDemo, &QPushButton::clicked, this, &EmuWidget::onEnableDemoClicked);
	connect(m_ui->btnXmlPathBrowse, &QPushButton::clicked, this, [=]() { browseFile(m_ui->lineEditXmlPath); });
	connect(m_ui->btnRxTxDeviceBrowse, &QPushButton::clicked, this,
		[=]() { browseFile(m_ui->lineEditRxTxDevice); });
}

EmuWidget::~EmuWidget()
{
	if(m_emuProcess) {
		killEmuProcess();
	}
	delete m_ui;
}

void EmuWidget::init()
{
	Preferences *p = Preferences::GetInstance();
	p->init("iio_emu_path", QCoreApplication::applicationDirPath());

	m_emuPath = findEmuPath();
	m_emuPath.isEmpty() ? setStatusMessage("Can't find iio-emu in the system!") : setStatusMessage("");
	this->setEnabled(!m_emuPath.isEmpty());

	QMovie *loadingIcon(new QMovie(this));
	loadingIcon->setFileName(":/gui/loading.gif");
	m_ui->btnEnableDemo->setAnimation(loadingIcon);

	m_ui->btnEnableDemo->setProperty("blue_button", QVariant(true));
	m_ui->btnXmlPathBrowse->setProperty("blue_button", QVariant(true));
	m_ui->btnRxTxDeviceBrowse->setProperty("blue_button", QVariant(true));
	m_ui->btnEnableDemo->setAutoDefault(true);
	m_ui->lineEditUri->setPlaceholderText("ip:127.0.0.1");
	for(const QString &item : m_availableOptions) {
		m_ui->comboBoxDemoOption->addItem(item);
	}
}

void EmuWidget::setStatusMessage(QString msg)
{
	m_ui->labelStatusMsg->clear();
	m_ui->labelStatusMsg->setText(msg);
}

void EmuWidget::onEnableDemoClicked()
{
	m_ui->btnEnableDemo->startAnimation();
	if(!m_enableDemo) {
		bool started = startIioEmuProcess();
		if(!started) {
			stopEnableBtn("Enable Demo");
			return;
		}
		stopEnableBtn("Disable Demo");
		m_ui->widgetEnable->setEnabled(false);
		if(m_ui->lineEditUri->text().isEmpty()) {
			m_ui->lineEditUri->setText("ip:127.0.0.1");
		}
		m_enableDemo = !m_enableDemo;
		Q_EMIT emuDeviceAvailable(m_ui->lineEditUri->text());
	} else {
		killEmuProcess();
	}
}

QStringList EmuWidget::createArgList()
{
	QString option = m_ui->comboBoxDemoOption->currentText();
	QStringList arguments;
	arguments.append(option);
	if(option.compare("generic") == 0) {
		arguments.append(m_ui->lineEditXmlPath->text());
		arguments.append(m_ui->lineEditRxTxDevice->text());
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
	m_ui->btnEnableDemo->stopAnimation();
	m_ui->btnEnableDemo->setText(btnText);
}

bool EmuWidget::startIioEmuProcess()
{
	QStringList arg = createArgList();
	m_emuProcess->setProgram(m_emuPath);
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
	stopEnableBtn("Enable Demo");
	m_ui->widgetEnable->setEnabled(true);
	m_enableDemo = !m_enableDemo;
}

void EmuWidget::onOptionChanged(QString option)
{
	bool isGeneric = option.contains("generic");
	m_ui->widgetXmlPath->setEnabled(isGeneric);
	m_ui->widgetRxTxDevice->setEnabled(isGeneric);
	m_ui->btnEnableDemo->setFocus();
}

void EmuWidget::browseFile(QLineEdit *lineEditPath)
{
	QString filePath =
		QFileDialog::getOpenFileName(this, "Open a file", "directoryToOpen",
					     "All (*);;XML Files (*.xml);;Text Files (*.txt);;BIN Files (*.bin)");
	lineEditPath->setText(filePath);
	m_ui->btnEnableDemo->setFocus();
}

void EmuWidget::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	m_ui->btnEnableDemo->setFocus();
}

#include "moc_emuwidget.cpp"
