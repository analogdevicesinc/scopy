#include "swiotad.hpp"
#include "src/tool/tool_view_builder.hpp"
#include "swiotgenericmenu.hpp"
#include "swiotmodel.hpp"
#include <iio.h>
#include <core/logging_categories.h>

using namespace adiscope;

SwiotAd::SwiotAd(QWidget* parent, ToolLauncher *toolLauncher, struct iio_device* iioDev, QVector<QString> chnlsFunc):
	m_iioDev(iioDev)
      ,m_widget(parent)
      ,m_testToggle(true)
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

		m_plot = new CapturePlot(parent, false, 16, 10, new TimePrefixFormatter, new MetricPrefixFormatter);
		initPlot();

		m_toolView = adiscope::gui::ToolViewBuilder(recipe, m_monitorChannelManager, toolLauncher).build();
		m_toolView->addFixedCentralWidget(m_plotWidget, 0, 0, 0, 0);
		initMonitorToolView();

		connect(m_toolView->getRunBtn(), &QPushButton::toggled, this, &SwiotAd::onRunBtnPressed);
		connect(m_readerThread, &SwiotAdReaderThread::bufferRefilled, this, &SwiotAd::onBufferRefilled, Qt::DirectConnection);
		connect(m_swiotAdLogic, &SwiotAdLogic::bufferCreated, m_readerThread, &SwiotAdReaderThread::onBufferCreated);
		connect(m_swiotAdLogic, &SwiotAdLogic::bufferDestroyed, m_readerThread, &SwiotAdReaderThread::onBufferDestroyed);
		connect(this, &SwiotAd::chnlsStatusChanged, m_readerThread, &SwiotAdReaderThread::onChnlsStatusChanged, Qt::DirectConnection);
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
	qDebug(CAT_SWIOT_RUNTIME) << plotChnlsNo;
	int configuredChnlsNo = plotChnlsNo / 2;
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

	m_plot->setSampleRate(20, 1, "");
	//number of curves; For TimeDomainDisplayPlot only
//	m_plot->setLeftVertAxesCount(m_chnlsFunction.size());
	m_plot->enableTimeTrigger(false);
	m_plot->setActiveVertAxis(0, true);
	// by default is in us; in this way we set the x axis values between -5 and 5 seconds
	m_plot->setAxisScale(QwtAxisId(QwtAxis::XBottom, 0), -5.0, 5.0);

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
	m_plot->setOffsetInterval(-DBL_MAX, DBL_MAX);
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
	//chId starts from 0
	for (int i = 0; i < m_chnlsFunction.size(); i++) {
		if (m_chnlsFunction[i].compare("high_z") != 0) {
			adiscope::gui::SwiotGenericMenu *menu = new adiscope::gui::SwiotGenericMenu(m_widget);
			menu->init(((deviceName + " - Channel ") + QString::number(i+1)) + (": " + m_chnlsFunction[i])
				   , m_chnlsFunction[i], new QColor(m_plot->getLineColor(chId)));

			struct iio_channel* iioChnl = m_swiotAdLogic->getIioChnl(i, true);
			SwiotAdModel* swiotModel = new SwiotAdModel(iioChnl);
			SwiotController* controller = new SwiotController(menu, swiotModel);

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
			m_channelWidgetList.push_back(chWidget);
			chId++;
		}
	}

	m_toolView->buildChannelGroup(m_monitorChannelManager, mainCh_widget, m_channelWidgetList);
	connectChnlsWidgesToPlot(m_channelWidgetList);
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


	if (en) {
		m_plot->AttachCurve(id);
		m_enabledChannels[id] = true;
		verifyChnlsChanges();
	}
	else {
		m_plot->DetachCurve(id);
		m_enabledChannels[id] = false;
		verifyChnlsChanges();
	}
	m_plot->replot();
}

void SwiotAd::onChannelWidgetSelected(bool checked)
{
//	if (!checked) {
//		return;
//	}
	ChannelWidget *w = static_cast<ChannelWidget *>(QObject::sender());
	int id = w->id();
	m_plot->setActiveVertAxis(id, true);

}

void SwiotAd::onRunBtnPressed()
{
	if (m_toolView->getRunBtn()->isChecked()) {
		double values[] =  {0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1,1.1,1.2,1.3,1.4,1.5,1.6,1.7,1.8,1.9,2};
		double values1[] = {-0.1,-0.2,-0.3,-0.4,-0.5,-0.6,-0.7,-0.8,-0.9,-1,-1.1,-1.2,-1.3,-1.4,-1.5,-1.6,-1.7,-1.8,-1.9,-2};
		double values2[] = {2.1,2.2,2.3,2.4,2.5,2.6,2.7,2.8,2.9,3,3.1,3.2,3.3,3.4,3.5,3.6,3.7,3.8,3.9,4};
		double values3[] = {-2.1,-2.2,-2.3,-2.4,-2.5,-2.6,-2.7,-2.8,-2.9,-3,-3.1,-3.2,-3.3,-3.4,-3.5,-3.6,-3.7,-3.8,-3.9,-4};
		double values4[] = {};
		std::vector<double*> dataPoints;
		if (m_testToggle) {
			dataPoints = {values, values1, values2, values3, values4, values4, values4, values4};
		}
		else{
			dataPoints = {values1, values, values2, values3, values4, values4, values4, values4};
		}

		m_plot->plotNewData("Active Channels", dataPoints, 20, 1);
		m_plot->replot();
		verifyChnlsChanges();

	} else {
		m_readerThread->requestInterruption();
		m_testToggle = !m_testToggle;
	}
}

void SwiotAd::drawCurves(std::vector<double*> dataPoints)
{
	int sampleRate = 60;
	m_plot->plotNewData("Active Channels", dataPoints, sampleRate, 1);
	m_plot->replot();
}

void SwiotAd::verifyChnlsChanges()
{
	bool changes = m_swiotAdLogic->verifyEnableChanges(m_enabledChannels);
	if (changes) {
		Q_EMIT chnlsStatusChanged();
		m_swiotAdLogic->destroyIioBuffer();
		m_swiotAdLogic->enableIioChnls(changes);
		m_swiotAdLogic->createIioBuffer(64);
	}
	if (m_toolView->getRunBtn()->isChecked()) {
		m_readerThread->start();
	}

}

void SwiotAd::onBufferRefilled(QVector<QVector<double>> bufferData)
{
	std::vector<double*> dataPoints;
	int j = 0;
	double values[] = {};

	for (int i = 0; i < m_channelWidgetList.size(); i++) {
		qDebug(CAT_SWIOT_RUNTIME) << bufferData[i];
		if (m_channelWidgetList[j]->isEnabled()) {
			dataPoints.push_back(&bufferData[j][0]);
			j++;
		} else {
			dataPoints.push_back(values);
		}

	}
	drawCurves(dataPoints);
}

adiscope::gui::ToolView* SwiotAd::getToolView()
{
	return m_toolView;
}

void SwiotAd::setChannelsFunction(QVector<QString> chnlsFunction)
{
	m_chnlsFunction = chnlsFunction;
}

