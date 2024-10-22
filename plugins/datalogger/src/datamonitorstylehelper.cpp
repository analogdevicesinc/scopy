#include "datamonitorstylehelper.hpp"

#include <style.h>
#include <stylehelper.h>

using namespace scopy::datamonitor;

DataMonitorStyleHelper::DataMonitorStyleHelper(QObject *parent)
	: QObject{parent}
{}

void DataMonitorStyleHelper::DataMonitorToolStyle(DatamonitorTool *tool)
{
	tool->tool->setRightContainerWidth(300);
	tool->tool->setLeftContainerWidth(185);
	tool->tool->centralContainer()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	Style::setStyle(tool->textMonitors, style::properties::widget::textEditBigLabel, true, true);

	Style::setStyle(tool->clearBtn, style::properties::button::clear, true, true);
}

void DataMonitorStyleHelper::SevenSegmentMonitorsStyle(SevenSegmentMonitor *sevenSegmentMonitor, QString lcdColor)
{

	QString style = QString(R"css(
						scopy--LcdNumber {
							background-color: transparent ;
							color : &&lcdColor&& ;
							border : 0px ;
						}

						)css");

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
	sevenSegmentMonitor->layout->setAlignment(sevenSegmentMonitor->lcdNumber, Qt::AlignHCenter | Qt::AlignVCenter);

	sevenSegmentMonitor->setStyleSheet(style);

	Style::setStyle(sevenSegmentMonitor, style::properties::widget::basicComponent);
}

void DataMonitorStyleHelper::SevenSegmentMonitorMenuStyle(SevenSegmentMonitorSettings *sevenSegmentMonitorSettings)
{
	sevenSegmentMonitorSettings->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void DataMonitorStyleHelper::DataLoggingMenuStyle(DataLoggingMenu *menu)
{
	StyleHelper::BasicButton(menu->dataLoggingBrowseBtn);
	StyleHelper::BasicButton(menu->dataLoggingBtn);
	StyleHelper::BasicButton(menu->dataLoadingBtn);
}

#include "moc_datamonitorstylehelper.cpp"
