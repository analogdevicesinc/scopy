#include "2Drenderer.hpp"

using namespace scopy;

BubbleLevelRenderer::BubbleLevelRenderer(QWidget *parent) : QWidget{parent} {
	QPainter painter(this);
	painter.drawEllipse(QPointF(50, 50), 20, 20);
}
