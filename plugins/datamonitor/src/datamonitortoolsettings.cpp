#include "datamonitortoolsettings.hpp"

#include <datamonitorutils.hpp>
#include <menucollapsesection.h>
#include <menuheader.h>
#include <menusectionwidget.h>
#include <qboxlayout.h>
#include <spinbox_a.hpp>
#include <stylehelper.h>

using namespace scopy;
using namespace datamonitor;

DataMonitorToolSettings::DataMonitorToolSettings(QWidget *parent)
	: QWidget{parent}
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(10);
	setLayout(mainLayout);

	MenuHeaderWidget *header =
		new MenuHeaderWidget(QString("Settings"), QPen(StyleHelper::getColor("ScopyBlue")), this);
	mainLayout->addWidget(header);

	MenuSectionWidget *readIntervalContainer = new MenuSectionWidget(parent);
	MenuCollapseSection *readIntervalSection =
		new MenuCollapseSection("Read interval", MenuCollapseSection::MHCW_NONE, readIntervalContainer);

	PositionSpinButton *readInterval = new PositionSpinButton(
		{
			{"s", 1},
			{"min", 60},
		},
		"", 0, (double)((long)1 << 31), false, false, readIntervalSection);

	readInterval->setValue(DataMonitorUtils::getReadIntervalDefaul());

	readIntervalSection->contentLayout()->addWidget(readInterval);
	readIntervalContainer->contentLayout()->addWidget(readIntervalSection);
	mainLayout->addWidget(readIntervalContainer);

	QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);
	mainLayout->addItem(spacer);

	connect(readInterval, &PositionSpinButton::valueChanged, this, &DataMonitorToolSettings::readIntervalChanged);
}
