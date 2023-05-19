#include "testplugin.h"
#include <QBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QPushButton>
#include <QLoggingCategory>
#include <QSpacerItem>
#include <pluginbase/preferences.h>
#include <pluginbase/messagebroker.h>
#include <pluginbase/preferenceshelper.h>
#include <QFile>
#include <QGraphicsEffect>
#include <QPainter>
#include <QDialog>
#include "tutorialoverlay.h"
#include <gui/utils.h>

Q_LOGGING_CATEGORY(CAT_TESTPLUGIN,"TestPlugin");
using namespace scopy;

bool TestPlugin::compatible(QString m_param, QString category) {
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
	QWidget *pref4 = PreferencesHelper::addPreferenceCombo(p,"pref4","languages",{"english","french","italian"},this);

	lay->addWidget(pref1);
	lay->addWidget(pref2);
	lay->addWidget(pref3);
	lay->addWidget(pref4);
	lay->addSpacerItem(new QSpacerItem(40,40,QSizePolicy::Minimum,QSizePolicy::Expanding));

	return true;
}

bool TestPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
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
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("test1first","FirstPlugin",":/gui/icons/scopy-default/icons/tool_home.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("test1second","Alexandra",":/gui/icons/scopy-default/icons/tool_io.svg"));
}

bool TestPlugin::loadExtraButtons()
{
	m_extraButtons.append(new QPushButton("Calibrate"));
	m_extraButtons.append(new QPushButton("Register"));
	connect(m_extraButtons[0],&QAbstractButton::clicked,this,[=](){edit->setText("Calibrating");});
	connect(m_extraButtons[1],&QAbstractButton::clicked,this,[=](){edit->setText("Registering");});
	return true;
}

void TestPlugin::startTutorial() {

	QWidget *window = Util::findContainingWindow(m_toolList[0]->tool());
	gui::TutorialOverlay *tut = new gui::TutorialOverlay(window);

	tut->addChapter({btn,lbl},
R"story(
# First Button

Two highlights with default **description** location.
)story");
	tut->addChapter(btn3, R"story(
# Second Button translated

One button, but tutorial *moved*
)story");
tut->addChapter(pic, R"story(
# Picture

Text overlayed on picture

You can even put a picture on top
![ADALM2000 pic!](:/gui/icons/adalm.svg "svgg")
)story");
tut->addChapter(lbl, R"story(
# Cristi's Label

This label is cool
)story");

tut->addChapter(nullptr,R"story(
# Link

For more info, visit [wiki](https://wiki.analog.com/)
![ADI](:/gui/icons/scopy-default/icons/logo.svg "ADI")
)story");


	tut->setTitle("Welcome to TestPlugin ! ");
	tut->start();
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
	lbl = new QLabel("TestPlugin", tool);
	pic = new QLabel("Picture",tool);
	lbl2 = new QLabel("m_initText->"+m_initText,tool);
	btn = new QPushButton("detach");
	btn2 = new QPushButton("renameTool");
	btn3 = new QPushButton("tutorial");
	connect(btn,&QPushButton::clicked,this,[=](){m_toolList[0]->setAttached(!m_toolList[0]->attached());});
	connect(btn2,&QPushButton::clicked,this,[=](){m_toolList[0]->setName("TestPlugin"+QString::number(renameCnt++));});
	connect(btn3,&QPushButton::clicked,this,[=](){startTutorial();});
	edit = new QLineEdit(tool);
	pic->setStyleSheet("border-image: url(\":/testplugin/testImage.png\") ");
	lay->addWidget(lbl);
	lay->addWidget(pic);
	lay->addWidget(edit);
	lay->addWidget(lbl2);
	lay->addWidget(btn);
	lay->addWidget(btn2);
	lay->addWidget(btn3);
	m_toolList[0]->setTool(tool);

	m_pluginApi = new TestPlugin_API(this);
	m_pluginApi->setObjectName(m_name);

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

void TestPlugin::saveSettings(QSettings &s)
{
	m_pluginApi->save(s);
}

void TestPlugin::loadSettings(QSettings &s)
{
	m_pluginApi->load(s);
}

QString TestPlugin::about()
{
	QFile f(":/testplugin/about.md");
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
