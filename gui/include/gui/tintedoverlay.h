#ifndef TINTEDOVERLAY_H
#define TINTEDOVERLAY_H

#include "scopy-gui_export.h"

#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QWidget>

namespace scopy::gui {
class SCOPY_GUI_EXPORT TintedOverlay : public QWidget
{
	Q_OBJECT
public:
	explicit TintedOverlay(QWidget *parent = nullptr, QColor color = QColor(0, 0, 0, 220));
	~TintedOverlay();

	const QList<QWidget *> &getHoles() const;
	void setHoles(const QList<QWidget *> &newHoles);
	void clearHoles();

protected:
	void paintEvent(QPaintEvent *ev) override;
	void mousePressEvent(QMouseEvent *ev) override;

private:
	QWidget *parent;
	QList<QWidget *> holes;
	QColor color;
};
} // namespace scopy::gui

#endif // TINTEDOVERLAY_H
