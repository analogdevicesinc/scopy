#include "testplugin.h"
#include <QBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QPushButton>
#include <QLoggingCategory>
#include <QUuid>
#include <QFile>
#include <QCheckBox>
#include <QSpacerItem>
#include <pluginbase/preferences.h>
#include <pluginbase/messagebroker.h>

Q_LOGGING_CATEGORY(CAT_TESTPLUGIN,"TestPlugin");
using namespace adiscope;

bool TestPlugin::compatible(QString uri) {
	qDebug(CAT_TESTPLUGIN)<<"compatible";
	return true;
}

void TestPlugin::initPreferences()
{
	Preferences *p = Preferences::GetInstance();
	p->init("pref1",false);
	p->init("pref2",true);
}

bool TestPlugin::loadPreferencesPage()
{

	Preferences *p = Preferences::GetInstance();

	bool pref1Val = p->get("pref1").toBool();
	bool pref2Val = p->get("pref2").toBool();

	m_preferencesPage = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_preferencesPage);
	QCheckBox *pref1 = new QCheckBox("First option",m_preferencesPage);
	QCheckBox *pref2 = new QCheckBox("Second preference",m_preferencesPage);
	lay->addWidget(pref1);
	lay->addWidget(pref2);

	pref1->setChecked(pref1Val);
	pref2->setChecked(pref2Val);

	connect(pref1,&QCheckBox::toggled,this,[=](bool b) { p->set("pref1",b);});
	connect(pref2,&QCheckBox::toggled,this,[=](bool b) { p->set("pref2",b);});

	return true;
}

bool TestPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/icons/adalm.svg");
	return true;
}

bool TestPlugin::loadPage()
{
	m_page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_page);
	lay->addWidget(new QLabel("TestPage",m_page));
	QPushButton* restartBtn = new QPushButton("restartPlugin",m_page);
	lay->addWidget(restartBtn);
	connect(restartBtn,SIGNAL(clicked()),this,SIGNAL(restartDevice()));
	return true;

}

void TestPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("FirstPlugin",":/icons/scopy-default/icons/tool_home.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("Alexandra",":/icons/scopy-default/icons/tool_io.svg"));
}

bool TestPlugin::loadExtraButtons()
{
	m_extraButtons.append(new QPushButton("Calibrate"));
	m_extraButtons.append(new QPushButton("Register"));
	connect(m_extraButtons[0],&QAbstractButton::clicked,this,[=](){edit->setText("Calibrating");});
	connect(m_extraButtons[1],&QAbstractButton::clicked,this,[=](){edit->setText("Registering");});
	return true;
}

bool TestPlugin::onConnect()
{
	MessageBroker::GetInstance()->subscribe(this, "TestPlugin");
	qDebug(CAT_TESTPLUGIN)<<"connect";
	qDebug(CAT_TESTPLUGIN)<<m_toolList[0]->id()<<m_toolList[0]->name();

	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setName("TestPlugin");
	m_toolList[0]->setRunBtnVisible(true);

	tool = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(tool);
	QLabel *lbl = new QLabel("TestPlugin", tool);
	QLabel *pic = new QLabel("Picture",tool);
	edit = new QLineEdit(tool);
	pic->setStyleSheet("border-image: url(\":/testImage.png\") ");
	lay->addWidget(lbl);
	lay->addWidget(pic);
	lay->addWidget(edit);
	m_toolList[0]->setTool(tool);

	return true;
}

bool TestPlugin::onDisconnect()
{
	qDebug(CAT_TESTPLUGIN)<<"disconnect";
	for(auto &tool : m_toolList) {
		tool->setEnabled(false);
		tool->setRunBtnVisible(false);
		if(tool->tool()) {
			delete tool->tool();
			tool->setTool(nullptr);
		}
	}
	return true;
}

void TestPlugin::messageCallback(QString topic, QString message)
{
	qInfo(CAT_TESTPLUGIN) << topic <<": "<<message;
}

QString TestPlugin::about()
{
	QFile f(":/about.md");
	f.open(QFile::ReadOnly);
	QString content = f.readAll();
	return content;
}

void TestPlugin::initMetadata()
{
	loadMetadata(
R"plugin(
	{
	   "priority":2,
	   "category":[
	      "test"
	   ]
	}
)plugin");
}

void TestPlugin::loadApi(){
	pluginApi = new TestPlugin_API(this);
	pluginApi->setObjectName(m_name);
}

QString TestPlugin::version() {
	return "0.1";
}

// --------------------

QString TestPlugin_API::testText() const
{
	return p->edit->text();
}

void TestPlugin_API::setTestText(const QString &newTestText)
{
	p->edit->setText(newTestText);
}
