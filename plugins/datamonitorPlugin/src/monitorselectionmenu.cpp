#include "monitorselectionmenu.hpp"

#include <QLabel>
#include <menucontrolbutton.h>
#include <menusectionwidget.h>

using namespace scopy;
using namespace datamonitor;

MonitorSelectionMenu::MonitorSelectionMenu(QMap<QString, DataMonitorModel *> *monitorList, QWidget *parent)
	: QWidget{parent}
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(10);
	setLayout(mainLayout);

	QWidget *settingsBody = new QWidget(this);
	layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(10);
	settingsBody->setLayout(layout);

	mainLayout->addLayout(layout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(settingsBody);
	mainLayout->addWidget(scrollArea);

	monitorsGroup = new QButtonGroup(this);

	foreach(QString monitor, monitorList->keys()) {
		addMonitor(monitor, monitorList->value(monitor)->getColor());
	}

	QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);
	layout->addItem(spacer);
}

void MonitorSelectionMenu::generateDeviceSection(QString device)
{
	MenuSectionWidget *devMontirosContainer = new MenuSectionWidget(this);

	MenuCollapseSection *devMonitorsSection =
		new MenuCollapseSection(device, MenuCollapseSection::MHCW_NONE, devMontirosContainer);
	devMontirosContainer->contentLayout()->addWidget(devMonitorsSection);
	layout->addWidget(devMontirosContainer);

	devMonitorsSection->header()->setChecked(false);

	deviceMap.insert(device, devMonitorsSection);
}

void MonitorSelectionMenu::addMonitor(QString monitor, QColor monitorColor)
{
	auto dev = monitor.split(":")[0];

	if(!deviceMap.contains(dev)) {
		generateDeviceSection(dev);
	}

	MenuControlButton *monitorChannel = new MenuControlButton(deviceMap.value(dev));
	monitorChannel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	deviceMap.value(dev)->contentLayout()->addWidget(monitorChannel);
	monitorChannel->setName(monitor);
	monitorChannel->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	monitorChannel->setOpenMenuChecksThis(true);
	monitorChannel->setColor(monitorColor);
	monitorChannel->button()->setVisible(false);
	monitorChannel->setCheckable(false);

	monitorChannel->setToolTip(monitor);

	monitorsGroup->addButton(monitorChannel);

	// apply hover to the buttons based on the color they have
	monitorChannel->setStyleSheet(QString(":hover{ background-color: %1 ; }").arg(monitorColor.name()));

	connect(monitorChannel, &MenuControlButton::clicked, monitorChannel->checkBox(), &QCheckBox::toggle);

	connect(monitorChannel->checkBox(), &QCheckBox::toggled, this,
		[=, this](bool toggled) { Q_EMIT monitorToggled(toggled, monitor); });

	// when removing the monitor disable all active monitors
	connect(this, &MonitorSelectionMenu::removeMonitor, monitorChannel, [=, this]() {
		if(monitorChannel->checkBox()->isChecked()) {
			Q_EMIT monitorToggled(false, monitor);
		}
	});
}
