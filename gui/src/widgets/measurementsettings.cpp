#include "measurementsettings.h"

#include <widgets/menusectionwidget.h>
#include <widgets/menuonoffswitch.h>

using namespace scopy;

MeasurementSettings::MeasurementSettings(QWidget *parent) {

	QVBoxLayout *lay = new QVBoxLayout(this);
	setLayout(lay);
	//		setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	lay->setMargin(0);

	MenuSectionWidget *measureSection = new MenuSectionWidget(this);
	MenuOnOffSwitch *measurePanelSwitch = new MenuOnOffSwitch("Measure Panel", this);
	measurePanelSwitch->onOffswitch()->setChecked(true);
	QHBoxLayout *hlay1 = new QHBoxLayout(measureSection);
	hlay1->setContentsMargins(0,6,0,6);
	hlay1->setSpacing(9);
	measureSection->contentLayout()->addWidget(measurePanelSwitch);
	measureSection->contentLayout()->addLayout(hlay1);

	QPushButton *showAllMeasure = new QPushButton("Show All", measureSection);
	StyleHelper::SmallBlueButton(showAllMeasure);

	QPushButton *hideAllMeasure = new QPushButton("Hide All", measureSection);
	StyleHelper::SmallBlueButton(hideAllMeasure);

	hideAllMeasure->setVisible(false);

	connect(measurePanelSwitch->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		Q_EMIT enableMeasurementPanel(b);
	});
	connect(showAllMeasure, &QPushButton::clicked, this, [=](){
		Q_EMIT toggleAllMeasurements(true);
		showAllMeasure->setVisible(false);
		hideAllMeasure->setVisible(true);
	});

	connect(hideAllMeasure, &QPushButton::clicked, this, [=](){
		Q_EMIT toggleAllMeasurements(false);
		hideAllMeasure->setVisible(false);
		showAllMeasure->setVisible(true);
	});
	hlay1->addWidget(showAllMeasure);
	hlay1->addWidget(hideAllMeasure);

	QHBoxLayout *hlay2 = new QHBoxLayout(measureSection);
	hlay2->setContentsMargins(0,6,0,6);
	hlay2->setSpacing(9);
	measureSection->contentLayout()->addLayout(hlay2);

	QPushButton *mesaureSortByChannel = new QPushButton("Sort by channel", measureSection);
	StyleHelper::SmallBlueButton(mesaureSortByChannel);

	QPushButton *measureSortByType = new QPushButton("Sort by type", measureSection);
	StyleHelper::SmallBlueButton(measureSortByType);

	mesaureSortByChannel->setVisible(false);

	connect(mesaureSortByChannel, &QPushButton::clicked, this, [=](){

		Q_EMIT sortMeasurements(MPM_SORT_CHANNEL);
		mesaureSortByChannel->setVisible(false);
		measureSortByType->setVisible(true);
	});

	connect(measureSortByType, &QPushButton::clicked, this, [=](){
		Q_EMIT sortMeasurements(MPM_SORT_TYPE);
		measureSortByType->setVisible(false);
		mesaureSortByChannel->setVisible(true);
	});
	hlay2->addWidget(mesaureSortByChannel);
	hlay2->addWidget(measureSortByType);


	MenuSectionWidget *statsSection = new MenuSectionWidget(this);
	MenuOnOffSwitch *statsPanelSwitch = new MenuOnOffSwitch("Stats Panel", this);
	connect(statsPanelSwitch->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		Q_EMIT enableStatsPanel(b);
	});
	statsSection->contentLayout()->addWidget(statsPanelSwitch);

	statsPanelSwitch->onOffswitch()->setChecked(false);

	QHBoxLayout *hlay3 = new QHBoxLayout(statsSection);
	hlay3->setContentsMargins(0,6,0,6);
	hlay3->setSpacing(9);
	statsSection->contentLayout()->addWidget(statsPanelSwitch);
	statsSection->contentLayout()->addLayout(hlay3);

	QPushButton *showAllStats= new QPushButton("Show All", statsSection);
	StyleHelper::SmallBlueButton(showAllStats);

	QPushButton *hideAllStats= new QPushButton("Hide All", statsSection);
	StyleHelper::SmallBlueButton(hideAllStats);

	hideAllStats->setVisible(false);

	connect(statsPanelSwitch->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		Q_EMIT enableStatsPanel(b);
	});
	connect(showAllStats, &QPushButton::clicked, this, [=](){
		Q_EMIT toggleAllStats(true);
		showAllStats->setVisible(false);
		hideAllStats->setVisible(true);
	});

	connect(hideAllStats, &QPushButton::clicked, this, [=](){
		Q_EMIT toggleAllStats(false);
		hideAllStats->setVisible(false);
		showAllStats->setVisible(true);
	});
	hlay3->addWidget(showAllStats);
	hlay3->addWidget(hideAllStats);

	QHBoxLayout *hlay4 = new QHBoxLayout(statsSection);
	hlay4->setContentsMargins(0,6,0,6);
	hlay4->setSpacing(9);
	statsSection->contentLayout()->addLayout(hlay4);

	QPushButton *statsSortByChannel = new QPushButton("Sort by channel", statsSection);
	StyleHelper::SmallBlueButton(statsSortByChannel);

	QPushButton *statsSortByType = new QPushButton("Sort by type", statsSection);
	StyleHelper::SmallBlueButton(statsSortByType);

	statsSortByChannel->setVisible(false);

	connect(statsSortByChannel, &QPushButton::clicked, this, [=](){

		Q_EMIT sortStats(MPM_SORT_CHANNEL);
		statsSortByChannel->setVisible(false);
		statsSortByType->setVisible(true);
	});

	connect(statsSortByType, &QPushButton::clicked, this, [=](){
		Q_EMIT sortStats(MPM_SORT_TYPE);
		statsSortByType->setVisible(false);
		statsSortByChannel->setVisible(true);
	});
	hlay4->addWidget(statsSortByChannel);
	hlay4->addWidget(statsSortByType);

	lay->addWidget(measureSection);
	lay->addWidget(statsSection);
}

MeasurementSettings::~MeasurementSettings() {}

#include "moc_measurementsettings.cpp"
