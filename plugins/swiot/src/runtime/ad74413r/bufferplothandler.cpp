#include "bufferplothandler.hpp"
#include <QGridLayout>
#include "src/captureplot/filemanager.h"
#include "src/swiot_logging_categories.h"
#include <QFileDialog>
#include <unistd.h>

using namespace adiscope::swiot;

BufferPlotHandler::BufferPlotHandler(QWidget *parent, int plotChnlsNo) :
	QWidget(parent)
{
	m_lock = new QMutex();
	m_plotChnlsNo = plotChnlsNo;
	for (int i = 0; i < plotChnlsNo; i++) {
		m_dataPoints.push_back(new double[m_plotSampleNumber]);
	}
	m_plot = new CapturePlot(parent, false, 16, 10, new TimePrefixFormatter, new MetricPrefixFormatter);
	initPlot(plotChnlsNo);
}
BufferPlotHandler::~BufferPlotHandler()
{
	if (m_dataPoints.size() > 0) {
		for (int i = 0; i < m_dataPoints.size(); i++) {
			delete[] m_dataPoints[i];
			m_dataPoints[i] = nullptr;
		}
	}
}

void BufferPlotHandler::initPlot(int plotChnlsNo)
{
	//the last 4 channels are for diagnostic
	int configuredChnlsNo = plotChnlsNo / 2;

	m_enabledPlots = std::vector<bool>(plotChnlsNo, false);

	m_plot->registerSink("Active Channels", plotChnlsNo, 0);
	m_plot->disableLegend();

	m_plotWidget = new QWidget(this);

	QGridLayout *gridPlot = new QGridLayout(m_plotWidget);
	gridPlot->setVerticalSpacing(0);
	gridPlot->setHorizontalSpacing(0);
	gridPlot->setContentsMargins(9, 0, 9, 0);
	m_plotWidget->setLayout(gridPlot);

	QSpacerItem *plotSpacer = new QSpacerItem(0, 5,
						  QSizePolicy::Fixed, QSizePolicy::Fixed);

	gridPlot->addWidget(m_plot->topArea(), 1, 0, 1, 4);
	gridPlot->addWidget(m_plot->leftHandlesArea(), 1, 0, 4, 1);
	gridPlot->addWidget(m_plot, 3, 1, 1, 1);
	gridPlot->addItem(plotSpacer, 5, 0, 1, 4);

	m_plot->setSampleRate(m_sampleRate, 1, "");
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

	}
	m_plot->setAllYAxis(0, 20000);
	m_plot->setOffsetInterval(__DBL_MIN__, __DBL_MAX__);
}

void BufferPlotHandler::onBufferRefilled(QVector<QVector<double>> bufferData, int bufferCounter)
{
	int bufferDataSize = bufferData.size();
	int j = 0;
	bool rolling = false;

	m_lock->lock();
	//	qDebug(CAT_SWIOT_RUNTIME) <<QString::number(bufferCounter)+" Before Copy";
	if (bufferDataSize > 0) {
		for (int i = 0; i < m_dataPoints.size(); i++) {
			if (m_bufferIndex == (m_buffersNumber - 1))
			{
				memmove(m_dataPoints[i], m_dataPoints[i] + MAX_BUFFER_SIZE, (m_plotSampleNumber - MAX_BUFFER_SIZE) * sizeof(double));
				rolling = true;
			}
			if (m_enabledPlots[i]) {
				if (j < bufferDataSize) {
					memmove(m_dataPoints[i]+m_plotDataIndex, &bufferData[j][0], (bufferData[j].size() * sizeof(double)));
					j++;
				} else {
					memmove(m_dataPoints[i]+m_plotDataIndex, &bufferData[0][0], bufferData[0].size() * sizeof(double));
				}

			} else {
				memmove(m_dataPoints[i]+m_plotDataIndex, &bufferData[0][0], bufferData[0].size() * sizeof(double));
			}

		}
		if (!rolling) {
			m_bufferIndex++;
			m_plotDataIndex+=MAX_BUFFER_SIZE;
		}
		if (m_plotDataIndex >= m_plotSampleNumber - 1) {
			m_plotDataIndex-=MAX_BUFFER_SIZE;
		}
		//		qDebug(CAT_SWIOT_RUNTIME) << QString::number(bufferCounter)+" After Copy";

	}
	//	qDebug(CAT_SWIOT_RUNTIME) << QString::number(bufferCounter)+" Before Drawing";
	m_plot->plotNewData("Active Channels", m_dataPoints, m_plotSampleNumber, 1);
	//	qDebug(CAT_SWIOT_RUNTIME) << QString::number(bufferCounter)+" After Drawing";
	m_lock->unlock();
}

void BufferPlotHandler::onPlotChnlsChanges(std::vector<bool> enabledPlots)
{
	m_enabledPlots = enabledPlots;
	for (int i = 0; i < m_enabledPlots.size(); i++) {
		if (m_enabledPlots[i]) {
			m_plot->AttachCurve(i);
		} else {
			m_plot->DetachCurve(i);
		}
	}
	qDebug(CAT_SWIOT_AD74413R) << "Sample number on chnl changes:" << QString::number(m_plotSampleNumber);
}

void BufferPlotHandler::onBtnExportClicked(QMap<int, bool> exportConfig)
{
	//	pause(true);
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
		fm.setSampleRate(m_plotSampleNumber);
		fm.performWrite();
	}
	//	pause(false);
}

void BufferPlotHandler::setPlotActiveAxis(int id) {
	m_plot->setActiveVertAxis(id, true);
}

QColor BufferPlotHandler::getCurveColor(int id) const
{
	return m_plot->getLineColor(id);
}

QWidget *BufferPlotHandler::getPlotWidget() const
{
	return m_plotWidget;
}

void BufferPlotHandler::resetPlot()
{
	int enabledPlotsNo = std::count(m_enabledPlots.begin(), m_enabledPlots.end(), true);
	m_bufferIndex = 0;
	m_plotDataIndex = 0;
	m_plotSampleNumber = m_sampleRate * m_timespan;
	m_plotSampleNumber = (enabledPlotsNo > 0) ? (m_plotSampleNumber / enabledPlotsNo) : m_plotSampleNumber;
	m_buffersNumber = (m_plotSampleNumber / MAX_BUFFER_SIZE) + 1;

	resetDataPoints();

	m_plot->setSampleRate(m_plotSampleNumber, 1, "");
	m_plot->setAxisScale(QwtAxisId(QwtAxis::XBottom, 0), 0, m_timespan);
	m_plot->replot();
	qDebug(CAT_SWIOT_AD74413R) << "Plot samples number: " << QString::number(m_plotSampleNumber);

}

void BufferPlotHandler::onTimespanChanged(double value)
{
	m_timespan = value;
	resetPlot();
}

void BufferPlotHandler::resetDataPoints()
{
	if (m_dataPoints.size() > 0) {
		for (int i = 0; i < m_dataPoints.size(); i++) {
			delete[] m_dataPoints[i];
			m_dataPoints[i] = nullptr;
		}
		m_dataPoints.clear();
		for (int i = 0; i < m_plotChnlsNo; i++) {
			m_dataPoints.push_back(new double[m_plotSampleNumber]);
		}
	}
}
