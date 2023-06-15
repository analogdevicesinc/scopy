#include "pluginentry.h"
#include "qboxlayout.h"
#include "qpainter.h"

#include <qstyleoption.h>

using namespace scopy;

PluginEntry::PluginEntry(QWidget *parent)
	: QWidget(parent)
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(5, 5, 5, 5);

	m_checkBox = new QCheckBox();
	m_checkBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	m_descriptionLabel = new QLabel();
	m_descriptionLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	m_descriptionLabel->setWordWrap(true);

	layout->addWidget(m_checkBox);
	layout->addWidget(m_descriptionLabel);
	layout->setStretch(0,1);
	layout->setStretch(1,3);

	this->setStyleSheet("background-color: black;");
	this->setAutoFillBackground(true);
}

PluginEntry::~PluginEntry()
{}

void PluginEntry::setDescription(QString description)
{
	m_descriptionLabel->clear();
	m_descriptionLabel->setText(description);
}

QCheckBox *PluginEntry::checkBox() const
{
	return m_checkBox;
}

void PluginEntry::paintEvent(QPaintEvent *e) {
	// https://forum.qt.io/topic/25142/solved-applying-style-on-derived-widget-with-custom-property-failes/2
	// https://doc.qt.io/qt-5/stylesheet-reference.html
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
