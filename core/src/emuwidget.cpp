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

#include "emuwidget.h"

#include "common/scopyconfig.h"
#include "pluginbase/preferences.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QLoggingCategory>
#include <stylehelper.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <style.h>
#include <menusectionwidget.h>

Q_LOGGING_CATEGORY(CAT_EMU_ADD_PAGE, "EmuAddPage")
using namespace scopy;

EmuWidget::EmuWidget(QWidget *parent)
	: QWidget(parent)
	, m_enableDemo(false)
	, m_emuProcess(nullptr)
	, m_workingDir("")
	, m_enDemoBtn(nullptr)
{
	QGridLayout *layout = new QGridLayout(this);
	layout->setSpacing(10);
	setLayout(layout);

	getJsonConfiguration();
	getEmuOptions();

	QWidget *container = new QWidget(this);
	container->setLayout(new QGridLayout(container));
	Style::setStyle(container, style::properties::widget::border_interactive);

	MenuSectionCollapseWidget *configSection = new MenuSectionCollapseWidget(
		"CONFIGURATION", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, container);
	configSection->menuSection()->layout()->setMargin(0);

	QWidget *emuWidget = new QWidget(configSection);
	QGridLayout *emuWidgetLay = new QGridLayout(emuWidget);
	emuWidgetLay->setMargin(0);
	emuWidgetLay->setSpacing(10);

	QLabel *demoLabel = new QLabel("Device", emuWidget);
	QWidget *demoOptWidget = createDemoOptWidget(emuWidget);
	emuWidgetLay->addWidget(demoLabel, 0, 0);
	emuWidgetLay->addWidget(demoOptWidget, 0, 1);
	connect(this, &EmuWidget::demoEnabled, demoLabel, &QWidget::setDisabled);
	connect(this, &EmuWidget::demoEnabled, demoOptWidget, &QWidget::setDisabled);

	QLabel *xmlLabel = new QLabel("XML", emuWidget);
	QWidget *xmlPathWidget = createXmlPathWidget(emuWidget);
	emuWidgetLay->addWidget(xmlLabel, 1, 0);
	emuWidgetLay->addWidget(xmlPathWidget, 1, 1);
	connect(this, &EmuWidget::demoEnabled, xmlLabel, &QWidget::setDisabled);
	connect(this, &EmuWidget::demoEnabled, xmlPathWidget, &QWidget::setDisabled);

	QLabel *rxTxLabel = new QLabel("Rx/Tx", emuWidget);
	QWidget *rxTxDevWidget = createRxTxDevWidget(emuWidget);
	emuWidgetLay->addWidget(rxTxLabel, 2, 0);
	emuWidgetLay->addWidget(rxTxDevWidget, 2, 1);
	connect(this, &EmuWidget::demoEnabled, rxTxLabel, &QWidget::setDisabled);
	connect(this, &EmuWidget::demoEnabled, rxTxDevWidget, &QWidget::setDisabled);

	QLabel *uriLabel = new QLabel("URI", emuWidget);
	QWidget *uriWidget = createUriWidget(emuWidget);
	emuWidgetLay->addWidget(uriLabel, 3, 0);
	emuWidgetLay->addWidget(uriWidget, 3, 1);
	connect(this, &EmuWidget::demoEnabled, uriLabel, &QWidget::setDisabled);

	m_uriMsgLabel = new QLabel(emuWidget);
	m_uriMsgLabel->setVisible(false);
	emuWidgetLay->addWidget(m_uriMsgLabel, 4, 1);

	configSection->add(emuWidget);
	container->layout()->addWidget(configSection);

	layout->addWidget(container, 0, 0);
	layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 1);
	layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 1, 0);

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
	Preferences::init("iio_emu_dir_path", "");
	m_emuPath = findEmuPath();
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
	m_xmlPathEdit->setText("");
	m_rxTxDevEdit->setText("");
	m_uriEdit->setText("");

	bool isNotAdalm2000 = !crtOpt.contains("adalm2000");

	xmlPathWidget->setEnabled(isNotAdalm2000);
	rxTxDevWidget->setEnabled(isNotAdalm2000);
	m_enDemoBtn->setFocus();

	configureOption(crtOpt);
}

void EmuWidget::setStatusMessage(QString msg)
{
	m_uriMsgLabel->setHidden(msg.isEmpty());
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
			stopEnableBtn("Enable");
			return;
		}
		stopEnableBtn("Disable");
		if(m_uriEdit->text().isEmpty()) {
			m_uriEdit->setText("ip:127.0.0.1");
		}
		setEnableDemo(!m_enableDemo);
		Q_EMIT emuDeviceAvailable(m_uriEdit->text());
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
		auto xmlFullPath = m_xmlPathEdit->text();
		QFileInfo f(xmlFullPath);
		m_workingDir = f.absoluteDir().path();

		arguments.append(f.fileName());
		arguments.append(m_rxTxDevEdit->text());
	} else {
		m_workingDir = "";
	}

	return arguments;
}

QString EmuWidget::findEmuPath()
{
	// The path from the preferences has the highest priority. The path must be added manually.
	QString emuPath = buildEmuPath(Preferences::get("iio_emu_dir_path").toString());
	if(!emuPath.isEmpty() && QFile::exists(emuPath)) {
		qInfo(CAT_EMU_ADD_PAGE) << "iio-emu path from preferences:" << emuPath;
		return emuPath;
	}
	// Search iio-emu next to scopy executable.
	emuPath = buildEmuPath(scopy::config::executableFolderPath());
	if(!emuPath.isEmpty() && QFile::exists(emuPath)) {
		qInfo(CAT_EMU_ADD_PAGE) << "iio-emu path:" << emuPath;
		return emuPath;
	}
	// Search iio-emu in system.
	emuPath = "iio-emu";
	if(startIioEmuProcess(emuPath)) {
		qInfo(CAT_EMU_ADD_PAGE) << "iio-emu from system!";
		return emuPath;
	}

	return "";
}

QString EmuWidget::buildEmuPath(QString dirPath)
{
	QString emuPath = "";
	if(!dirPath.isEmpty()) {
		emuPath = dirPath + QDir::separator() + "iio-emu";
#ifdef WIN32
		emuPath += ".exe";
#endif
	}
	return emuPath;
}

void EmuWidget::stopEnableBtn(QString btnText)
{
	m_enDemoBtn->stopAnimation();
	m_enDemoBtn->setText(btnText);
}

bool EmuWidget::startIioEmuProcess(QString processPath, QStringList arg)
{
	m_emuProcess->setProgram(processPath);
	m_emuProcess->setWorkingDirectory(m_workingDir);
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
	stopEnableBtn("Enable");
	setEnableDemo(!m_enableDemo);
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
				m_xmlPathEdit->setText(currentPath + xmlPath);
			}

			if(jsonObject.contains("rx_tx_device")) {
				QString rxTxDevice = jsonObject.value(QString("rx_tx_device")).toString();
				rxTxDevice += "@";
				rxTxDevice += currentPath;
				rxTxDevice += jsonObject.value(QString("rx_tx_bin_path")).toString();
				m_rxTxDevEdit->setText(rxTxDevice);
			}

			if(jsonObject.contains("uri")) {
				QString uri = jsonObject.value(QString("uri")).toString();
				m_uriEdit->setText(uri);
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
	// Check the local file first
	QString filePath = QCoreApplication::applicationDirPath() + "/plugins/resources/scopy_emu_options_config.json";
	QFile file(filePath);
	if(!file.exists()) {
		filePath = config::defaultPluginFolderPath() + "/resources/scopy_emu_options_config.json";
		file.setFileName(filePath);
	}
	qDebug(CAT_EMU_ADD_PAGE) << "Emu configuration file: " << filePath;

	if(file.exists()) {
		file.open(QIODevice::ReadOnly | QIODevice::Text);
		m_jsonConfigVal = file.readAll();
		file.close();
	} else {
		qWarning(CAT_EMU_ADD_PAGE) << "Emu configuration file is missing ";
		m_availableOptions.append("generic");
	}
}

void EmuWidget::setEnableDemo(bool en)
{
	m_enableDemo = en;
	Q_EMIT demoEnabled(en);
}

void EmuWidget::browseFile(QLineEdit *lineEditPath)
{
	bool useNativeDialogs = Preferences::get("general_use_native_dialogs").toBool();
	QString filePath = QFileDialog::getOpenFileName(
		this, "Open a file", "directoryToOpen",
		"All (*);;XML Files (*.xml);;Text Files (*.txt);;BIN Files (*.bin)", nullptr,
		(useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));
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
	for(const QString &opt : qAsConst(m_availableOptions)) {
		m_demoOptCb->addItem(opt);
	}
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

	m_xmlPathEdit = new QLineEdit(w);

	QPushButton *xmlPathBtn = new QPushButton("...", w);
	StyleHelper::BrowseButton(xmlPathBtn);
	connect(xmlPathBtn, &QPushButton::clicked, this, [=]() { browseFile(m_xmlPathEdit); });

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

	m_rxTxDevEdit = new QLineEdit(w);
	m_rxTxDevEdit->setPlaceholderText("iio:device0@/absolutePathTo/data.bin");

	QPushButton *rxTxDevBtn = new QPushButton("...", w);
	StyleHelper::BrowseButton(rxTxDevBtn);

	connect(rxTxDevBtn, &QPushButton::clicked, this, [=]() { browseFile(m_rxTxDevEdit); });

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

	m_uriEdit = new QLineEdit(w);
	m_uriEdit->setPlaceholderText("ip:127.0.0.1");
	connect(this, &EmuWidget::demoEnabled, m_uriEdit, &QWidget::setDisabled);

	initEnBtn(w);

	layout->addWidget(m_uriEdit);
	layout->addWidget(m_enDemoBtn);
	return w;
}

void EmuWidget::initEnBtn(QWidget *parent)
{
	m_enDemoBtn = new AnimationPushButton(parent);
	m_enDemoBtn->setText("Enable");
	StyleHelper::BasicButton(m_enDemoBtn);
	m_enDemoBtn->setFixedWidth(Style::getDimension(json::global::unit_6));
	QMovie *loadingIcon(new QMovie(this));
	loadingIcon->setFileName(":/gui/loading.gif");
	m_enDemoBtn->setAnimation(loadingIcon);
	m_enDemoBtn->setAutoDefault(true);
}

void EmuWidget::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	m_enDemoBtn->setFocus();
}

#include "moc_emuwidget.cpp"
