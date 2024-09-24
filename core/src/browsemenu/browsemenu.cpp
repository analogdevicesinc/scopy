#include "browsemenu/browsemenu.h"

#include <QPushButton>
#include <stylehelper.h>

using namespace scopy;

BrowseMenu::BrowseMenu(QWidget *parent)
	: QWidget(parent)
	, m_collapsed(false)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setMargin(0);
	lay->setSpacing(0);
	lay->setAlignment(Qt::AlignTop);
	setLayout(lay);

	QWidget *menuHeader = createHeader(this);

	m_content = new QWidget(this);
	m_content->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	m_contentLay = new QVBoxLayout(m_content);
	m_contentLay->setMargin(0);
	m_contentLay->setSpacing(0);
	m_content->setLayout(m_contentLay);

	m_spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Preferred);
	m_contentLay->addSpacerItem(m_spacer);

	m_instrumentMenu = new InstrumentMenu(m_content);
	QButtonGroup *btnGroup = m_instrumentMenu->btnGroup();
	m_instrumentMenu->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

	QPushButton *homeBtn = new QPushButton(tr("Home"), m_content);
	homeBtn->setIcon(QIcon(":/gui/icons/scopy-default/icons/tool_home.svg"));
	StyleHelper::ToolMenuHome(homeBtn, "toolMenuHome");
	connect(homeBtn, &QPushButton::clicked, this, [=]() { Q_EMIT requestTool("home"); });

	QWidget *saveLoadWidget = new QWidget(m_content);
	saveLoadWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	saveLoadWidget->setLayout(new QHBoxLayout(saveLoadWidget));
	saveLoadWidget->layout()->setSpacing(0);
	saveLoadWidget->layout()->setMargin(0);

	QPushButton *saveBtn = createBtn("Save", ":/gui/icons/scopy-default/icons/save.svg", saveLoadWidget);
	saveBtn->setCheckable(false);
	connect(saveBtn, &QPushButton::clicked, this, &BrowseMenu::requestSave);

	QPushButton *loadBtn = createBtn("Load", ":/gui/icons/scopy-default/icons/load.svg", saveLoadWidget);
	loadBtn->setCheckable(false);
	connect(loadBtn, &QPushButton::clicked, this, &BrowseMenu::requestLoad);

	saveLoadWidget->layout()->addWidget(saveBtn);
	saveLoadWidget->layout()->addWidget(loadBtn);

	QPushButton *preferencesBtn =
		createBtn("Preferences", ":/gui/icons/scopy-default/icons/preferences.svg", m_content);
	connect(preferencesBtn, &QPushButton::clicked, this, [=]() { Q_EMIT requestTool("preferences"); });

	QPushButton *aboutBtn = createBtn("About", ":/gui/icons/scopy-default/icons/info.svg", m_content);
	connect(aboutBtn, &QPushButton::clicked, this, [=]() { Q_EMIT requestTool("about"); });

	QLabel *logo = createScopyLogo(m_content);

	btnGroup->addButton(homeBtn);
	btnGroup->addButton(preferencesBtn);
	btnGroup->addButton(aboutBtn);

	add(createHLine(m_content), "headerLine", MA_TOPLAST);
	add(homeBtn, "homeBtn", MA_TOPLAST);
	add(createHLine(m_content), "instrMenuLine1", MA_TOPLAST);
	add(m_instrumentMenu, "instrumentMenu", MA_TOPLAST);

	add(createHLine(m_content), "instrMenuLine2", MA_BOTTOMLAST);
	add(saveLoadWidget, "saveLoad", MA_BOTTOMLAST);
	add(preferencesBtn, "preferencesBtn", MA_BOTTOMLAST);
	add(aboutBtn, "aboutBtn", MA_BOTTOMLAST);
	add(logo, "logo", MA_BOTTOMLAST);

	lay->addWidget(menuHeader);
	lay->addWidget(m_content);
}

BrowseMenu::~BrowseMenu() {}

InstrumentMenu *BrowseMenu::instrumentMenu() const { return m_instrumentMenu; }

void BrowseMenu::add(QWidget *w, QString name, MenuAlignment position)
{
	int spacerIndex = m_contentLay->indexOf(m_spacer);
	switch(position) {

	case MA_TOPLAST:
		m_contentLay->insertWidget(spacerIndex, w);
		break;
	case MA_BOTTOMLAST:
		m_contentLay->insertWidget(-1, w);
		break;
	}
}

void BrowseMenu::toggleCollapsed()
{
	m_collapsed = !m_collapsed;
	m_btnCollapse->setHidden(m_collapsed);
	m_content->setHidden(m_collapsed);
	Q_EMIT collapsed(m_collapsed);
}

QPushButton *BrowseMenu::createBtn(QString name, QString iconPath, QWidget *parent)
{
	QPushButton *btn = new QPushButton(parent);
	btn->setIcon(QIcon(iconPath));
	btn->setText(tr(name.toStdString().c_str()));
	StyleHelper::ToolMenuBtn(btn, name + "ToolMenuBtn");
	return btn;
}

QFrame *BrowseMenu::createHLine(QWidget *parent)
{
	QFrame *line = new QFrame(parent);
	line->setFrameShape(QFrame::HLine);
	line->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	return line;
}

QWidget *BrowseMenu::createHeader(QWidget *parent)
{
	QWidget *menuHeader = new QWidget(parent);
	menuHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QHBoxLayout *headerLay = new QHBoxLayout(menuHeader);
	headerLay->setSpacing(0);
	headerLay->setMargin(0);
	QPushButton *btnCollapseMini = new QPushButton(menuHeader);
	StyleHelper::ToolMenuCollapseMini(btnCollapseMini, "CollapseMiniBtn");
	headerLay->addWidget(btnCollapseMini);

	m_btnCollapse = new QPushButton(menuHeader);
	StyleHelper::ToolMenuCollapse(m_btnCollapse, "ToolMenuCollapse");

	headerLay->addWidget(m_btnCollapse);
	connect(m_btnCollapse, &QPushButton::clicked, this, &BrowseMenu::toggleCollapsed);
	connect(btnCollapseMini, &QPushButton::clicked, this, &BrowseMenu::toggleCollapsed);

	return menuHeader;
}

QLabel *BrowseMenu::createScopyLogo(QWidget *parent)
{
	QLabel *logo = new QLabel(m_content);
	StyleHelper::ScopyLogo(logo, "logo");
	return logo;
}
