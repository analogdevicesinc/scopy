#include "datamonitorstylehelper.hpp"

#include <stylehelper.h>

using namespace scopy;
using namespace datamonitor;

DataMonitorStyleHelper::DataMonitorStyleHelper(QObject *parent)
	: QObject{parent}
{}

void DataMonitorStyleHelper::DataMonitorViewStyle(DataMonitorView *dataMonitorView, QColor color)
{
	QString style = QString(R"css(
						.scopy--datamonitor--DataMonitorView {
						margin : 16px ;
						padding: 16px ;
						}

						.scopy--datamonitor--DataMonitorView:hover {
							border: 1px solid &&hoverBackground&& ;
							border-radius: 4px;
						}

						*[is_selected=true] {
							border: 1px solid &&hoverBackground&& ;
							border-radius: 4px;
						}

						QWidget {
							color: &&widgetColor&& ;
							background-color : &&backgroundColor&& ;
						}
						QLCDNumber {
							color: &&LCDColor&& ;
							border : 0px ; }

						)css");

	style.replace("&&LCDColor&&", color.name());
	style.replace("&&hoverBackground&&", StyleHelper::getColor("LabelText"));
	style.replace("&&widgetColor&&", StyleHelper::getColor("LabelText"));
	style.replace("&&borderColor&&", StyleHelper::getColor("UIElementHighlight"));
	style.replace("&&backgroundColor&&", StyleHelper::getColor("UIElementBackground"));

	dataMonitorView->removeBtn->setMaximumSize(25, 25);
	dataMonitorView->removeBtn->setIcon(QIcon(":/gui/icons/orange_close.svg"));

	StyleHelper::MenuLargeLabel(dataMonitorView->m_measuringUnit);

	dataMonitorView->m_monitorPlot->setMinimumHeight(300);
	dataMonitorView->m_monitorPlot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	dataMonitorView->m_monitorPlot->m_plot->setStyleSheet("color: rgba(192,192,192,255)");

	dataMonitorView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	dataMonitorView->setMinimumWidth(500);

	dataMonitorView->setStyleSheet(style);
}

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

void DataMonitorStyleHelper::DataMonitorToolSettingsStyle(DataMonitorToolSettings *dataMonitorSettings)
{
	QString style = QString(R"css(
						QWidget {
							background-color : &&backgroundColor&& ;
						}

						)css");

	style.replace("&&backgroundColor&&", "transparent");

	StyleHelper::BlueButton(dataMonitorSettings->dataLoggingBrowseBtn);
	StyleHelper::BlueButton(dataMonitorSettings->dataLoggingBtn);

	dataMonitorSettings->setStyleSheet(style);
}

void DataMonitorStyleHelper::DataMonitorToolStyle(DataMonitorTool *tool)
{
	tool->clearBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	tool->clearBtn->setStyleSheet(RemoveButtonStyle());

	StyleHelper::SquareToggleButtonWithIcon(tool->addMonitorButton, "add_monitor_btn", false);
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
