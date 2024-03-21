#include "guidetailsview.h"

using namespace scopy::iiodebugplugin;

GuiDetailsView::GuiDetailsView(QWidget *parent)
	: QWidget(parent)
	, m_attrSeparator(new gui::SubsectionSeparator(this))
	, m_detailsSeparator(new gui::SubsectionSeparator(this))
	, m_scrollArea(new QScrollArea(this))
	, m_scrollAreaContents(new QWidget(this))
{
	setupUi();
}

void GuiDetailsView::setupUi()
{
	setContentsMargins(0, 0, 0, 0);
	setLayout(new QVBoxLayout(this));

	m_scrollAreaContents->setLayout(new QVBoxLayout(m_scrollAreaContents));
	m_scrollAreaContents->setObjectName("DetailsViewScrollAreaContents");
	m_scrollAreaContents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	m_scrollArea->setWidget(m_scrollAreaContents);
	m_scrollArea->setWidgetResizable(true);

	m_attrSeparator->setLabel("Attributes");
	m_attrSeparator->getContentWidget()->layout()->addWidget(m_scrollArea);
	m_attrSeparator->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

	m_detailsSeparator->setLabel("Device info");
	m_detailsSeparator->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	layout()->addWidget(m_attrSeparator);
	layout()->addWidget(m_detailsSeparator);
}

void GuiDetailsView::setIIOStandardItem(IIOStandardItem *item)
{
	m_currentItem = item;
	clearWidgets();
	auto iioWidgets = item->getIIOWidgets();
	for(auto widget : iioWidgets) {
		widget->show();
		m_currentWidgets.append(widget);
		m_scrollAreaContents->layout()->addWidget(widget);
	}

	auto details = item->details();
	for(const auto &detail : details) {
		auto label = new QLabel(detail);
		label->show();
		m_detailsList.append(label);
		m_detailsSeparator->getContentWidget()->layout()->addWidget(label);
	}
}

void GuiDetailsView::clearWidgets()
{
	QLayoutItem *child;
	while((child = m_scrollAreaContents->layout()->takeAt(0)) != nullptr) {
		child->widget()->hide();
		m_scrollAreaContents->layout()->removeWidget(child->widget());
	}
	m_currentWidgets.clear();

	while((child = m_detailsSeparator->getContentWidget()->layout()->takeAt(0)) != nullptr) {
		child->widget()->hide();
		m_detailsSeparator->getContentWidget()->layout()->removeWidget(child->widget());
	}
	m_detailsList.clear();
}
