#include "plotinfo.h"

#include <QLabel>
#include <stylehelper.h>

using namespace scopy;

PlotInfo::PlotInfo(QWidget *parent)
	: QWidget(parent)
	, m_parent(parent)
	, m_margin(6)
	, m_spacing(6)
	, m_leftInfo(new QWidget())
	, m_rightInfo(new QWidget())
{
	initLayouts();
}

PlotInfo::~PlotInfo() {}

void PlotInfo::addCustomInfo(QWidget *info, InfoPosition hpos, InfoPosition vpos)
{
	switch(hpos) {
	case InfoPosition::IP_LEFT:
		if(vpos == IP_BOTTOM) {
			m_leftLayout->addWidget(info);
		} else {
			m_leftLayout->insertWidget(0,info);
		}
		info->setParent(m_leftInfo);
		break;

	case InfoPosition::IP_RIGHT:
	default:
		if(vpos == IP_BOTTOM) {
			m_rightLayout->addWidget(info);
		} else {
			m_rightLayout->insertWidget(0,info);
		}
		info->setParent(m_rightInfo);

		// align to right if it's a label
		QLabel *labelInfo = dynamic_cast<QLabel *>(info);
		if(labelInfo) {
			labelInfo->setAlignment(Qt::AlignRight);
		}
		break;
	}
	info->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

QLabel *PlotInfo::addLabelInfo(InfoPosition hpos,  InfoPosition vpos)
{
	QLabel *label = new QLabel();
	StyleHelper::PlotInfoLabel(label);
	addCustomInfo(label, hpos, vpos);

	return label;
}

void PlotInfo::removeInfo(uint index, InfoPosition pos)
{
	QWidget *widget = getInfo(index, pos);
	if(widget) {
		switch(pos) {
		case InfoPosition::IP_LEFT:
			m_leftLayout->removeWidget(widget);
			break;

		case InfoPosition::IP_RIGHT:
		default:
			m_rightLayout->removeWidget(widget);
			break;
		}
		widget->setParent(nullptr);
	}
}

QWidget *PlotInfo::getInfo(uint index, InfoPosition pos)
{
	switch(pos) {
	case InfoPosition::IP_LEFT:
		if(index >= m_leftLayout->count())
			return nullptr;
		return m_leftLayout->itemAt(index)->widget();

	case InfoPosition::IP_RIGHT:
	default:
		if(index >= m_rightLayout->count())
			return nullptr;
		return m_rightLayout->itemAt(index)->widget();
	}

	return nullptr;
}

void PlotInfo::initLayouts()
{
	// left info
	m_leftInfo->setAttribute(Qt::WA_TransparentForMouseEvents);
	m_leftLayout = new QVBoxLayout(m_leftInfo);
	m_leftLayout->setSpacing(m_spacing);
	m_leftLayout->setMargin(m_margin);

	m_leftHover = new HoverWidget(m_leftInfo, m_parent, m_parent);
	m_leftHover->setAnchorPos(HoverPosition::HP_TOPLEFT);
	m_leftHover->setContentPos(HoverPosition::HP_BOTTOMRIGHT);
	m_leftHover->setAttribute(Qt::WA_TransparentForMouseEvents);
	m_leftHover->show();

	// right info
	m_rightInfo->setAttribute(Qt::WA_TransparentForMouseEvents);
	m_rightLayout = new QVBoxLayout(m_rightInfo);
	m_rightLayout->setSpacing(m_spacing);
	m_rightLayout->setMargin(m_margin);

	m_rightHover = new HoverWidget(m_rightInfo, m_parent, m_parent);
	m_rightHover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	m_rightHover->setContentPos(HoverPosition::HP_BOTTOMLEFT);
	m_rightHover->setAttribute(Qt::WA_TransparentForMouseEvents);
	m_rightHover->show();
}

#include "moc_plotinfo.cpp"
