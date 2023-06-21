#include "drawarea.h"
#include <QPainter>
#include <QPixmap>
#include <QtXml/QDomElement>
#include <QFile>
#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>

#include "src/swiot_logging_categories.h"

using namespace scopy::swiot;

DrawArea::DrawArea(QWidget *parent) :
	QWidget(parent),
	m_filePath(":/swiot/swiot_board.svg"),
	m_drawLabel(new QLabel(this)),
	m_baseImage(new QImage(m_filePath)),
	m_connectionsMap(new QMap<QPair<int, int>, QPixmap *>())
{
	this->setLayout(new QVBoxLayout(this));
	this->setStyleSheet("background-color: black;");

	// insert, for each channel, the 2 PixMaps, one for ad74413r and one for max14906
	for (int i = 1; i <= 4; ++i) {
		m_connectionsMap->insert({i, AD74413R},
					 new QPixmap(":/swiot/ad74413r_channel" + QString::number(i) + ".png"));
		m_connectionsMap->insert({i, MAX14906},
					 new QPixmap(":/swiot/max14906_channel" + QString::number(i) + ".png"));
	}

	m_boardImage = new QImage(m_filePath);
	m_drawLabel->setPixmap(QPixmap::fromImage(*m_boardImage));
	m_drawLabel->setStyleSheet("border: 20px solid #272730;");
	this->layout()->addWidget(m_drawLabel);
}

DrawArea::~DrawArea() {
	delete m_connectionsMap;
}

void DrawArea::activateConnection(int channelIndex, DrawArea::ChannelName channelName) {
	QPainter painter(m_boardImage);
	painter.drawPixmap(0, 0, *m_connectionsMap->value(qMakePair(channelIndex, channelName)));
	m_drawLabel->setPixmap(QPixmap::fromImage(*m_boardImage));
}

void DrawArea::deactivateConnections() {
	m_boardImage = new QImage(*m_baseImage);
	m_drawLabel->setPixmap(QPixmap::fromImage(*m_baseImage));
}
