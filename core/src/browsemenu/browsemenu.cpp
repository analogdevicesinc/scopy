#include "browsemenu/browsemenu.h"

#include <QPushButton>
#include <stylehelper.h>

using namespace scopy;

BrowseMenu::BrowseMenu(QWidget *parent)
	: QWidget(parent)
	, m_menu(nullptr)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setMargin(0);
	setLayout(lay);

	m_menu = new MenuWidget("SCOPY", QPen(), this);
	// TBD
	QScrollArea *menuScroll = dynamic_cast<QScrollArea *>(m_menu->layout()->itemAt(1)->widget());
	QVBoxLayout *scrollLay = dynamic_cast<QVBoxLayout *>(menuScroll->widget()->layout());
	for(int i = 0; i < scrollLay->count(); ++i) {
		QLayoutItem *layoutItem = scrollLay->itemAt(i);
		if(layoutItem->spacerItem()) {
			layoutItem->spacerItem()->changeSize(0, 0, QSizePolicy::Minimum, QSizePolicy::Preferred);
			break;
		}
	}

	QPushButton *homeBtn = new QPushButton(m_menu);
	homeBtn->setText("Home");
	homeBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	StyleHelper::BlueButton(homeBtn);
	m_menu->add(homeBtn, "homeBtn", MenuWidget::MA_TOPFIRST);
	connect(homeBtn, &QPushButton::clicked, this, [=]() { Q_EMIT requestTool("home"); });

	QFrame *firstLine = new QFrame(m_menu);
	firstLine->setFrameShape(QFrame::HLine);
	firstLine->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_menu->add(firstLine, "firstLine", MenuWidget::MA_TOPLAST);

	m_instrumentMenu = new InstrumentMenu(m_menu);
	m_instrumentMenu->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	m_menu->add(m_instrumentMenu, "instrumentMenu", MenuWidget::MA_TOPLAST);

	QFrame *lastLine = new QFrame(m_menu);
	lastLine->setFrameShape(QFrame::HLine);
	lastLine->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_menu->add(lastLine, "lastLine", MenuWidget::MA_BOTTOMFIRST);

	QWidget *saveLoadWidget = new QWidget(m_menu);
	saveLoadWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	saveLoadWidget->setLayout(new QHBoxLayout(saveLoadWidget));
	saveLoadWidget->layout()->setMargin(0);

	QPushButton *saveBtn = new QPushButton(m_menu);
	saveBtn->setText("Save");
	saveBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	QPushButton *loadBtn = new QPushButton(m_menu);
	loadBtn->setText("Load");
	loadBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	saveLoadWidget->layout()->addWidget(saveBtn);
	saveLoadWidget->layout()->addWidget(loadBtn);

	m_menu->add(saveLoadWidget, "saveLoad", MenuWidget::MA_BOTTOMLAST);

	QPushButton *preferencesBtn = new QPushButton(m_menu);
	preferencesBtn->setText("Preferences");
	preferencesBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_menu->add(preferencesBtn, "preferencesBtn", MenuWidget::MA_BOTTOMLAST);
	connect(preferencesBtn, &QPushButton::clicked, this, [=]() { Q_EMIT requestTool("preferences"); });

	QPushButton *aboutBtn = new QPushButton(m_menu);
	aboutBtn->setText("About");
	aboutBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_menu->add(aboutBtn, "aboutBtn", MenuWidget::MA_BOTTOMLAST);
	connect(aboutBtn, &QPushButton::clicked, this, [=]() { Q_EMIT requestTool("about"); });

	lay->addWidget(m_menu);
}

BrowseMenu::~BrowseMenu() {}

InstrumentMenu *BrowseMenu::instrumentMenu() const { return m_instrumentMenu; }
