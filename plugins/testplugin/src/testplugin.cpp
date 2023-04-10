#include "testplugin.h"
#include <QBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QPushButton>
#include <QLoggingCategory>
#include <QUuid>
#include <QSpacerItem>
#include <pluginbase/preferences.h>
#include <pluginbase/messagebroker.h>
#include <pluginbase/preferenceshelper.h>
#include <QFile>

Q_LOGGING_CATEGORY(CAT_TESTPLUGIN,"TestPlugin");
using namespace adiscope;

bool TestPlugin::compatible(QString m_param) {
	qDebug(CAT_TESTPLUGIN)<<"compatible";
	return true;
}

void TestPlugin::initPreferences()
{
	Preferences *p = Preferences::GetInstance();
	p->init("pref1",false);
	p->init("pref2",true);
	p->init("prefstr","this is a string");
	p->init("pref4","english");
}

bool TestPlugin::loadPreferencesPage()
{

	Preferences *p = Preferences::GetInstance();

	m_preferencesPage = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_preferencesPage);
	QCheckBox *pref1 = PreferencesHelper::addPreferenceCheckBox(p,"pref1","First Option",this);
	QCheckBox *pref2 = PreferencesHelper::addPreferenceCheckBox(p,"pref2","Second Option",this);
	QLineEdit *pref3 = PreferencesHelper::addPreferenceEdit(p,"prefstr","PreferenceString",this);
	QComboBox *pref4 = PreferencesHelper::addPreferenceCombo(p,"pref4","languages",{"english","french","italian"},this);

	lay->addWidget(pref1);
	lay->addWidget(pref2);
	lay->addWidget(pref3);
	lay->addWidget(pref4);
	lay->addSpacerItem(new QSpacerItem(40,40,QSizePolicy::Minimum,QSizePolicy::Expanding));

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
	renameCnt = 0;
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("test1first","FirstPlugin",":/icons/scopy-default/icons/tool_home.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("test1second","Alexandra",":/icons/scopy-default/icons/tool_io.svg"));
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
	QLabel *lbl2 = new QLabel("m_initText->"+m_initText,tool);
	QPushButton *btn = new QPushButton("detach");
	QPushButton *btn2 = new QPushButton("renameTool");
	connect(btn,&QPushButton::clicked,this,[=](){m_toolList[0]->setAttached(!m_toolList[0]->attached());});
	connect(btn2,&QPushButton::clicked,this,[=](){m_toolList[0]->setName("TestPlugin"+QString::number(renameCnt++));});
	edit = new QLineEdit(tool);
	pic->setStyleSheet("border-image: url(\":/testImage.png\") ");
	lay->addWidget(lbl);
	lay->addWidget(pic);
	lay->addWidget(edit);
	lay->addWidget(lbl2);
	lay->addWidget(btn);
	lay->addWidget(btn2);
	m_toolList[0]->setTool(tool);


	return true;
}

bool TestPlugin::onDisconnect()
{
	qDebug(CAT_TESTPLUGIN)<<"disconnect";
	for(auto &tool : m_toolList) {
		tool->setEnabled(false);
		tool->setRunBtnVisible(false);
		QWidget *w = tool->tool();
		if(w) {
			tool->setTool(nullptr);
			delete(w);
		}
	}
	return true;
}

void TestPlugin::cloneExtra(Plugin *p)
{
	static int i = 0;

	m_initText = dynamic_cast<TestPlugin*>(p)->m_initText;

	m_initText += " Cloned from original " +QString::number(i)+" times";
	i++;
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

void TestPlugin::init()
{
	m_initText = "This text was initialized";
}

void TestPlugin::loadApi(){
	m_pluginApi = new TestPlugin_API(this);
	m_pluginApi->setObjectName(m_name);
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

#include "moc_testplugin.cpp"
