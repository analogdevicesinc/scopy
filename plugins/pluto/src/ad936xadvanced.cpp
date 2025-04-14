#include "ad963xadvanced.h"
#include "auxadcdaciowidget.h"
#include "elnawidget.h"
#include "ensmmodeclockswidget.h"
#include "gainwidget.h"
#include "rssiwidget.h"
#include "txmonitorwidget.h"
#include "miscwidget.h"

#include <bistwidget.h>
#include <iiowidgetbuilder.h>
#include <menuonoffswitch.h>
#include <style.h>
#include <toolbuttons.h>
#include <iioutil/connectionprovider.h>

using namespace scopy;
using namespace pluto;

AD936XAdvanced::AD936XAdvanced(QString uri, QWidget *parent)
	: m_uri(uri)
	, QWidget{parent}
{
	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setMargin(0);
	m_mainLayout->setContentsMargins(0, 0, 0, 0);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);

	m_mainLayout->addWidget(m_tool);

	///////// info button /////////////////
	InfoBtn *infoBtn = new InfoBtn(this, true);
	m_tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);

	connect(infoBtn, &InfoBtn::clicked, this, [=, this]() {
		infoBtn->generateInfoPopup(this);
		// TODO
		// connect(infoBtn->getTutorialButton(), &QPushButton::clicked, this, [=]() {
		// 	if(searchBarWidget->isVisible()) {
		// 		startTutorial();
		// 	} else {
		// 		startSimpleTutorial();
		// 	}
		// });

		// connect(infoBtn->getDocumentationButton(), &QAbstractButton::clicked, this, [=]() {
		// 	QDesktopServices::openUrl(
		// 		QUrl("https://analogdevicesinc.github.io/scopy/plugins/registermap/registermap.html"));
		// });
	});

	// main widget body

	QStackedWidget *centralWidget = new QStackedWidget(this);
	m_tool->addWidgetToCentralContainerHelper(centralWidget);

	QButtonGroup *navigationButtons = new QButtonGroup(this);
	navigationButtons->setExclusive(true);

	QPushButton *ensmModeClocksBtn = new QPushButton("ENSM/Mode/Clocks", this);
	Style::setStyle(ensmModeClocksBtn, style::properties::button::basicButton);

	EnsmModeClocksWidget *ensmModeClocks = new EnsmModeClocksWidget(m_uri, centralWidget);
	centralWidget->addWidget(ensmModeClocks);

	connect(ensmModeClocksBtn, &QPushButton::clicked, this,
		[=, this]() { centralWidget->setCurrentWidget(ensmModeClocks); });

	QPushButton *eLnaBtn = new QPushButton("eLNA", this);
	Style::setStyle(eLnaBtn, style::properties::button::basicButton);

	ElnaWidget *elna = new ElnaWidget(m_uri, centralWidget);
	centralWidget->addWidget(elna);

	connect(eLnaBtn, &QPushButton::clicked, this, [=, this]() { centralWidget->setCurrentWidget(elna); });

	QPushButton *rssiBtn = new QPushButton("RSSI", this);
	Style::setStyle(rssiBtn, style::properties::button::basicButton);

	RssiWidget *rssi = new RssiWidget(m_uri, centralWidget);
	centralWidget->addWidget(rssi);

	connect(rssiBtn, &QPushButton::clicked, this, [=, this]() { centralWidget->setCurrentWidget(rssi); });

	QPushButton *gainBtn = new QPushButton("GAIN", this);
	Style::setStyle(gainBtn, style::properties::button::basicButton);

	GainWidget *gainWidget = new GainWidget(m_uri, centralWidget);
	centralWidget->addWidget(gainWidget);

	connect(gainBtn, &QPushButton::clicked, this, [=, this]() { centralWidget->setCurrentWidget(gainWidget); });

	QPushButton *txMonitorBtn = new QPushButton("TX MONITOR", this);
	Style::setStyle(txMonitorBtn, style::properties::button::basicButton);

	TxMonitorWidget *txMonitor = new TxMonitorWidget(m_uri, centralWidget);
	centralWidget->addWidget(txMonitor);
	connect(txMonitorBtn, &QPushButton::clicked, this, [=, this]() { centralWidget->setCurrentWidget(txMonitor); });

	QPushButton *auxAdcDacIioBtn = new QPushButton("Aux ADC/DAC/IIO", this);
	Style::setStyle(auxAdcDacIioBtn, style::properties::button::basicButton);

	AuxAdcDacIoWidget *auxAdcDacIo = new AuxAdcDacIoWidget(m_uri, centralWidget);
	centralWidget->addWidget(auxAdcDacIo);
	connect(auxAdcDacIioBtn, &QPushButton::clicked, this,
		[=, this]() { centralWidget->setCurrentWidget(auxAdcDacIo); });

	QPushButton *miscBtb = new QPushButton("MISC", this);
	Style::setStyle(miscBtb, style::properties::button::basicButton);

	MiscWidget *misc = new MiscWidget(m_uri, centralWidget);
	centralWidget->addWidget(misc);

	connect(miscBtb, &QPushButton::clicked, this, [=, this]() { centralWidget->setCurrentWidget(misc); });

	QPushButton *bistBtn = new QPushButton("BIST", this);
	Style::setStyle(bistBtn, style::properties::button::basicButton);

	BistWidget *bist = new BistWidget(m_uri, centralWidget);
	centralWidget->addWidget(bist);
	connect(bistBtn, &QPushButton::clicked, this, [=, this]() { centralWidget->setCurrentWidget(bist); });

	navigationButtons->addButton(ensmModeClocksBtn);
	navigationButtons->addButton(eLnaBtn);
	navigationButtons->addButton(rssiBtn);
	navigationButtons->addButton(gainBtn);
	navigationButtons->addButton(txMonitorBtn);
	navigationButtons->addButton(auxAdcDacIioBtn);
	navigationButtons->addButton(miscBtb);
	navigationButtons->addButton(bistBtn);

	m_tool->addWidgetToTopContainerHelper(ensmModeClocksBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(eLnaBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(rssiBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(gainBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(txMonitorBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(auxAdcDacIioBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(miscBtb, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(bistBtn, TTA_LEFT);

	// Style::setStyle(scrollWidget, style::properties::widget::border_interactive, true, true);
}

AD936XAdvanced::~AD936XAdvanced()
{
	// close Connection
	ConnectionProvider::close(m_uri);
}
