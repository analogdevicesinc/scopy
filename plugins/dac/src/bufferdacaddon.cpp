#include "bufferdacaddon.h"
#include "dacdatamodel.h"

#include <QHBoxLayout>
#include <QLabel>

using namespace scopy;
BufferDacAddon::BufferDacAddon(DacDataModel *model, QWidget *parent)
	: DacAddon("blue", parent)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	auto m_layout = new QHBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setSpacing(10);
	setLayout(m_layout);

	QLabel *name = new QLabel("BUFFER DAC ADDON", this);
	m_layout->addWidget(name);

	QString color;
	if (color == "") {
		// is disabled
		name->setText("DISABLED");
		color = "grey";
	}


	setStyleSheet("background-color:" + color);
}

BufferDacAddon::~BufferDacAddon()
{

}
