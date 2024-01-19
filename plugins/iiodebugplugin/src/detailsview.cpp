#include "detailsview.h"
#include <QVBoxLayout>
#include <QLoggingCategory>

using namespace scopy::iiodebugplugin;

DetailsView::DetailsView(QWidget *parent)
	: QWidget(parent)
	, m_titleLabel(new QLabel("Select an IIO item.", this))
	, m_topSeparator(new gui::SubsectionSeparator(this))
{
	setLayout(new QVBoxLayout(this));

	m_titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_titleLabel->setStyleSheet("color: white;");
	m_titleLabel->setAlignment(Qt::AlignCenter);

	// FIXME: once this get in scopy, fix some of this style
	//	m_topSeparator->setButtonVisible(false);
	m_topSeparator->setLabelVisible(false);
	m_topSeparator->setLabel("Device info.");
	m_topSeparator->getContentWidget()->layout()->addWidget(new QLabel("The device is buffer capable."));

	layout()->addWidget(m_titleLabel);
	layout()->addWidget(m_topSeparator);
	layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));
}

void DetailsView::setTitle(QString title)
{
	int dataIndex = title.indexOf('(');
	if(dataIndex != -1) {
		title.remove(dataIndex, title.length() - dataIndex);
	}
	m_titleLabel->setText(title);
}

void DetailsView::setIIOStandardItem(IIOStandardItem *item) { setTitle(item->text()); }
