#include "dataloggergenericmenu.hpp"

#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "gui/customSwitch.hpp"

using namespace scopy::gui;

DataLoggerGenericMenu::DataLoggerGenericMenu(QWidget *parent):
	GenericMenu(parent)
{
	connect(this, &DataLoggerGenericMenu::historySizeIndexChanged, this, &DataLoggerGenericMenu::setHistorySize );
	connect(this, &DataLoggerGenericMenu::lineStyleIndexChanged, this, &DataLoggerGenericMenu::changeLineStyle );
}

DataLoggerGenericMenu::~DataLoggerGenericMenu()
{}

void DataLoggerGenericMenu::init(QString title, QColor* color)
{
	initInteractiveMenu();
	setMenuHeader(title,color,false);

	adiscope::gui::SubsectionSeparator *scaleSection = new adiscope::gui::SubsectionSeparator("Settings", false);
	insertSection(scaleSection);

	//scale
	QHBoxLayout *scaleLayout = new QHBoxLayout();
	scaleSWitch = new CustomSwitch(scaleSection->getContentWidget());
	scaleSWitch->setChecked(true);

	scaleLayout->addWidget(new QLabel("Scale",scaleSection->getContentWidget()));
	scaleLayout->addWidget(scaleSWitch);

	scaleSection->getContentWidget()->layout()->addItem(scaleLayout);

	////history
	adiscope::gui::SubsectionSeparator *historySection = new adiscope::gui::SubsectionSeparator("History", true);
	insertSection(historySection);

	historySwitch = new CustomSwitch(historySection->getContentWidget());
	historySwitch->setChecked(true);
	historySection->getContentWidget()->layout()->addWidget(historySwitch);
	historySection->getContentWidget()->layout()->setAlignment(historySwitch,Qt::AlignRight);

	historySize = new QComboBox(historySection->getContentWidget());
	historySize->addItem(QString("1s"));
	historySize->addItem(QString("10s"));
	historySize->addItem(QString("60s"));
	historySize->setCurrentIndex(1);

	QHBoxLayout *h1layout = new QHBoxLayout();

	h1layout->addWidget(new QLabel("History Size",historySection->getContentWidget()));
	h1layout->addWidget(historySize);

	historySection->getContentWidget()->layout()->addItem(h1layout);

	QHBoxLayout *h2layout = new QHBoxLayout();
	historyStyle = new QComboBox(historySection->getContentWidget());
	historyStyle->addItem(QString("Solid Line"));
	historyStyle->addItem(QString("Dot Line"));
	historyStyle->addItem(QString("Dash Line"));
	historyStyle->addItem(QString("Dash-Dot Line"));
	historyStyle->addItem(QString("Dash-Dot-Dot Line"));
	historyStyle->setCurrentIndex(0);

	h2layout->addWidget(new QLabel("History Style",historySection->getContentWidget()));
	h2layout->addWidget(historyStyle);

	historySection->getContentWidget()->layout()->addItem(h2layout);

	////Peak holder
	adiscope::gui::SubsectionSeparator *peakHolderSection = new adiscope::gui::SubsectionSeparator("Peak Holder", true);
	insertSection(peakHolderSection);

	QHBoxLayout *peakHolderLayout = new QHBoxLayout();
	peakHolderSwitch = new CustomSwitch(peakHolderSection->getContentWidget());
	peakHolderSwitch->setChecked(true);
	peakHolderReset = new QPushButton("Reset",peakHolderSection->getContentWidget());
	peakHolderReset->setProperty("blue_button",true);
	peakHolderReset->setMinimumHeight(30);

	peakHolderLayout->addWidget(peakHolderReset);
	peakHolderLayout->addWidget(peakHolderSwitch);

	peakHolderLayout->setAlignment(peakHolderSwitch,Qt::AlignRight);

	peakHolderSection->getContentWidget()->layout()->addItem(peakHolderLayout);

	layout()->addItem( new QSpacerItem(10,10, QSizePolicy::Fixed, QSizePolicy::Expanding));

	connect(scaleSWitch, &CustomSwitch::toggled, this, &DataLoggerGenericMenu::toggleScale);
	connect(historySwitch,  &CustomSwitch::toggled, this, &DataLoggerGenericMenu::toggleHistory);
	connect(peakHolderReset, &QPushButton::clicked,this, &DataLoggerGenericMenu::resetPeakHolder);
	connect(peakHolderSwitch, &CustomSwitch::toggled, this, &DataLoggerGenericMenu::togglePeakHolder);
	connect(historySize, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DataLoggerGenericMenu::historySizeIndexChanged);
	connect(historyStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DataLoggerGenericMenu::lineStyleIndexChanged);
}

void DataLoggerGenericMenu::scaleToggle(bool toggled)
{
	scaleSWitch->setChecked(toggled);
}

void DataLoggerGenericMenu::peakHolderToggle(bool toggled)
{
	peakHolderSwitch->setChecked(toggled);
}

void DataLoggerGenericMenu::historyToggle(bool toggled)
{
	historySwitch->setChecked(toggled);
}

void DataLoggerGenericMenu::peakHolderResetClicked()
{
	peakHolderReset->click();
}

void DataLoggerGenericMenu::historySizeChanged(int size)
{
	historySize->setCurrentIndex(size);
	setHistorySize(size);
}

void DataLoggerGenericMenu::changeLineStyle(int index)
{
	historyStyle->setCurrentIndex(index);
	Q_EMIT lineStyleChanged(lineStyleFromIdx(index));
}

void DataLoggerGenericMenu::setHistorySize(int idx)
{
	Q_EMIT changeHistorySize(numSamplesFromIdx(idx));
}

double DataLoggerGenericMenu::numSamplesFromIdx(int idx)
{
	switch(idx) {
	case 0:	return 1.0;
	case 1:	return 10.0;
	case 2:	return 60.0;
	default:throw std::runtime_error("Invalid INDEX");
	}
}

Qt::PenStyle DataLoggerGenericMenu::lineStyleFromIdx(int idx)
{
	switch (idx) {
	case 0:	return Qt::PenStyle::SolidLine;
	case 1:	return Qt::PenStyle::DotLine;
	case 2:	return Qt::PenStyle::DashLine;
	case 3:	return Qt::PenStyle::DashDotLine;
	case 4: return Qt::PenStyle::DashDotDotLine;
	}
	return Qt::PenStyle::SolidLine;
}

