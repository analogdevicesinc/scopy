#include "datamonitorstylehelper.hpp"

#include <stylehelper.h>

using namespace scopy::datamonitor;

DataMonitorStyleHelper::DataMonitorStyleHelper(QObject *parent)
	: QObject{parent}
{}

void DataMonitorStyleHelper::DataMonitorSettingsStyle(DataMonitorSettings *dataMonitorSettings)
{
	QString style = QString(R"css(
						QWidget {
							background-color : &&backgroundColor&& ;
						}

						)css");

	style.replace("&&backgroundColor&&", "transparent");

	dataMonitorSettings->setStyleSheet(style);
}

void DataMonitorStyleHelper::DataMonitorToolStyle(DatamonitorTool *tool)
{
	tool->clearBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	tool->clearBtn->setStyleSheet(RemoveButtonStyle());
	tool->textMonitors->setStyleSheet("color: white; font-size: 16px;");
	tool->tool->setRightContainerWidth(285);
	tool->tool->setLeftContainerWidth(185);
	tool->tool->centralContainer()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	StyleHelper::SquareToggleButtonWithIcon(tool->addMonitorButton, "add_monitor_btn", false);
}

void DataMonitorStyleHelper::SevenSegmentMonitorsStyle(SevenSegmentMonitor *sevenSegmentMonitor, QString lcdColor)
{
	QString style = QString(R"css(

						QWidget {
								background-color: &&childWidgetBackground&& ;
								height: 60px;
						}

						scopy--LcdNumber {
							background-color: transparent ;
							color : &&lcdColor&& ;
							border : 0px ;
						}
						.scopy--datamonitor--SevenSegmentMonitor:hover {
							border: 1px solid &&hoverBackground&& ;
							border-radius: 4px;
						}
						)css");

	style.replace("&&childWidgetBackground&&", StyleHelper::getColor("UIElementBackground"));
	style.replace("&&hoverBackground&&", StyleHelper::getColor("LabelText"));
	style.replace("&&lcdColor&&", lcdColor);

	sevenSegmentMonitor->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	StyleHelper::MenuLargeLabel(sevenSegmentMonitor->name);
	sevenSegmentMonitor->name->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	StyleHelper::MenuLargeLabel(sevenSegmentMonitor->unitOfMeasurement);
	sevenSegmentMonitor->unitOfMeasurement->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	sevenSegmentMonitor->lcdNumber->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	sevenSegmentMonitor->lcdNumber->setMinimumSize(150, 50);
	sevenSegmentMonitor->lcdNumber->setSegmentStyle(QLCDNumber::SegmentStyle::Flat);
	sevenSegmentMonitor->lcdNumberMin->setSegmentStyle(QLCDNumber::SegmentStyle::Flat);
	sevenSegmentMonitor->lcdNumberMax->setSegmentStyle(QLCDNumber::SegmentStyle::Flat);

	sevenSegmentMonitor->setStyleSheet(style);
}

void DataMonitorStyleHelper::SevenSegmentMonitorMenuStyle(SevenSegmentMonitorSettings *sevenSegmentMonitorSettings)
{
	sevenSegmentMonitorSettings->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void DataMonitorStyleHelper::DataLoggingMenuStyle(DataLoggingMenu *menu)
{
	StyleHelper::BlueButton(menu->dataLoggingBrowseBtn);
	StyleHelper::BlueButton(menu->dataLoggingBtn);
	StyleHelper::BlueButton(menu->dataLoadingBtn);
}

QString DataMonitorStyleHelper::RemoveButtonStyle()
{
	QString style = QString(R"css(
			QPushButton {
				width: 88px;
				height: 48px;
					border-radius: 2px;
					text-align: center;
					padding-left: 20px;
					padding-right: 20px;
					color: white;
					font-weight: 700;
					font-size: 14px;
			}

			QPushButton:!checked {
				background-color: &&backgroundColor&& ;
			}

			QPushButton:disabled {
				background-color: &&disabledColor&& ;
			})css");

	style.replace("&&backgroundColor&&", StyleHelper::getColor("CH6"));
	style.replace("&&disabledColor&&", "gray");

	return style;
}
