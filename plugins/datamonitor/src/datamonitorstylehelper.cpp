#include "datamonitorstylehelper.hpp"

#include <stylehelper.h>

using namespace scopy;
using namespace datamonitor;

DataMonitorStyleHelper::DataMonitorStyleHelper(QObject *parent)
	: QObject{parent}
{}

void DataMonitorStyleHelper::DataMonitorToolStyle(DataMonitorTool *tool)
{

	tool->clearBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	tool->clearBtn->setStyleSheet(RemoveButtonStyle());

	QIcon icon1;
	icon1.addPixmap(Util::ChangeSVGColor(":/gui/icons/add.svg", "white", 1));
	tool->addMonitorButton->setIcon(icon1);

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
