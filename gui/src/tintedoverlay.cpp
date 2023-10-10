#include "tintedoverlay.h"

using namespace scopy::gui;
TintedOverlay::TintedOverlay(QWidget *parent, QColor color)
	: QWidget(parent)
	, color(color)
{
	setAttribute(Qt::WA_TranslucentBackground);
	this->parent = parent;
	holes.clear();

	QRect geo = parent->rect();
	setGeometry(geo);
	setAttribute(Qt::WA_TransparentForMouseEvents, false);
}

TintedOverlay::~TintedOverlay() {}

const QList<QWidget *> &TintedOverlay::getHoles() const { return holes; }

void TintedOverlay::setHoles(const QList<QWidget *> &newHoles) { holes = newHoles; }

void TintedOverlay::clearHoles() { holes.clear(); }

void TintedOverlay::paintEvent(QPaintEvent *ev)
{
	QPainter painter(this);
	if(!holes.isEmpty()) {
		QPainterPath p1, p2;
		p1.addRect(rect());
		for(QWidget *holeWidget : qAsConst(holes)) {
			QPoint offset = holeWidget->mapTo(parent, QPoint(0, 0));
			QRect hole = holeWidget->geometry();
			hole.moveTo(offset);

			p2.addRect(hole);
			p1 -= p2;
		}
		QRegion r(p1.toFillPolygon().toPolygon());
		setMask(r);
	} else {
		setMask(rect());
	}
	painter.fillRect(rect(), color);
}

void TintedOverlay::mousePressEvent(QMouseEvent *ev)
{
	// prevent clicks from being sent to parents
	ev->accept();
}

#include "moc_tintedoverlay.cpp"
