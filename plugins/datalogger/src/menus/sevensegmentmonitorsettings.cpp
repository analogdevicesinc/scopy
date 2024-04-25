#include "menus/sevensegmentmonitorsettings.hpp"

#include <QBoxLayout>
#include <datamonitorstylehelper.hpp>
#include <datamonitorutils.hpp>
#include <menucollapsesection.h>
#include <menusectionwidget.h>

using namespace scopy;
using namespace datamonitor;

SevenSegmentMonitorSettings::SevenSegmentMonitorSettings(QWidget *parent)
	: QWidget{parent}
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(10);
	this->setLayout(layout);

	MenuSectionWidget *sevenSegmentSettingsContainer = new MenuSectionWidget(parent);
	MenuCollapseSection *sevenSegmentSettingsSection =
		new MenuCollapseSection("7 SEGMENT", MenuCollapseSection::MHCW_NONE, sevenSegmentSettingsContainer);

	sevenSegmentSettingsContainer->contentLayout()->addWidget(sevenSegmentSettingsSection);
	sevenSegmentSettingsSection->contentLayout()->setSpacing(10);

	QHBoxLayout *precisionLayout = new QHBoxLayout(this);

	precision = new QLineEdit(sevenSegmentSettingsSection);
	precision->setText(QString::number(DataMonitorUtils::getDefaultPrecision()));

	precisionLayout->addWidget(new QLabel("Precision: ", this));
	precisionLayout->addWidget(precision);

	connect(precision, &QLineEdit::returnPressed, this, &SevenSegmentMonitorSettings::changePrecision);
	connect(precision, &QLineEdit::textChanged, this, &SevenSegmentMonitorSettings::changePrecision);

	peakHolderToggle = new MenuOnOffSwitch(tr("Min/Max"), sevenSegmentSettingsSection, false);
	peakHolderToggle->onOffswitch()->setChecked(true);

	connect(peakHolderToggle->onOffswitch(), &QAbstractButton::toggled, this,
		&SevenSegmentMonitorSettings::peakHolderToggled);

	sevenSegmentSettingsSection->contentLayout()->addLayout(precisionLayout);
	sevenSegmentSettingsSection->contentLayout()->addWidget(peakHolderToggle);

	layout->addWidget(sevenSegmentSettingsContainer);

	DataMonitorStyleHelper::SevenSegmentMonitorMenuStyle(this);
}

void SevenSegmentMonitorSettings::changePrecision()
{
	// precision value can be between 0 and 9
	auto value = precision->text().toInt();
	if(value < 0) {
		precision->setText("0");
	}
	if(value >= 10) {
		precision->setText("9");
	}

	Q_EMIT precisionChanged(precision->text().toInt());
}
