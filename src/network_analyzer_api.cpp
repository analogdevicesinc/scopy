#include "network_analyzer_api.hpp"
#include "ui_network_analyzer.h"

namespace adiscope {
void NetworkAnalyzer_API::show()
{
        Q_EMIT net->showTool();
}

double NetworkAnalyzer_API::getMinFreq() const
{
	return net->start_freq->value();
}

double NetworkAnalyzer_API::getMaxFreq() const
{
	return net->stop_freq->value();
}

double NetworkAnalyzer_API::getSamplesCount() const
{
        return net->samplesCount->value();
}

double NetworkAnalyzer_API::getAmplitude() const
{
        return net->amplitude->value();
}

double NetworkAnalyzer_API::getOffset() const
{
        return net->offset->value();
}

void NetworkAnalyzer_API::setMinFreq(double freq)
{
	net->start_freq->setValue(freq);
    net->m_dBgraph.setXMin(freq);
    net->m_phaseGraph.setXMin(freq);
}

void NetworkAnalyzer_API::setMaxFreq(double freq)
{
	net->stop_freq->setValue(freq);
    net->m_dBgraph.setXMax(freq);
    net->m_phaseGraph.setXMax(freq);
}

void NetworkAnalyzer_API::setSamplesCount(double step)
{
        net->samplesCount->setValue(step);
}

void NetworkAnalyzer_API::setAmplitude(double amp)
{
        net->amplitude->setValue(amp);
}

void NetworkAnalyzer_API::setOffset(double offset)
{
        net->offset->setValue(offset);
}

double NetworkAnalyzer_API::getMinMag() const
{
        return net->magMin->value();
}

double NetworkAnalyzer_API::getMaxMag() const
{
        return net->magMax->value();
}

double NetworkAnalyzer_API::getMinPhase() const
{
        return net->phaseMin->value();
}

double NetworkAnalyzer_API::getMaxPhase() const
{
        return net->phaseMax->value();
}

void NetworkAnalyzer_API::setMinMag(double val)
{
        net->magMin->setValue(val);
    net->m_dBgraph.setYMin(val);
	net->ui->xygraph->setMin(val);
	net->ui->nicholsgraph->setYMin(val);
}

void NetworkAnalyzer_API::setMaxMag(double val)
{
        net->magMax->setValue(val);
    net->m_dBgraph.setYMax(val);
	net->ui->xygraph->setMax(val);
	net->ui->nicholsgraph->setYMax(val);
}

void NetworkAnalyzer_API::setMinPhase(double val)
{
        net->phaseMin->setValue(val);
    net->m_phaseGraph.setYMin(val);
	net->ui->nicholsgraph->setXMin(val);
}

void NetworkAnalyzer_API::setMaxPhase(double val)
{
        net->phaseMax->setValue(val);
    net->m_phaseGraph.setYMax(val);
	net->ui->nicholsgraph->setXMax(val);
}

bool NetworkAnalyzer_API::isLogFreq() const
{
    return net->ui->btnIsLog->isChecked();
}

void NetworkAnalyzer_API::setLogFreq(bool is_log)
{
    net->ui->btnIsLog->setChecked(is_log);
}

int NetworkAnalyzer_API::getRefChannel() const
{
    if (net->ui->btnRefChn->isChecked())
		return 1;
	else
		return 2;
}

void NetworkAnalyzer_API::setRefChannel(int chn)
{
	if (chn == 1)
        net->ui->btnRefChn->setChecked(true);
	else
        net->ui->btnRefChn->setChecked(false);
}

bool NetworkAnalyzer_API::getCursors() const
{
	return net->d_cursorsEnabled;
}

void NetworkAnalyzer_API::setCursors(bool enabled)
{
    net->ui->boxCursors->setChecked(enabled);
}

bool NetworkAnalyzer_API::running() const
{
	return net->ui->runSingleWidget->runButtonChecked()
			|| net->ui->runSingleWidget->singleButtonChecked();
}
void NetworkAnalyzer_API::run(bool enabled)
{
	net->ui->runSingleWidget->toggle(enabled);
}

int NetworkAnalyzer_API::getCursorsPosition() const
{
    if (!net->ui->boxCursors->isChecked()) {
        return 0;
    }
    auto currentPos = net->m_dBgraph.getCursorReadoutCurrentPosition();
    switch (currentPos) {
    case CustomPlotPositionButton::ReadoutsPosition::topLeft:
    default:
        return 0;
    case CustomPlotPositionButton::ReadoutsPosition::topRight:
        return 1;
    case CustomPlotPositionButton::ReadoutsPosition::bottomLeft:
        return 2;
    case CustomPlotPositionButton::ReadoutsPosition::bottomRight:
        return 3;
    }
}

void NetworkAnalyzer_API::setCursorsPosition(int val)
{
    if (!net->ui->boxCursors->isChecked()) {
        return;
    }
    enum CustomPlotPositionButton::ReadoutsPosition types[] = {
        CustomPlotPositionButton::ReadoutsPosition::topLeft,
        CustomPlotPositionButton::ReadoutsPosition::topRight,
        CustomPlotPositionButton::ReadoutsPosition::bottomLeft,
        CustomPlotPositionButton::ReadoutsPosition::bottomRight
    };
    net->ui->posSelect->setPosition(types[val]);
    net->m_dBgraph.moveCursorReadouts(types[val]);
    net->m_dBgraph.replot();
    net->m_phaseGraph.moveCursorReadouts(types[val]);
    net->m_phaseGraph.replot();
}

int NetworkAnalyzer_API::getCursorsTransparency() const
{
    if (!net->ui->boxCursors->isChecked()) {
        return 0;
    }
    return net->ui->horizontalSlider->value();
}

void NetworkAnalyzer_API::setCursorsTransparency(int val)
{
    if (!net->ui->boxCursors->isChecked()) {
        return;
    }
    net->ui->horizontalSlider->setValue(val);
    net->ui->transLabel->setText("Transparency " + QString::number(val) + "%");
    net->m_dBgraph.setCursorReadoutsTransparency(val);
    net->m_phaseGraph.setCursorReadoutsTransparency(val);
}


int NetworkAnalyzer_API::getPlotType() const
{
	return net->ui->cmb_graphs->currentIndex();
}
void NetworkAnalyzer_API::setPlotType(int val)
{
	 net->ui->cmb_graphs->setCurrentIndex(val);
}
int NetworkAnalyzer_API::getLineThickness() const
{
	return net->ui->cbLineThickness->currentIndex();
}
void NetworkAnalyzer_API::setLineThickness(int index)
{
	net->ui->cbLineThickness->setCurrentIndex(index);
}

QList<double> NetworkAnalyzer_API::data() const
{
	QList<double> list = net->m_dBgraph.getXAxisData().toList();
	return list;
}

QList<double> NetworkAnalyzer_API::freq() const
{
	QList<double> list = net->m_dBgraph.getYAxisData().toList();
	return list;
}

QList<double> NetworkAnalyzer_API::phase() const
{
	QList<double> list = net->m_phaseGraph.getXAxisData().toList();
	return list;
}

}
