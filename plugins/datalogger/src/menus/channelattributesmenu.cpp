#include "menus/channelattributesmenu.hpp"

#include <iiowidgetfactory.h>
#include <menuheader.h>

using namespace scopy;
using namespace datamonitor;

ChannelAttributesMenu::ChannelAttributesMenu(DataMonitorModel *model, QWidget *parent)
	: QWidget{parent}
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(10);
	setLayout(mainLayout);

	MenuHeaderWidget *header = new MenuHeaderWidget(model->getName(), model->getColor(), this);
	mainLayout->addWidget(header);

	QWidget *settingsBody = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout();
	layout->setMargin(0);
	layout->setSpacing(10);
	settingsBody->setLayout(layout);

	mainLayout->addLayout(layout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(settingsBody);
	mainLayout->addWidget(scrollArea);

	MenuSectionWidget *attrcontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *attr =
		new MenuCollapseSection("ATTRIBUTES", MenuCollapseSection::MHCW_NONE, attrcontainer);

	QVBoxLayout *attrLayout = new QVBoxLayout();
	attrLayout->setSpacing(10);
	attrLayout->setMargin(0);
	attrLayout->setContentsMargins(0, 0, 0, 10); // bottom margin

	if(qobject_cast<DmmDataMonitorModel *>(model)) {
		QList<IIOWidget *> attrWidgets = IIOWidgetFactory::buildAllAttrsForChannel(
			dynamic_cast<DmmDataMonitorModel *>(model)->iioChannel());

		for(auto w : attrWidgets) {
			attrLayout->addWidget(w);
		}

	} else {
		attrLayout->addWidget(new QLabel("NO ATTRIBUTE FOUND !"));
	}

	attr->contentLayout()->addLayout(attrLayout);
	attrcontainer->contentLayout()->addWidget(attr);

	layout->addWidget(attrcontainer);

	QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);
	layout->addItem(spacer);
}
