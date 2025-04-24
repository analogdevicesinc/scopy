#include "2Drenderer.hpp"

using namespace scopy;

BubbleLevelRenderer::BubbleLevelRenderer(QWidget *parent) : QWidget{parent} {
	QHBoxLayout *lay = new QHBoxLayout(this);
	lay->setMargin(0);
	setLayout(lay);


	m_rot = {0.0f, 0.0f, 0.0f};
	//lay->addWidget(this);
}

void BubbleLevelRenderer::setRot(rotation rot){
	m_rot = rot;
	this->update();
}

void BubbleLevelRenderer::paintEvent(QPaintEvent *event){

	QPainter painter(this);
	painter.setBrush(Qt::red);
	painter.drawEllipse(QPointF(200.0f + m_rot.rotX, 100.0f), 100, 100);
	painter.drawEllipse(QPointF(200.0f, 400 + m_rot.rotY), 100, 100);
}
