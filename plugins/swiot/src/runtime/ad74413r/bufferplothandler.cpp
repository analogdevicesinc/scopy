#include "bufferplothandler.h"
#include <QGridLayout>
#include <gui/filemanager.h>
#include "linked_button.hpp"
#include "pluginbase/preferences.h"
#include "qlabel.h"
#include "qpushbutton.h"
#include "src/swiot_logging_categories.h"
#include <QFileDialog>
#include <unistd.h>

using namespace scopy::swiot;

BufferPlotHandler::BufferPlotHandler(QWidget *parent, int plotChnlsNo, int samplingFreq) :
	QWidget(parent)
{
	m_lock = new QMutex();
	m_plotChnlsNo = plotChnlsNo;
	m_samplingFreq = samplingFreq;
	m_bufferSize = (m_samplingFreq > MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : MIN_BUFFER_SIZE;
	m_plot = new CapturePlot(parent, false, 10, 10, new TimePrefixFormatter, new MetricPrefixFormatter);
	initPlot(plotChnlsNo);
	resetPlotParameters();
	readPreferences();
}
BufferPlotHandler::~BufferPlotHandler()
{
	if (m_dataPoints.size() > 0) {
		for (int i = 0; i < m_dataPoints.size(); i++) {
			delete[] m_dataPoints[i];
			m_dataPoints[i] = nullptr;
		}
	}
	if (m_plot) {
		delete m_plot;
	}
}

void BufferPlotHandler::initPlot(int plotChnlsNo)
{
	//the last 4 channels are for diagnostic
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

	QSpacerItem *plotSpacer = new QSpacerItem(0, 5,
						  QSizePolicy::Fixed, QSizePolicy::Fixed);

	gridPlot->addWidget(m_plot->topArea(), 1, 0, 1, 4);
	gridPlot->addWidget(m_plot->topHandlesArea(), 2, 0, 1, 4);
	gridPlot->addWidget(m_plot->leftHandlesArea(), 1, 0, 4, 1);
	gridPlot->addWidget(m_plot->bottomHandlesArea(), 4, 0, 2, 4);
	gridPlot->addItem(plotSpacer, 5, 0, 1, 4);
	gridPlot->addWidget(m_plot, 3, 1, 1, 1);

	m_plot->setSampleRate(m_samplingFreq, 1, "");
	m_plot->enableTimeTrigger(false);
	m_plot->setActiveVertAxis(0, true);
	m_plot->setAxisScale(QwtAxisId(QwtAxis::XBottom, 0), 0, m_timespan);

	for (unsigned int i = 0; i < plotChnlsNo; i++) {
		m_plot->Curve(i)->setAxes(
					QwtAxisId(QwtAxis::XBottom, 0),
					QwtAxisId(QwtAxis::YLeft, i));
		if (i < configuredChnlsNo) {
			m_plot->Curve(i)->setTitle("CH " + QString::number(i+1));
		} else {
			m_plot->Curve(i)->setTitle("DIAG " + QString::number(i+1));
		}
		m_plot->DetachCurve(i);
		m_plot->setOffsetHandleVisible(i, false);
		m_plot->addZoomer(i);
		m_plot->setYaxisUnit("V", i);
	}
	m_plot->configureAllYAxis();
	m_plot->setAllYAxis(-5, 5);
	m_plot->setOffsetInterval(-__DBL_MAX__, __DBL_MAX__);
	connect(m_plot, &CapturePlot::channelSelected, this, [=](int hdlIdx, bool selected) {
		m_plot->setActiveVertAxis(hdlIdx, true);
		Q_EMIT offsetHandleSelected(hdlIdx, selected);
	});
	connect(m_plot->getZoomer(), &OscPlotZoomer::zoomFinished, [=](bool isZoomOut){
		if (isZoomOut) {
			m_plot->setAxisScale(QwtAxisId(QwtAxis::XBottom, 0), 0, m_timespan);
			m_plot->replot();
		}
	});
}

void BufferPlotHandler::initStatusWidget()
{
	QWidget *statusWidget = new QWidget();
	QHBoxLayout *statusLayout = new QHBoxLayout(statusWidget);

	m_samplesAquiredLabel = new QLabel("0");
	m_plotSamplesNumber = new QLabel(QString::number(m_samplingFreq));
	m_btnInfoStatus = new scopy::LinkedButton();
	m_btnInfoStatus->installEventFilter(this);
	m_btnInfoStatus->setObjectName(QString::fromUtf8("btnHelp"));
	m_btnInfoStatus->setCheckable(false);
	m_btnInfoStatus->setText(QString());
	statusLayout->insertWidget(0, m_samplesAquiredLabel);
	statusLayout->insertWidget(1, new QLabel("Samples at"));
	statusLayout->insertWidget(2, m_plotSamplesNumber);
	statusLayout->insertWidget(3, new QLabel("sps"));
	statusLayout->insertWidget(4, m_btnInfoStatus);
	m_plot->setStatusWidget(statusWidget);
}

//bufferCounter is used only for debug
void BufferPlotHandler::onBufferRefilled(QVector<QVector<double>> bufferData, int bufferCounter)
{
	int bufferDataSize = bufferData.size();
	int enPlotIndex = 0;
	bool rolling = false;
	auto plotSampleRate = (bufferDataSize > 0) ? ( m_samplingFreq / bufferDataSize ) : m_samplingFreq;
	m_lock->lock();
	resetDataPoints();
	if (!(m_singleCapture && (m_bufferIndex == m_buffersNumber))) {
		if (bufferDataSize > 0) {
			for (int i = 0; i < m_dataPointsDeque.size(); i++) {
				if (m_bufferIndex == m_buffersNumber)
				{
					int dequeSize = m_dataPointsDeque[i].size();
					if (dequeSize > 0) {
						m_dataPointsDeque[i].pop_front();
						rolling = true;
					}
				} else {
					int plotDataSamplesNumber = m_buffersNumber * m_bufferSize;
					int currentPlotDataSamplesNumber = (m_bufferIndex + 1) * m_bufferSize;
					m_plot->setDataStartingPoint(plotDataSamplesNumber - currentPlotDataSamplesNumber);
					m_plot->resetXaxisOnNextReceivedData();
					m_samplesAquiredLabel->setText(QString::number(currentPlotDataSamplesNumber));
				}
				if (m_enabledPlots[i]) {
					if (enPlotIndex < bufferDataSize) {
						m_dataPointsDeque[i].push_back(bufferData[enPlotIndex]);
						enPlotIndex++;
					}
				}
			}
			if (!rolling) {
				m_bufferIndex++;
			}
			drawPlot();
		}
	} else {
		Q_EMIT singleCaptureFinished();
	}
	m_lock->unlock();
}

void BufferPlotHandler::drawPlot()
{
	int dataPointsNumber = m_bufferIndex * m_bufferSize;
	for (int i = 0; i < m_dataPointsDeque.size(); i++) {
		m_dataPoints.push_back(new double[dataPointsNumber]());
		int dequeSize = m_dataPointsDeque[i].size();
		if (dequeSize > 0) {
			for (int j = 0; j < dequeSize; j++) {
				std::copy(m_dataPointsDeque[i][j].begin(), m_dataPointsDeque[i][j].end(), m_dataPoints[i] + (j * m_bufferSize));
			}
		}
	}
	m_plot->plotNewData("Active Channels", m_dataPoints, dataPointsNumber, 1);

}

void BufferPlotHandler::onBtnExportClicked(QMap<int, bool> exportConfig)
{
	QStringList filter;
	bool useNativeDialogs = false;
	filter += QString(tr("Comma-separated values files (*.csv)"));
	filter += QString(tr("Tab-delimited values files (*.txt)"));
	filter += QString(tr("All Files(*)"));

	QString selectedFilter = filter[0];

	QString fileName = QFileDialog::getSaveFileName(this,
							tr("Export"), "", filter.join(";;"),
							&selectedFilter, (useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));

	if (fileName.split(".").size() <= 1) {
		// file name w/o extension. Let's append it
		QString ext = selectedFilter.split(".")[1].split(")")[0];
		fileName += "." + ext;
	}
	bool atleastOneChannelEnabled = false;

	auto keys = exportConfig.keys();
	for (auto x : qAsConst(keys))
		if (exportConfig[x]){
			atleastOneChannelEnabled = true;
			break;
		}
	if (!atleastOneChannelEnabled){
		return;
	}

	if (!fileName.isEmpty()){
		FileManager fm("SWIOTad74413rRuntime");
		fm.open(fileName, FileManager::EXPORT);

		int channels_number = m_plotChnlsNo;
		QVector<double> time_data;

		for (size_t i = 0; i < m_plot->Curve(0)->data()->size(); ++i) {
			time_data.push_back(m_plot->Curve(0)->sample(i).x());
		}

		fm.save(time_data, "Time(S)");

		for (int i = 0; i < channels_number; ++i){
			if (exportConfig[i]){
				QVector<double> data;
				int samples = m_plot->Curve(i)->data()->size();
				for (int j = 0; j < samples; ++j)
					data.push_back(m_plot->Curve(i)->data()->sample(j).y());
				QString chNo = (i > 1) ? QString::number(i - 1) : QString::number(i + 1);

				fm.save(data, "CH" + chNo + "(V)");
			}
		}
		auto enabledPlotsNo = std::count(m_enabledPlots.begin(), m_enabledPlots.end(), true);
		auto plotSampleRate = (enabledPlotsNo > 0) ? ( m_samplingFreq / enabledPlotsNo ) : m_samplingFreq;
		fm.setSampleRate(plotSampleRate);
		fm.performWrite();
	}
}

void BufferPlotHandler::onTimespanChanged(double value)
{
	m_timespan = value;
	resetPlotParameters();
}

void BufferPlotHandler::onSamplingFreqWritten(int samplingFreq)
{
	m_samplingFreq = samplingFreq;
	resetPlotParameters();
}

void BufferPlotHandler::setSingleCapture(bool en)
{
	m_singleCapture = en;
}

QColor BufferPlotHandler::getCurveColor(int id) const
{
	return m_plot->getLineColor(id);
}

QWidget *BufferPlotHandler::getPlotWidget() const
{
	return m_plotWidget;
}

void BufferPlotHandler::onChannelWidgetEnabled(int curveId, bool en)
{
	m_enabledPlots[curveId] = en;
	m_plot->setOffsetHandleVisible(curveId, en);
	if (en) {
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

void BufferPlotHandler::resetPlotParameters()
{
	m_lock->lock();
	int enabledPlotsNo = std::count(m_enabledPlots.begin(), m_enabledPlots.end(), true);
	auto plotSampleRate = (enabledPlotsNo > 0) ? ( m_samplingFreq / enabledPlotsNo ) : m_samplingFreq;
	int plotSampleNumber = m_samplingFreq * m_timespan;

	plotSampleNumber = (enabledPlotsNo > 0) ? (plotSampleNumber / enabledPlotsNo) : plotSampleNumber;
	m_bufferSize = (m_samplingFreq > MAX_BUFFER_SIZE) ? MAX_BUFFER_SIZE : MIN_BUFFER_SIZE;
	m_buffersNumber = ((plotSampleNumber % m_bufferSize) == 0) ?
				(plotSampleNumber / m_bufferSize) : ((plotSampleNumber / m_bufferSize) + 1);
	m_bufferIndex = 0;
	resetDeque();

	m_plotSamplesNumber->setText(QString::number(plotSampleRate));

	m_plot->setSampleRate(plotSampleRate, 1, "");
	m_plot->setAxisScale(QwtAxisId(QwtAxis::XBottom, 0), 0, m_timespan);
	m_plot->replot();
	qDebug(CAT_SWIOT_AD74413R) << "Plot samples number: " << QString::number(plotSampleNumber) <<" "<<QString::number(m_buffersNumber)
				      +" "  + QString::number(plotSampleNumber / m_bufferSize) + " ";
	m_lock->unlock();
}

void BufferPlotHandler::resetDataPoints()
{
	for (int i = 0; i < m_dataPoints.size(); i++) {
		delete[] m_dataPoints[i];
		m_dataPoints[i] = nullptr;
	}
	m_dataPoints.clear();
}

void BufferPlotHandler::resetDeque()
{
	resetDataPoints();
	for(int i = 0; i < m_dataPointsDeque.size(); i++) {
		m_dataPointsDeque.clear();
	}
	m_dataPointsDeque.clear();
	for (int i = 0; i < m_plotChnlsNo; i++) {
		m_dataPointsDeque.push_back(std::deque<QVector<double>>());
	}
}

void BufferPlotHandler::readPreferences()
{
	Preferences *p = Preferences::GetInstance();
	bool showFps = p->get("general_show_plot_fps").toBool();
	m_plot->setVisibleFpsLabel(showFps);
}

bool BufferPlotHandler::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == (QObject*)m_btnInfoStatus) {
		if (event->type() == QEvent::Enter)
		{
			auto enabledPlotsNo = std::count(m_enabledPlots.begin(), m_enabledPlots.end(), true);
			m_btnInfoStatus->setToolTip("sps = samples per second \n"
					      "sps = sampling_frequency / enabled_channels \n"
					      "Enabled channels = " + QString::number(enabledPlotsNo) + "\n" +
					      "Samples per channel = " + m_samplesAquiredLabel->text() + "\n" +
					      "Sampling frequency = " + QString::number(m_samplingFreq));
		}
		return false;
	} else {
		return QWidget::eventFilter(obj, event);
	}
}

bool BufferPlotHandler::singleCapture() const
{
	return m_singleCapture;
}
