#include "monitorselectionmenu.hpp"

#include <QLabel>
#include <datamonitorstylehelper.hpp>
#include <menucontrolbutton.h>
#include <menusectionwidget.h>
#include <readabledatamonitormodel.hpp>

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

	deviceChannelsWidget = new QWidget(this);
	QVBoxLayout *deviceChannelsWidgetLayout = new QVBoxLayout(deviceChannelsWidget);
	deviceChannelsWidgetLayout->setMargin(0);
	deviceChannelsWidgetLayout->setSpacing(10);
	deviceChannelsWidget->setLayout(deviceChannelsWidgetLayout);

	importedChannelsWidget = new QWidget(this);
	QVBoxLayout *importedChannelsWidgetLayout = new QVBoxLayout(importedChannelsWidget);
	importedChannelsWidgetLayout->setMargin(0);
	importedChannelsWidgetLayout->setSpacing(10);
	importedChannelsWidget->setLayout(importedChannelsWidgetLayout);

	m_monitorsGroup = new SemiExclusiveButtonGroup(this);
	m_monitorsGroup->setExclusive(true);

	foreach(QString monitor, monitorList->keys()) {
		addMonitor(monitorList->value(monitor));
	}

	QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);

	layout->addWidget(deviceChannelsWidget);
	layout->addItem(spacer);
	layout->addWidget(importedChannelsWidget);
}

void MonitorSelectionMenu::generateDeviceSection(QString device, bool import)
{
	MenuCollapseSection *devMonitorsSection = new MenuCollapseSection(device, MenuCollapseSection::MHCW_NONE, this);

	if(import) {

		QPushButton *removeBtn = new QPushButton(devMonitorsSection);
		removeBtn->setMaximumSize(25, 25);
		removeBtn->setIcon(QIcon(":/gui/icons/orange_close.svg"));

		HoverWidget *removeHover = new HoverWidget(removeBtn, devMonitorsSection, devMonitorsSection);
		removeHover->setStyleSheet("background-color: transparent; border: 0px;");
		removeHover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
		removeHover->setContentPos(HoverPosition::HP_CENTER);
		removeHover->setAnchorOffset(QPoint(-20, 20));
		removeHover->setVisible(true);
		removeHover->raise();

		connect(removeBtn, &QPushButton::clicked, this,
			[=, this]() { Q_EMIT requestRemoveImportedDevice(device); });

		importedChannelsWidget->layout()->addWidget(devMonitorsSection);
	} else {
		deviceChannelsWidget->layout()->addWidget(devMonitorsSection);
	}

	DataMonitorStyleHelper::MonitorSelectionMenuMenuCollapseSectionStyle(devMonitorsSection);

	devMonitorsSection->header()->setChecked(false);

	deviceMap.insert(device, devMonitorsSection);
}

void MonitorSelectionMenu::addMonitor(DataMonitorModel *monitor)
{
	if(!deviceMap.contains(monitor->getDeviceName())) {
		generateDeviceSection(monitor->getDeviceName(), !qobject_cast<ReadableDataMonitorModel *>(monitor));
	}

	MenuControlButton *monitorChannel = new MenuControlButton(deviceMap.value(monitor->getDeviceName()));
	monitorChannel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	deviceMap.value(monitor->getDeviceName())->contentLayout()->addWidget(monitorChannel);
	monitorChannel->setName(monitor->getShortName());
	monitorChannel->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	monitorChannel->setOpenMenuChecksThis(true);
	monitorChannel->setColor(monitor->getColor());
	monitorChannel->button()->setVisible(false);
	monitorChannel->setCheckable(true);

	monitorChannel->setToolTip(monitor->getName());

	m_monitorsGroup->addButton(monitorChannel);

	// apply hover to the buttons based on the color they have
	monitorChannel->setStyleSheet(monitorChannel->styleSheet() +
				      QString(":hover{ background-color: %1 ; }").arg(monitor->getColor().name()));

	connect(monitorChannel, &MenuControlButton::clicked, this, [=, this]() {
		if(!monitorChannel->checkBox()->isChecked()) {
			monitorChannel->checkBox()->setChecked(true);
		}
		Q_EMIT requestMonitorMenu(monitor->getName());
	});

	connect(monitorChannel->checkBox(), &QCheckBox::toggled, this,
		[=, this](bool toggled) { Q_EMIT monitorToggled(toggled, monitor->getName()); });

	// when removing the monitor disable all active monitors
	connect(this, &MonitorSelectionMenu::removeMonitor, monitorChannel, [=, this]() {
		if(monitorChannel->checkBox()->isChecked()) {
			Q_EMIT monitorToggled(false, monitor->getName());
		}
	});
}

void MonitorSelectionMenu::removeDevice(QString device)
{
	if(deviceMap.contains(device)) {
		delete deviceMap.value(device);
		deviceMap.remove(device);
	}
}

SemiExclusiveButtonGroup *MonitorSelectionMenu::monitorsGroup() const { return m_monitorsGroup; }

void MonitorSelectionMenu::setMonitorsGroup(SemiExclusiveButtonGroup *newMonitorsGroup)
{
	m_monitorsGroup = newMonitorsGroup;
}
