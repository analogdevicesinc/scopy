#ifndef TINTEDOVERLAY_H
#define TINTEDOVERLAY_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QDebug>
#include <QMouseEvent>
#include "scopy-gui_export.h"

namespace scopy::gui {
class SCOPY_GUI_EXPORT TintedOverlay : public QWidget {
	Q_OBJECT
public:
	explicit TintedOverlay(QWidget *parent = nullptr, QColor color = QColor(0,0,0,220));
	~TintedOverlay();

	const QList<QWidget *> &getHoles() const;
	void setHoles(const QList<QWidget *> &newHoles);
	void clearHoles();

protected:
	void paintEvent(QPaintEvent *ev) override;
	void mousePressEvent(QMouseEvent *ev) override;

private:
	QWidget *parent;
	QList<QWidget*> holes;
	QColor color;
};
}


#endif // TINTEDOVERLAY_H
