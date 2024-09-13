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
	lay->setSpacing(10);
	lay->setAlignment(Qt::AlignTop);
	setLayout(lay);

	QWidget *menuHeader = createHeader(this);

	m_content = new QWidget(this);
	m_content->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	m_contentLay = new QVBoxLayout(m_content);
	m_contentLay->setMargin(0);
	m_contentLay->setSpacing(10);
	m_content->setLayout(m_contentLay);

	m_spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Preferred);
	m_contentLay->addSpacerItem(m_spacer);

	m_instrumentMenu = new InstrumentMenu(m_content);
	QButtonGroup *btnGroup = m_instrumentMenu->btnGroup();
	m_instrumentMenu->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

	QPushButton *homeBtn = createBtn(tr("Home"), ":/gui/icons/scopy-default/icons/tool_home.svg", m_content);
	connect(homeBtn, &QPushButton::clicked, this, [=]() { Q_EMIT requestTool("home"); });

	QWidget *saveLoadWidget = new QWidget(m_content);
	saveLoadWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	saveLoadWidget->setLayout(new QHBoxLayout(saveLoadWidget));
	saveLoadWidget->layout()->setMargin(0);

	QPushButton *saveBtn = createBtn(tr("Save"), ":/gui/icons/scopy-default/icons/save.svg", saveLoadWidget);
	connect(saveBtn, &QPushButton::clicked, this, &BrowseMenu::requestSave);

	QPushButton *loadBtn = createBtn(tr("Load"), ":/gui/icons/scopy-default/icons/load.svg", saveLoadWidget);
	connect(loadBtn, &QPushButton::clicked, this, &BrowseMenu::requestLoad);

	saveLoadWidget->layout()->addWidget(saveBtn);
	saveLoadWidget->layout()->addWidget(loadBtn);

	QPushButton *preferencesBtn =
		createBtn(tr("Preferences"), ":/gui/icons/scopy-default/icons/preferences.svg", m_content);
	connect(preferencesBtn, &QPushButton::clicked, this, [=]() { Q_EMIT requestTool("preferences"); });

	QPushButton *aboutBtn = createBtn(tr("About"), ":/gui/icons/scopy-default/icons/info.svg", m_content);
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
	btn->setCheckable(true);
	btn->setText(name);
	btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
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
	btnCollapseMini->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	btnCollapseMini->setCheckable(true);
	btnCollapseMini->setStyleSheet(" height:40px;\n"
				       " width:40px;\n"
				       " background-image: url(:/gui/icons/scopy-default/icons/menu.svg);\n"
				       " background-repeat: no-repeat;\n"
				       " background-position: center center;\n");

	headerLay->addWidget(btnCollapseMini);

	m_btnCollapse = new QPushButton(menuHeader);
	m_btnCollapse->setStyleSheet(" height:40px;\n"
				     " background-image: url(:/gui/icons/scopy-default/icons/logo.svg);\n"
				     " background-repeat: no-repeat;\n"
				     " background-position: center center;\n");
	headerLay->addWidget(m_btnCollapse);
	connect(m_btnCollapse, &QPushButton::clicked, this, &BrowseMenu::toggleCollapsed);
	connect(btnCollapseMini, &QPushButton::clicked, this, &BrowseMenu::toggleCollapsed);

	return menuHeader;
}

QLabel *BrowseMenu::createScopyLogo(QWidget *parent)
{
	QLabel *logo = new QLabel(m_content);
	logo->setStyleSheet("background-image: url(:/gui/icons/scopy-default/icons/logo_analog.svg);\n"
			    "background-repeat: no-repeat;\n"
			    "background-position: center center;\n"
			    "background-repeat: no-repeat;\n"
			    "min-height:36px;\n");

	logo->setObjectName(QString::fromUtf8("logo"));
	logo->setEnabled(true);
	logo->setMinimumSize(QSize(0, 36));
	return logo;
}
