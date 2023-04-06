#include "swiotad.hpp"
#include "src/refactoring/tool/tool_view_builder.hpp"
#include "swiotgenericmenu.hpp"
#include "swiotmodel.hpp"
#include <iio.h>
#include <core/logging_categories.h>

using namespace adiscope;

SwiotAd::SwiotAd(QWidget* parent, struct iio_device* iioDev, QVector<QString> chnlsFunc):
	m_iioDev(iioDev)
      ,m_swiotAdLogic(nullptr)
      ,m_widget(parent)
{
	m_chnlsFunction = chnlsFunc;
	m_enabledChannels = std::vector<bool>(m_chnlsFunction.size(), false);
	qDebug(CAT_SWIOT_RUNTIME) << m_enabledChannels;
	if (m_iioDev) {
		m_swiotAdLogic = new SwiotAdLogic(m_iioDev);
		m_readerThread = new SwiotAdReaderThread();

		adiscope::gui::ToolViewRecipe recipe;
		recipe.helpBtnUrl = "";
		recipe.hasRunBtn = true;
		recipe.hasSingleBtn = true;
		recipe.hasPairSettingsBtn = true;
		recipe.hasPrintBtn = false;
		recipe.hasChannels = true;
		recipe.channelsPosition = adiscope::gui::ChannelsPositionEnum::VERTICAL;

		m_monitorChannelManager = new adiscope::gui::ChannelManager(recipe.channelsPosition);
		m_monitorChannelManager->setChannelIdVisible(false);
		m_monitorChannelManager->setToolStatus("Channels");

		m_plot = new CapturePlot(m_widget, false, 16, 10, new TimePrefixFormatter, new MetricPrefixFormatter);
		initPlot();

		m_toolView = adiscope::gui::ToolViewBuilder(recipe, m_monitorChannelManager, m_widget).build();
		m_toolView->addFixedCentralWidget(m_plotWidget, 0, 0, 0, 0);
		initMonitorToolView();

		connect(m_toolView->getRunBtn(), &QPushButton::toggled, this, &SwiotAd::onRunBtnPressed);
		connect(m_readerThread, &SwiotAdReaderThread::bufferRefilled, this, &SwiotAd::onBufferRefilled, Qt::QueuedConnection);
		connect(m_swiotAdLogic, &SwiotAdLogic::bufferCreated, m_readerThread, &SwiotAdReaderThread::onBufferCreated);
		connect(m_swiotAdLogic, &SwiotAdLogic::bufferDestroyed, m_readerThread, &SwiotAdReaderThread::onBufferDestroyed);
	}
}

SwiotAd::~SwiotAd()
{
	if (m_readerThread->isRunning()) {
		m_readerThread->requestInterruption();
		m_readerThread->quit();
		m_readerThread->wait();
	}
	if (m_readerThread) {
		delete m_readerThread;
	}

}

void SwiotAd::initPlot()
{
	//the last 4 channels are for diagnostic
	int plotChnlsNo = m_swiotAdLogic->getPlotChnlsNo();
	int configuredChnlsNo = plotChnlsNo / 2;

	m_enabledPlots = std::vector<bool>(plotChnlsNo, false);

	m_plot->registerSink("Active Channels", plotChnlsNo, 0);
	m_plot->disableLegend();

	m_plotWidget = new QWidget(this);
	m_gridPlot = new QGridLayout(m_plotWidget);
	m_gridPlot->setVerticalSpacing(0);
	m_gridPlot->setHorizontalSpacing(0);
	m_gridPlot->setContentsMargins(9, 0, 9, 0);
	m_plotWidget->setLayout(m_gridPlot);

	QSpacerItem *plotSpacer = new QSpacerItem(0, 5,
		QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_gridPlot->addWidget(m_plot->topArea(), 1, 0, 1, 4);
	m_gridPlot->addWidget(m_plot->leftHandlesArea(), 1, 0, 4, 1);
	m_gridPlot->addWidget(m_plot, 3, 1, 1, 1);
	m_gridPlot->addItem(plotSpacer, 5, 0, 1, 4);

	m_plot->setSampleRate(m_sampleRate, m_timespan, "");
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

void SwiotAd::initMonitorToolView()
{
	int mainChId = m_chnlsFunction.size();
	int chId = 0;
	bool first = true;

	QString deviceName(iio_device_get_name(m_iioDev));
	ChannelWidget *mainCh_widget =
			m_toolView->buildNewChannel(m_monitorChannelManager, nullptr, false, mainChId, false, false,
						    QColor("green"), deviceName, deviceName);
	std::vector<ChannelWidget*> channelWidgetList;
	//chId starts from 0
	for (int i = 0; i < m_chnlsFunction.size(); i++) {
		if (m_chnlsFunction[i].compare("high_z") != 0) {
			adiscope::gui::SwiotGenericMenu *menu = new adiscope::gui::SwiotGenericMenu(m_widget);
			menu->init(((deviceName + " - Channel ") + QString::number(i+1)) + (": " + m_chnlsFunction[i])
				   , m_chnlsFunction[i], new QColor(m_plot->getLineColor(chId)));

			struct iio_channel* iioChnl = m_swiotAdLogic->getIioChnl(i, true);
			SwiotAdModel* swiotModel = new SwiotAdModel(iioChnl);
			SwiotController* controller = new SwiotController(menu, swiotModel, i);

			controller->addMenuAttrValues();
			if (controller) {
				m_controllers.push_back(controller);
			}
			ChannelWidget *chWidget =
					m_toolView->buildNewChannel(m_monitorChannelManager, menu, false, chId, false, false,
								    m_plot->getLineColor(chId),QString::fromStdString("channel"),
								    m_chnlsFunction[i] +" "+QString::number(i+1));
			if (first) {
				chWidget->menuButton()->click();
				first = false;
			}
			controller->createConnections();
			channelWidgetList.push_back(chWidget);
			chId++;
		}
	}

	m_toolView->buildChannelGroup(m_monitorChannelManager, mainCh_widget, channelWidgetList);
	connectChnlsWidgesToPlot(channelWidgetList);
}

void SwiotAd::connectChnlsWidgesToPlot(std::vector<ChannelWidget*> channelList)
{
	for (int i = 0; i < channelList.size(); i++) {
		connect(channelList[i], SIGNAL(enabled(bool)),
					SLOT(onChannelWidgetEnabled(bool)));
		connect(channelList[i], SIGNAL(selected(bool)),
					SLOT(onChannelWidgetSelected(bool)));
	}
}

void SwiotAd::onChannelWidgetEnabled(bool en)
{
	ChannelWidget *w = static_cast<ChannelWidget *>(QObject::sender());
	int id = w->id();
	int chnlIdx = m_controllers[id]->getChnlIdx();

	if (en) {
		m_plot->AttachCurve(id);
		m_enabledPlots[id] = true;
		m_enabledChannels[chnlIdx] = true;
		verifyChnlsChanges();
	}
	else {
		m_plot->DetachCurve(id);
		m_enabledPlots[id] = false;
		m_enabledChannels[chnlIdx] = false;
		verifyChnlsChanges();
	}
}

void SwiotAd::onChannelWidgetSelected(bool checked)
{
	ChannelWidget *w = static_cast<ChannelWidget *>(QObject::sender());
	int id = w->id();
	m_plot->setActiveVertAxis(id, true);

}

void SwiotAd::onRunBtnPressed()
{
	if (m_toolView->getRunBtn()->isChecked()) {
		verifyChnlsChanges();
	} else {
		m_readerThread->requestInterruption();
	}
}

void SwiotAd::drawCurves(std::vector<double*> dataPoints, int numberOfPoints)
{
	//1 is time interval; that variable is never used in plotNewData method
	m_plot->plotNewData("Active Channels", dataPoints, numberOfPoints, 1);
}

void SwiotAd::verifyChnlsChanges()
{
	bool changes = m_swiotAdLogic->verifyEnableChanges(m_enabledChannels);
	if (changes) {
		m_readerThread->requestInterruption();
		m_readerThread->exit();
		m_swiotAdLogic->destroyIioBuffer();
		m_swiotAdLogic->enableIioChnls(changes);
		m_swiotAdLogic->createIioBuffer(m_sampleRate, m_timespan);
		resetPlot();
	}
	if (m_toolView->getRunBtn()->isChecked()) {
		qDebug(CAT_SWIOT_RUNTIME) << "Start";
		m_readerThread->start();
	}

}

void SwiotAd::onBufferRefilled(QVector<QVector<double>> bufferData)
{
	std::vector<double*> dataPoints;
	int bufferDataSize = bufferData.size();
	int j = 0;

	qDebug(CAT_SWIOT_RUNTIME) << "Samples per buffer: " + QString::number(bufferData[0].size());

	if (bufferDataSize > 0) {
		for (int i = 0; i < m_enabledPlots.size(); i++) {
			if (m_enabledPlots[i]) {
				if (j < bufferDataSize) {
					dataPoints.push_back(&bufferData[j][0]);
					j++;
				} else {
					dataPoints.push_back(&bufferData[0][0]);
				}

			} else {
				dataPoints.push_back(&bufferData[0][0]);
			}
		}
	}
	drawCurves(dataPoints, bufferData[0].size());
}

//must be called when the sample rate or timespan are changed
void SwiotAd::resetPlot()
{
	int sampleRate = 0;
	sampleRate = m_timespan * m_sampleRate;
	m_plot->setSampleRate(MAX_BUFFER_SIZE, 1, "");

//	m_plot->setAxisScale(QwtAxisId(QwtAxis::XBottom, 0), 0, m_timestamp);
}

adiscope::gui::ToolView* SwiotAd::getToolView()
{
	return m_toolView;
}

void SwiotAd::setChannelsFunction(QVector<QString> chnlsFunction)
{
	m_chnlsFunction = chnlsFunction;
}

