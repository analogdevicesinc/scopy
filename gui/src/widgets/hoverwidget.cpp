#include "widgets/hoverwidget.h"
#include "stylehelper.h"
#include <QDebug>
#include <QLoggingCategory>
#include <stylehelper.h>
using namespace scopy;
Q_LOGGING_CATEGORY(CAT_HOVERWIDGET, "HoverWidget")

HoverWidget::HoverWidget(QWidget *content, QWidget *anchor, QWidget *parent)
	: QWidget(parent), m_parent(parent), m_anchor(anchor), m_content(content),
	m_anchorPos(HP_TOPLEFT), m_contentPos(HP_TOPRIGHT), is_dragging(false), m_draggable(false) {
	m_container = new QWidget(this);
	StyleHelper::TransparentWidget(this, "hoverWidget");
	StyleHelper::TransparentWidget(m_container, "hoverWidgetContainer");

	m_lay = new QHBoxLayout(m_container);
	m_lay->setMargin(0);
	m_lay->setContentsMargins(0, 0, 0, 0);
	m_lay->setSizeConstraint(QBoxLayout::SetMinimumSize);
	m_container->setLayout(m_lay);
	m_lay->addWidget(content);

	if (m_content) {
		setContent(m_content);
	}
	if (m_anchor) {
		setAnchor(m_anchor);
	}
	if (m_parent) {
		setParent(m_parent);
	}

	hide();
}

void HoverWidget::setDraggable(bool draggable)
{
	m_draggable = draggable;
	StyleHelper::HoverWidget(m_container, m_draggable);
}

void HoverWidget::mousePressEvent(QMouseEvent *event)
{
	raise();
	if (event->button() == Qt::LeftButton &&
			m_container->geometry().contains(event->pos()) &&
			m_draggable) {
		is_dragging = true;
		mouse_pos = event->pos();
	}
}

void HoverWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton &&
			is_dragging) {
		is_dragging = false;
		mouse_pos = QPoint();
	}
}

void HoverWidget::mouseMoveEvent(QMouseEvent *event)
{
	QPoint new_pos = event->pos() - mouse_pos;
	QPoint container_bottomLeft = mapToParent(m_container->geometry().bottomLeft() + new_pos);
	QPoint container_topRight = mapToParent(m_container->geometry().topRight() + new_pos);

	if (is_dragging && m_parent->geometry().contains(QRect(container_bottomLeft, container_topRight))) {
		move(mapToParent(new_pos));
	}
}

void HoverWidget::setContent(QWidget *content)
{
	if (m_content) {
		m_lay->removeWidget(m_content);
		m_content->setParent(nullptr);
		m_content->removeEventFilter(this);
	}

	m_content = content;
	m_content->setParent(m_container);
	m_lay->addWidget(m_content);
	m_container->resize(m_content->size());
	resize(m_content->size());

	m_content->installEventFilter(this);
	moveToAnchor();
}

void HoverWidget::setAnchor(QWidget *anchor)
{
	if (m_anchor) {
		m_anchor->removeEventFilter(this);
	}

	m_anchor = anchor;
	m_anchor->installEventFilter(this);
	moveToAnchor();
}

void HoverWidget::setParent(QWidget *parent)
{
	if (m_parent) {
		m_parent->removeEventFilter(this);
	}

	bool visible = isVisible();
	m_parent = parent;
	QWidget::setParent(m_parent);
	m_parent->installEventFilter(this);
	moveToAnchor();
	setVisible(visible);
}

HoverWidget::~HoverWidget()
{

}

bool HoverWidget::eventFilter(QObject *watched, QEvent *event)
{
	if(watched == m_content) {
		if(event->type() == QEvent::Resize) {
			m_container->resize(m_content->size());
			resize(m_content->size());
		}

		if (event->type() == QEvent::HoverMove) {
			QMouseEvent* e = static_cast<QMouseEvent*>(event);
			if (m_container->geometry().contains(e->pos()) && m_draggable)
				m_content->setCursor(Qt::ClosedHandCursor);
			else
				m_content->setCursor(Qt::ArrowCursor);
		}
	}
	if(watched == m_anchor || watched == m_parent || watched == m_content) {
		if((event->type() == QEvent::Move || event->type() == QEvent::Resize) && !m_draggable) {
			moveToAnchor();
		}
	}

	return QObject::eventFilter(watched, event);
}

QPoint HoverWidget::getAnchorOffset()
{
	return m_anchorOffset;
}

void HoverWidget::setAnchorOffset(QPoint pt) {
	m_anchorOffset = pt;
	moveToAnchor();
}

HoverPosition HoverWidget::getAnchorPos()
{
	return m_anchorPos;
}

void HoverWidget::setAnchorPos(HoverPosition pos)
{
	m_anchorPos = pos;
	moveToAnchor();
}

HoverPosition HoverWidget::getContentPos()
{
	return m_contentPos;
}

void HoverWidget::setContentPos(HoverPosition pos)
{
	m_contentPos = pos;
	moveToAnchor();
}

void HoverWidget::moveToAnchor()
{
	if (!m_content || !m_anchor || !m_parent) return;
	QPoint global = m_anchor->mapToGlobal(QPoint(0,0));
	QPoint mappedPoint =  m_parent->mapFromGlobal(global);
	QPoint anchorPosition = QPoint(0,0);
	QPoint contentPosition = QPoint(0,0);

	switch (m_anchorPos) {
	case HP_LEFT:
		anchorPosition = QPoint(0 , m_anchor->height() / 2);
		break;
	case HP_TOPLEFT:
		anchorPosition = QPoint(0,0);
		break;
	case HP_TOP:
		anchorPosition = QPoint(m_anchor->width() / 2 , 0);
		break;
	case HP_TOPRIGHT:
		anchorPosition = QPoint(m_anchor->width(), 0);
		break;
	case HP_RIGHT:
		anchorPosition = QPoint(m_anchor->width(), m_anchor->height()/2);
		break;
	case HP_BOTTOMRIGHT:
		anchorPosition = QPoint(m_anchor->width(), m_anchor->height());
		break;
	case HP_BOTTOM:
		anchorPosition = QPoint(m_anchor->width() / 2, m_anchor->height());
		break;
	case HP_BOTTOMLEFT:
		anchorPosition = QPoint(0, m_anchor->height());
		break;
	case HP_CENTER:
		anchorPosition = QPoint(m_anchor->width() / 2, m_anchor->height()/2);
		break;
	default:
		anchorPosition = QPoint(0,0);
		break;
	}

	switch (m_contentPos) {
	case HP_LEFT:
		contentPosition = QPoint(-m_content->width(), -m_content->height()/2);
		break;
	case HP_TOPLEFT:
		contentPosition = QPoint(-m_content->width(), -m_content->height());
		break;
	case HP_TOP:
		contentPosition = QPoint(-m_content->width()/2, -m_content->height());
		break;
	case HP_TOPRIGHT:
		contentPosition = QPoint(0, -m_content->height());
		break;
	case HP_RIGHT:
		contentPosition = QPoint(0, -m_content->height()/2);
		break;
	case HP_BOTTOMRIGHT:
		contentPosition = QPoint(0, 0);
		break;
	case HP_BOTTOM:
		contentPosition = QPoint(-m_content->width()/2, 0);
		break;
	case HP_BOTTOMLEFT:
		contentPosition = QPoint(-m_content->width(), 0);
		break;
	case HP_CENTER:
		contentPosition = QPoint(-m_content->width()/2, -m_content->height()/2);
		break;

	default:
		contentPosition = QPoint(0,0);
		break;
	}

	qDebug(CAT_HOVERWIDGET)<<"moveAnchor"<<"mapped"<<mappedPoint<<"contentPosition"<<contentPosition<<
		"anchorPosition"<<anchorPosition<<"offset"<<m_anchorOffset;
	move(mappedPoint + contentPosition + anchorPosition + m_anchorOffset);
}

void HoverWidget::showEvent(QShowEvent *event)
{
	if(!m_draggable) {
		moveToAnchor();
	}
	raise();
	QWidget::showEvent(event);
}
