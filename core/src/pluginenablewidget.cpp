#include "pluginenablewidget.h"

#include "qboxlayout.h"

using namespace scopy;

PluginEnableWidget::PluginEnableWidget(QWidget *parent)
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
	layout->setAlignment(m_checkBox, Qt::AlignTop);
	layout->addWidget(m_descriptionLabel);
	layout->setAlignment(m_descriptionLabel, Qt::AlignTop);
	layout->setStretch(0, 1);
	layout->setStretch(1, 3);
}

PluginEnableWidget::~PluginEnableWidget() {}

void PluginEnableWidget::setDescription(QString description)
{
	m_descriptionLabel->clear();
	m_descriptionLabel->setText(description);
}

QCheckBox *PluginEnableWidget::checkBox() const { return m_checkBox; }

#include "moc_pluginenablewidget.cpp"
