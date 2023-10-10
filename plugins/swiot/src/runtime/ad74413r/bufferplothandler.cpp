/*
 * Copyright (c) 2023 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "bufferplothandler.h"

#include "linked_button.hpp"
#include "pluginbase/preferences.h"
#include "qlabel.h"
#include "src/runtime/ad74413r/ad74413r.h"
#include "src/swiot_logging_categories.h"

#include <QFileDialog>
#include <QGridLayout>

#include <gui/filemanager.h>
#include <unistd.h>

using namespace scopy::swiot;

BufferPlotHandler::BufferPlotHandler(QWidget *parent, int plotChnlsNo)
	: QWidget(parent)
{
	m_lock = new QMutex();
	m_plotChnlsNo = plotChnlsNo;
	m_plot = new CapturePlot(parent, false, 16, 10, new TimePrefixFormatter, new MetricPrefixFormatter,
				 QwtAxis::YRight);
	m_plotScalesController = new ChannelPlotScalesController(this);
	initPlot(plotChnlsNo);
	readPreferences();
}
BufferPlotHandler::~BufferPlotHandler()
{
	if(m_dataPoints.size() > 0) {
		for(int i = 0; i < m_dataPoints.size(); i++) {
			delete[] m_dataPoints[i];
			m_dataPoints[i] = nullptr;
		}
	}
	if(m_plot) {
		delete m_plot;
	}
	if(m_plotScalesController) {
		delete m_plotScalesController;
	}
}

void BufferPlotHandler::initPlot(int plotChnlsNo)
{
	// the last 4 channels are for diagnostic
	int configuredChnlsNo = plotChnlsNo - DIAG_CHNLS_NUMBER;

	m_enabledPlots = std::vector<bool>(plotChnlsNo, false);

	m_plot->registerSink("Active Channels", plotChnlsNo, 0);
	m_plot->disableLegend();

	m_plotWidget = new QWidget(this);

	initStatusWidget();

	QGridLayout *gridPlot = new QGridLayout(m_plotWidget);
	gridPlot->setVerticalSpacing(0);
	gridPlot->setHorizontalSpacing(0);
	gridPlot->setContentsMargins(9, 0, 9, 0);
	m_plotWidget->setStyleSheet("background-color:black;");
	m_plotWidget->setLayout(gridPlot);

	QSpacerItem *plotSpacer = new QSpacerItem(0, 5, QSizePolicy::Fixed, QSizePolicy::Fixed);

	gridPlot->addWidget(m_plot->topArea(), 1, 0, 1, 4);
	gridPlot->addWidget(m_plot->topHandlesArea(), 2, 0, 1, 4);
	gridPlot->addWidget(m_plot->leftHandlesArea(), 1, 0, 4, 1);
	gridPlot->addWidget(m_plotScalesController, 4, 0, 1, 4);

	gridPlot->addWidget(m_plot->rightHandlesArea(), 1, 3, 4, 1);
	gridPlot->addItem(plotSpacer, 5, 0, 1, 4);
	gridPlot->addWidget(m_plot, 3, 1, 1, 1);

	m_plot->setSampleRate(m_samplingFreq, 1, "");
	m_plot->enableTimeTrigger(false);
	m_plot->setActiveVertAxis(0, true);
	m_plot->setAxisVisible(QwtAxis::YLeft, false);
	setYAxisVisible(true);
	m_plot->enableColoredLabels(true);
	updatePlotTimespan();

	for(unsigned int i = 0; i < plotChnlsNo; i++) {
		m_plot->Curve(i)->setAxes(QwtAxisId(QwtAxis::XBottom, 0), QwtAxisId(QwtAxis::YRight, i));
		if(i < configuredChnlsNo) {
			m_plot->Curve(i)->setTitle("CH " + QString::number(i + 1));
		} else {
			m_plot->Curve(i)->setTitle("DIAG " + QString::number(i + 1));
		}
		m_plot->DetachCurve(i);
		m_plot->setOffsetHandleVisible(i, false);
		m_plot->addZoomer(i);
		m_plot->setYaxisUnit("V", i);
	}
	m_plot->configureAllYAxis();
	m_plot->setOffsetInterval(-__DBL_MAX__, __DBL_MAX__);
	connect(m_plot, &CapturePlot::channelSelected, this, [=, this](int hdlIdx, bool selected) {
		m_plot->setActiveVertAxis(hdlIdx, true);
		Q_EMIT offsetHandleSelected(hdlIdx, selected);
	});
}

void BufferPlotHandler::initStatusWidget()
{
	QWidget *statusWidget = new QWidget();
	QHBoxLayout *statusLayout = new QHBoxLayout(statusWidget);

	m_samplesAquiredLabel = new QLabel("0");
	m_plotSamplesNumberLabel = new QLabel(QString::number(m_samplingFreq));
	m_btnInfoStatus = new scopy::LinkedButton();
	m_btnInfoStatus->installEventFilter(this);
	m_btnInfoStatus->setObjectName(QString::fromUtf8("btnHelp"));
	m_btnInfoStatus->setCheckable(false);
	m_btnInfoStatus->setText(QString());
	statusLayout->insertWidget(0, m_samplesAquiredLabel);
	statusLayout->insertWidget(1, new QLabel("Samples at"));
	statusLayout->insertWidget(2, m_plotSamplesNumberLabel);
	statusLayout->insertWidget(3, new QLabel("sps"));
	statusLayout->insertWidget(4, m_btnInfoStatus);
	m_plot->setStatusWidget(statusWidget);
}

void BufferPlotHandler::setYAxisVisible(bool visible) { m_plot->setAxisVisible(QwtAxis::YRight, visible); }

void BufferPlotHandler::setHandlesName(QMap<int, QString> chnlsId)
{
	connect(m_plot, &CapturePlot::channelOffsetChanged, this, [=, this](int chn, double offset) {
		double unitPerDiv = m_plot->VertUnitsPerDiv(chn);
		int curveId = m_channelCurveId[chn];
		setUnitPerDivision(curveId, unitPerDiv);
	});
	QList<RoundedHandleV *> offsetHandles = m_plot->getOffsetHandles();
	int offsetHandlesSize = offsetHandles.size();
	int mapSize = chnlsId.size();
	int activeChnlsIndex = 0;
	if(mapSize == offsetHandlesSize) {
		auto keys = chnlsId.keys();
		for(int key : keys) {
			if(activeChnlsIndex < (m_plotChnlsNo - DIAG_CHNLS_NUMBER)) {
				QString chId = QString::number(key + 1);
				offsetHandles[activeChnlsIndex]->setName(chnlsId[key][0].toUpper() + chId);
			} else {
				QString chId = QString::number(key + 1);
				offsetHandles[activeChnlsIndex]->setName("DIAG" + chId);
			}
			activeChnlsIndex++;
		}
	}
}

// bufferCounter is used only for debug
void BufferPlotHandler::onBufferRefilled(QVector<QVector<double>> bufferData, int bufferCounter)
{
	int plotSampleNumber = m_plotSampleRate * m_timespan;
	int bufferDataSize = bufferData.size();
	int enPlotIndex = 0;
	bool rolling = false;
	m_lock->lock();
	resetDataPoints();
	if(!(m_singleCapture && (m_bufferIndex == m_buffersNumber))) {
		if(bufferDataSize > 0) {
			int currentPlotDataSamplesNumber = 0;
			for(int i = 0; i < m_dataPointsDeque.size(); i++) {
				if(m_bufferIndex == m_buffersNumber) {
					int dequeSize = m_dataPointsDeque[i].size();
					if(dequeSize > 0) {
						m_dataPointsDeque[i].pop_front();
						rolling = true;
					}
				} else {
					int lastBuffer = plotSampleNumber % m_bufferSize;
					if((m_bufferIndex == (m_buffersNumber - 1)) && lastBuffer != 0) {
						currentPlotDataSamplesNumber =
							(m_bufferIndex * m_bufferSize) + lastBuffer;
					} else {
						currentPlotDataSamplesNumber = (m_bufferIndex + 1) * m_bufferSize;
					}
					// the range is between (-currentPlotDataSamplesNumber, 0]
					// so we need to add 1 to currentPlotDataSamplesNumber
					m_plot->setDataStartingPoint(-currentPlotDataSamplesNumber + 1);
					m_plot->resetXaxisOnNextReceivedData();
					m_samplesAquiredLabel->setText(QString::number(currentPlotDataSamplesNumber));
				}
				if(m_enabledPlots[i] && (enPlotIndex < bufferDataSize)) {
					m_dataPointsDeque[i].push_back(bufferData[enPlotIndex]);
					enPlotIndex++;
				}
			}
			m_bufferIndex = (rolling) ? m_bufferIndex : m_bufferIndex + 1;
			drawPlot();
		}
	}
	if(m_singleCapture && (m_bufferIndex == m_buffersNumber)) {
		Q_EMIT singleCaptureFinished();
	}
	m_lock->unlock();
}

int BufferPlotHandler::getRequiredBuffersNumber() { return m_buffersNumber; }

void BufferPlotHandler::drawPlot()
{
	int plotSampleNumber = m_plotSampleRate * m_timespan;
	int dataPointsNumber = m_bufferIndex * m_bufferSize;
	int lastBufferData = plotSampleNumber % m_bufferSize;
	if((m_bufferIndex == m_buffersNumber) && (lastBufferData != 0)) {
		dataPointsNumber = (dataPointsNumber - m_bufferSize) + lastBufferData;
	}
	for(int i = 0; i < m_dataPointsDeque.size(); i++) {
		m_dataPoints.push_back(new double[dataPointsNumber]());
		int dequeSize = m_dataPointsDeque[i].size();
		if(dequeSize > 0) {
			for(int j = 0; j < dequeSize; j++) {
				if(j == (m_buffersNumber - 1) && (lastBufferData != 0)) {
					std::copy(m_dataPointsDeque[i][j].begin(),
						  m_dataPointsDeque[i][j].begin() + lastBufferData,
						  m_dataPoints[i] + (j * m_bufferSize));
				} else {
					std::copy(m_dataPointsDeque[i][j].begin(), m_dataPointsDeque[i][j].end(),
						  m_dataPoints[i] + (j * m_bufferSize));
				}
			}
		}
	}
	m_plot->plotNewData("Active Channels", m_dataPoints, dataPointsNumber, 1);
}

void BufferPlotHandler::setChnlsUnitOfMeasure(QVector<QString> unitsOfMeasure)
{
	for(int i = 0; i < m_plot->leftVertAxesCount(); i++) {
		m_plot->setYaxisUnit(unitsOfMeasure[i], i);
	}
	m_plot->replot();
}

void BufferPlotHandler::setUnitPerDivision(int i, double unitPerDiv)
{
	m_unitPerDivision.insert(i, unitPerDiv);
	updateScale(i);
	Q_EMIT unitPerDivisionChanged(i, unitPerDiv);
}

void BufferPlotHandler::setInstantValue(int channel, double value)
{
	m_plotScalesController->setInstantValue(channel, value);
}

void BufferPlotHandler::addChannelScale(int index, QColor color, QString unit, bool enabled)
{
	m_plotScalesController->addChannel(index, color, unit, enabled);
}

void BufferPlotHandler::mapChannelCurveId(int curveId, int channelId) { m_channelCurveId.insert(curveId, channelId); }

void BufferPlotHandler::updateScale(int channel)
{
	int curveId = m_channelCurveId.key(channel);
	m_plot->setVertUnitsPerDiv(m_unitPerDivision[channel], curveId);
	m_plotScalesController->setUnitPerDivision(channel, m_unitPerDivision[channel]);
	m_plot->replot();
}

void BufferPlotHandler::onBtnExportClicked(QMap<int, bool> exportConfig)
{
	QStringList filter;
	bool useNativeDialogs = false;
	filter += QString(tr("Comma-separated values files (*.csv)"));
	filter += QString(tr("Tab-delimited values files (*.txt)"));
	filter += QString(tr("All Files(*)"));

	QString selectedFilter = filter[0];

	QString fileName = QFileDialog::getSaveFileName(
		this, tr("Export"), "", filter.join(";;"), &selectedFilter,
		(useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));

	if(fileName.split(".").size() <= 1) {
		// file name w/o extension. Let's append it
		QString ext = selectedFilter.split(".")[1].split(")")[0];
		fileName += "." + ext;
	}
	bool atleastOneChannelEnabled = false;

	auto keys = exportConfig.keys();
	for(auto x : qAsConst(keys))
		if(exportConfig[x]) {
			atleastOneChannelEnabled = true;
			break;
		}
	if(!atleastOneChannelEnabled) {
		return;
	}

	if(!fileName.isEmpty()) {
		FileManager fm("SWIOTad74413rRuntime");
		fm.open(fileName, FileManager::EXPORT);
		QVector<double> time_data;

		for(size_t i = 0; i < m_plot->Curve(0)->data()->size(); ++i) {
			time_data.push_back(m_plot->Curve(0)->sample(i).x());
		}

		fm.save(time_data, "Time(S)");

		for(int i = 0; i < m_plotChnlsNo; ++i) {
			if(exportConfig[i]) {
				QVector<double> data;
				int samples = m_plot->Curve(i)->data()->size();
				for(int j = 0; j < samples; ++j)
					data.push_back(m_plot->Curve(i)->data()->sample(j).y());
				QString chNo = QString::number(i + 1);
				QString chType =
					(i < (m_plotChnlsNo - DIAG_CHNLS_NUMBER)) ? QString("CH") : QString("DIAG");
				fm.save(data, chType + chNo + "(" + m_plot->yAxisUnit(i) + ")");
			}
		}
		fm.performWrite(false);
	}
}

void BufferPlotHandler::onTimespanChanged(double value)
{
	m_plot->cancelZoom();
	m_timespan = value;
	updatePlotTimespan();
	resetPlotParameters();
}

void BufferPlotHandler::onSamplingFreqWritten(int samplingFreq)
{
	m_samplingFreq = samplingFreq;
	resetPlotParameters();
}

void BufferPlotHandler::setSingleCapture(bool en) { m_singleCapture = en; }

QColor BufferPlotHandler::getCurveColor(int id) const { return m_plot->getLineColor(id); }

QWidget *BufferPlotHandler::getPlotWidget() const { return m_plotWidget; }

void BufferPlotHandler::onChannelWidgetEnabled(int curveId, bool en)
{
	m_enabledPlots[curveId] = en;
	m_plot->setOffsetHandleVisible(curveId, en);
	m_plotScalesController->setChannelEnabled(m_channelCurveId.value(curveId), en);
	if(en) {
		m_plot->AttachCurve(curveId);
	} else {
		m_plot->DetachCurve(curveId);
	}
}

void BufferPlotHandler::onChannelWidgetSelected(int curveId)
{
	m_plot->bringCurveToFront(curveId);
	m_plot->setActiveVertAxis(curveId);
	m_plot->setAllAxes(curveId);
}

void BufferPlotHandler::onPrintBtnClicked() { m_plot->printWithNoBackground(AD_NAME); }

void BufferPlotHandler::resetPlotParameters()
{
	m_lock->lock();
	int enabledPlotsNo = std::count(m_enabledPlots.begin(), m_enabledPlots.end(), true);
	m_plotSampleRate = (enabledPlotsNo > 0) ? (m_samplingFreq / enabledPlotsNo) : m_samplingFreq;

	auto plotSampleNumber = m_plotSampleRate * m_timespan;
	m_bufferSize = (m_samplingFreq > MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : MIN_BUFFER_SIZE;
	m_buffersNumber = (((int)plotSampleNumber % m_bufferSize) == 0) ? (plotSampleNumber / m_bufferSize)
									: ((plotSampleNumber / m_bufferSize) + 1);
	m_bufferIndex = 0;
	resetDeque();

	m_plotSamplesNumberLabel->setText(QString::number(m_plotSampleRate));
	// setSampleRate sets the samples range
	// in our case (-m_plotSampleRate, 0], that's why we subtract by 1
	m_plot->setSampleRate(m_plotSampleRate - 1, 1, "");
	m_plot->replot();

	qDebug(CAT_SWIOT_AD74413R) << "Plot samples number: " << QString::number(plotSampleNumber) << " "
				   << QString::number(m_buffersNumber) + " " +
			QString::number(plotSampleNumber / m_bufferSize) + " ";
	m_lock->unlock();
}

void BufferPlotHandler::resetDataPoints()
{
	for(int i = 0; i < m_dataPoints.size(); i++) {
		delete[] m_dataPoints[i];
		m_dataPoints[i] = nullptr;
	}
	m_dataPoints.clear();
}

void BufferPlotHandler::resetDeque()
{
	resetDataPoints();
	for(int i = 0; i < m_dataPointsDeque.size(); i++) {
		m_dataPointsDeque[i].clear();
	}
	m_dataPointsDeque.clear();
	for(int i = 0; i < m_plotChnlsNo; i++) {
		m_dataPointsDeque.push_back(std::deque<QVector<double>>());
	}
}

void BufferPlotHandler::readPreferences()
{
	Preferences *p = Preferences::GetInstance();
	bool showFps = p->get("general_show_plot_fps").toBool();
	m_plot->setVisibleFpsLabel(showFps);
}

void BufferPlotHandler::updatePlotTimespan()
{
	double unitsPerDiv = m_timespan / m_plot->xAxisNumDiv();
	m_plot->setHorizUnitsPerDiv(unitsPerDiv);
	// the offset is calculated so that the 0 moment of time to be on the right side of the x axis
	// we divide by 2 the number of x divisions because the moment
	// 0 of time is by default in the middle of the x Axis
	double offset = unitsPerDiv * (m_plot->xAxisNumDiv() / 2);
	m_plot->setHorizOffset(-offset);
	m_plot->resetXaxisOnNextReceivedData();
}

bool BufferPlotHandler::eventFilter(QObject *obj, QEvent *event)
{
	if(obj == (QObject *)m_btnInfoStatus) {
		if(event->type() == QEvent::Enter) {
			auto enabledPlotsNo = std::count(m_enabledPlots.begin(), m_enabledPlots.end(), true);
			m_btnInfoStatus->setToolTip("sps = samples per second \n"
						    "sps = sampling_frequency / enabled_channels \n"
						    "Enabled channels = " +
						    QString::number(enabledPlotsNo) + "\n" +
						    "Samples per channel = " + m_samplesAquiredLabel->text() + "\n" +
						    "Sampling frequency = " + QString::number(m_samplingFreq));
		}
		return false;
	} else {
		return QWidget::eventFilter(obj, event);
	}
}

bool BufferPlotHandler::singleCapture() const { return m_singleCapture; }
