#include "channelmonitorcomponent.hpp"
#include "ui_channelmonitorcomponent.h"

#include <QDebug>


using namespace adiscope;

ChannelMonitorComponent::ChannelMonitorComponent(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ChannelMonitorComponent),
	m_minValue(Q_INFINITY),
	m_maxValue(-Q_INFINITY)
{
	ui->setupUi(this);
	ui->frame->setStyleSheet("QWidget#frame{border: 3px solid grey;}");

}

void ChannelMonitorComponent::init(double value,QString nameOfUnitOfMeasure,QString symbolOfUnitOfMeasure, QString title,QColor color)
{

	ui->scaleCh1->setOrientation(Qt::Horizontal);
	ui->scaleCh1->setScalePosition(QwtThermo::LeadingScale);
	ui->scaleCh1->setOriginMode(QwtThermo::OriginCustom);
	ui->scaleCh1->setStyleSheet("font-size:16px;");
	ui->scaleCh1->setObjectName("DataLogger");

	ui->monitorTitle->setText(title);

	//set unit of measure for components
	ui->labelCh1->setText(symbolOfUnitOfMeasure);
	ui->label_minUnitOfMeasure->setText(symbolOfUnitOfMeasure);
	ui->label_maxUnitOfMeasure->setText(symbolOfUnitOfMeasure);
	ui->sismograph_ch1_2->setPlotAxisXTitle(nameOfUnitOfMeasure + "(" +symbolOfUnitOfMeasure + ")");

	setColor(color);

	ui->sismograph_ch1_2->setNumSamples(10);

	ui->lcdCh1->setPrecision(3);

	setMonitorColor(color.name());
}

void ChannelMonitorComponent::updateValue(double value,QString nameOfUnitOfMeasure,QString symbolOfUnitOfMeasure)
{

	MetricPrefixFormatter m_prefixFormater;
	QString formatedPrefix = m_prefixFormater.getFormatedMeasureUnit(value);
	double formatedValue = m_prefixFormater.getFormatedValue(value);

	ui->lcdCh1->display(formatedValue);
	ui->labelCh1->setText(formatedPrefix + symbolOfUnitOfMeasure);

	ui->scaleCh1->setValue(formatedValue);

	ui->sismograph_ch1_2->setUnitOfMeasure(nameOfUnitOfMeasure, symbolOfUnitOfMeasure);
	ui->sismograph_ch1_2->plot(value);
	checkPeakValues(value, symbolOfUnitOfMeasure);
}

void ChannelMonitorComponent::resizeEvent(QResizeEvent *event)
{
	Q_EMIT contentChanged();
}


void ChannelMonitorComponent::checkPeakValues(double value,QString unitOfMeasure)
{
	MetricPrefixFormatter m_prefixFormater;
	QString formatedPrefix = m_prefixFormater.getFormatedMeasureUnit(value);
	double formatedValue = m_prefixFormater.getFormatedValue(value);

	if (value < m_minValue) {
		m_minValue = value;
		ui->minCh1->display(formatedValue);
		ui->label_minUnitOfMeasure->setText(formatedPrefix + unitOfMeasure);
	}
	if (value > m_maxValue) {
		m_maxValue = value;
		ui->maxCh1->display(formatedValue);
		ui->label_maxUnitOfMeasure->setText(formatedPrefix + unitOfMeasure);
	}
}

void ChannelMonitorComponent::resetPeakHolder()
{
	m_minValue = Q_INFINITY;
	ui->minCh1->display(0);
	m_maxValue = -Q_INFINITY;
	ui->maxCh1->display(0);
}

void ChannelMonitorComponent::displayPeakHold(bool checked)
{
	if (checked) {
		ui->minCh1->show();
		ui->label_min->show();
		ui->label_minUnitOfMeasure->show();
		ui->maxCh1->show();
		ui->label_max->show();
		ui->label_maxUnitOfMeasure->show();
	} else {
		ui->minCh1->hide();
		ui->label_min->hide();
		ui->label_minUnitOfMeasure->hide();
		ui->maxCh1->hide();
		ui->label_max->hide();
		ui->label_maxUnitOfMeasure->hide();
	}
}

void ChannelMonitorComponent::displayHistory(bool checked)
{
	if (checked) {
		ui->sismograph_ch1_2->show();
	} else {
		ui->sismograph_ch1_2->hide();
	}
}

void ChannelMonitorComponent::displayScale(bool checked)
{
	if (checked) {
		ui->scaleCh1->show();
	} else {
		ui->scaleCh1->hide();
	}
}
std::string ChannelMonitorComponent::getChannelId()
{
	return m_channelId;
}

void ChannelMonitorComponent::setChannelId(std::string channelId)
{
	m_channelId = channelId;
}

QString ChannelMonitorComponent::getTitle()
{
	return ui->monitorTitle->text();
}

void ChannelMonitorComponent::updateUnitOfMeasure(QString unitOfMeasure)
{
	m_unitOfMeasure = unitOfMeasure;
	ui->labelCh1->setText(unitOfMeasure);
}
QString ChannelMonitorComponent::getUnitOfMeasure()
{
	return m_unitOfMeasure;
}

void ChannelMonitorComponent::updateLcdNumberPrecision(int precision)
{
	ui->lcdCh1->setPrecision(precision);
	ui->lcdCh1->setDigitCount(4 + precision);
	ui->lcdCh1->display(ui->lcdCh1->value());

	ui->minCh1->setPrecision(precision);
	ui->minCh1->setDigitCount(4 + precision);

	ui->maxCh1->setPrecision(precision);
	ui->maxCh1->setDigitCount(4 + precision);

	if (m_minValue != Q_INFINITY) {
		ui->minCh1->display(m_minValue);
	}
	if (m_maxValue != -Q_INFINITY) {
		ui->maxCh1->display(m_maxValue);
	}
}

void ChannelMonitorComponent::setMonitorColor(QString color)
{
	ui->sismograph_ch1_2->setColor(QColor(color));
	ui->lcdCh1->setStyleSheet("QLCDNumber{color: "+color+" ;}");
	ui->minCh1->setStyleSheet("QLCDNumber{color: "+color+" ;}");
	ui->maxCh1->setStyleSheet("QLCDNumber{color: "+color+" ;}");
}

void ChannelMonitorComponent::setHistoryDuration(int duration)
{
	ui->sismograph_ch1_2->setHistoryDuration(duration);
}

void ChannelMonitorComponent::setRecordingInterval(double interval)
{
	ui->sismograph_ch1_2->setSampleRate(1/interval);
}

void ChannelMonitorComponent::setColor(QColor color)
{
	m_color = color;
	ui->sismograph_ch1_2->setColor(color);
	ui->lcdCh1->setStyleSheet("QLCDNumber{color: "+color.name()+" ;}");
	ui->minCh1->setStyleSheet("QLCDNumber{color: "+color.name()+" ;}");
	ui->maxCh1->setStyleSheet("QLCDNumber{color: "+color.name()+" ;}");
}

QColor ChannelMonitorComponent::getColor()
{
	return m_color;
}

void ChannelMonitorComponent::setID(int id)
{
	m_id = id;
}

int ChannelMonitorComponent::getID()
{
	return m_id;
}

void ChannelMonitorComponent::setLineStyle(Qt::PenStyle lineStyle)
{
	ui->sismograph_ch1_2->setLineStyle(lineStyle);
}

ChannelMonitorComponent::~ChannelMonitorComponent()
{
	delete ui;
}
