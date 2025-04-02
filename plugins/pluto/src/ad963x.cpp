#include "ad963x.h"

#include <QLabel>
#include <QTabWidget>
#include <menucombo.h>
#include <toolbuttons.h>
#include <utils.h>

using namespace scopy::pluto;

AD936X::AD936X(QWidget *parent) {

	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setMargin(0);
	m_mainLayout->setContentsMargins(0, 0, 0, 0);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);

	m_mainLayout->addWidget(m_tool);

	///////// info button /////////////////
	InfoBtn *infoBtn = new InfoBtn(this, true);
	m_tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);

	connect(infoBtn, &InfoBtn::clicked, this, [=]() {
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

	QTabWidget *tabWidget = new QTabWidget(this);

	m_controlsWidget = new QWidget(tabWidget);
	QVBoxLayout *controlsLayout = new QVBoxLayout(m_controlsWidget);
	m_controlsWidget->setLayout(controlsLayout);

	m_blockDiagramWidget = new QWidget(tabWidget);
	QVBoxLayout *blockDiagramLayout = new QVBoxLayout(m_blockDiagramWidget);
	m_blockDiagramWidget->setLayout(blockDiagramLayout);

	QLabel *tab1Content = new QLabel("TAB 1", m_controlsWidget);
	controlsLayout->addWidget(tab1Content);
	/// this tab will have 4 main wigets each of them in a separate class ??

	QWidget *controlsWidget = new QWidget(tabWidget);
	QVBoxLayout *controlWidgetLayout = new QVBoxLayout(controlsWidget);
	controlsWidget->setLayout(controlWidgetLayout);

	///  fist widget the global settings can be created with iiowigets only ??

	/// second is Rx ( receive chain)

	/// third is Tx (transimt chain)

	/// FPGA Settings


	QLabel *tab2Content = new QLabel("TAB 2", m_blockDiagramWidget);
	blockDiagramLayout->addWidget(tab2Content);
	QPixmap pixmap(":/pluto/ad936x.svg"); // Use the resource path
	tab2Content->setPixmap(pixmap);
	// tab2Content->setAlignment(Qt::AlignCenter);

	tabWidget->addTab(m_controlsWidget, "Controls");
	tabWidget->addTab(m_blockDiagramWidget, "Block Diagram");

	m_tool->addWidgetToCentralContainerHelper(tabWidget);


}

AD936X::~AD936X() {}

QWidget *AD936X::generateGlobalSettingsWidget(QWidget *parent)
{
	QWidget *globalSettingsWidget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout();
	globalSettingsWidget->setLayout(layout);

	//// ENSM

	MenuCombo *cbEnsmModeW = new MenuCombo("ENSM Mode", globalSettingsWidget);
	QComboBox *cbEnsmMode = cbEnsmModeW->combo();

	layout->addWidget(cbEnsmModeW);

}
