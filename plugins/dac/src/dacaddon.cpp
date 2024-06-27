#include "dacaddon.h"

#include <QHBoxLayout>
#include <QLabel>

using namespace scopy;
DacAddon::DacAddon(QString color, QWidget *parent)
	: QWidget(parent)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//	auto m_layout = new QHBoxLayout(this);
//	m_layout->setMargin(0);
//	m_layout->setSpacing(10);
//	setLayout(m_layout);

//	QLabel *name = new QLabel("TEST", this);
//	m_layout->addWidget(name);

//	if (color == "") {
//		// is disabled
//		name->setText("DISABLED");
//		color = "grey";
//	}


//	setStyleSheet("background-color:" + color);
}

DacAddon::~DacAddon()
{

}
