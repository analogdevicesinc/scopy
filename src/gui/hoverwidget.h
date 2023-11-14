#ifndef HOVERWIDGET_H
#define HOVERWIDGET_H

#include <QEvent>
#include <QHBoxLayout>
#include <QWidget>

namespace scopy {
enum HoverPosition
{
	HP_LEFT,
	HP_TOPLEFT,
	HP_TOP,
	HP_TOPRIGHT,
	HP_RIGHT,
	HP_BOTTOMRIGHT,
	HP_BOTTOM,
	HP_BOTTOMLEFT,
	HP_CENTER,
};

class HoverWidget : public QWidget
{
	Q_OBJECT
public:
	HoverWidget(QWidget *content = nullptr, QWidget *anchor = nullptr, QWidget *parent = nullptr);
	~HoverWidget();

	bool eventFilter(QObject *watched, QEvent *event) override;
	QPoint getAnchorOffset();
	void setAnchorOffset(QPoint);
	HoverPosition getAnchorPos();
	void setAnchorPos(HoverPosition);
	HoverPosition getContentPos();
	void setContentPos(HoverPosition);

public:
	void updatePos();
	void moveToRelativePos();
	void moveToAnchor();
	void setContent(QWidget *widget);
	void setAnchor(QWidget *anchor);
	void setParent(QWidget *parent);
	void setDraggable(bool draggable);
	void setRelative(bool relative);
	void setRelativeOffset(QPointF offset);

private:
	QHBoxLayout *m_lay;
	QWidget *m_parent;
	QWidget *m_anchor;
	QWidget *m_content;
	QPoint m_anchorOffset;
	HoverPosition m_anchorPos;
	HoverPosition m_contentPos;
	bool m_draggable;
	bool is_dragging;
	QPoint *mouse_pos;
	bool m_relative;
	QPointF *m_relativeOffset;

private:
	void updateRelativeOffset();

	// QWidget interface
protected:
	void showEvent(QShowEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
};
} // namespace scopy
#endif // HOVERWIDGET_H
