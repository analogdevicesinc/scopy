#include "ad74413r.hpp"
#include "src/refactoring/tool/tool_view_builder.hpp"
#include "buffermenuview.hpp"
#include "buffermenumodel.hpp"
#include <iio.h>
#include "core/logging_categories.h"

using namespace adiscope::swiot;

Ad74413r::Ad74413r(QWidget* parent, struct iio_device* iioDev, QVector<QString> chnlsFunc):
	m_iioDev(iioDev)
      ,m_swiotAdLogic(nullptr)
      ,m_widget(parent)
{
	m_chnlsFunction = chnlsFunc;
	m_enabledChannels = std::vector<bool>(m_chnlsFunction.size(), false);
	if (m_iioDev) {
		m_swiotAdLogic = new BufferLogic(m_iioDev);
		m_readerThread = new ReaderThread(true);
                m_readerThread->addBufferedDevice(m_iioDev);

		m_plotHandler = new BufferPlotHandler(parent, m_swiotAdLogic->getPlotChnlsNo());
		m_enabledPlots = std::vector<bool>(m_swiotAdLogic->getPlotChnlsNo(), false);

		setupToolView();
		initMonitorToolView();
		setupConnections();
	}
}

Ad74413r::~Ad74413r()
{
	if (m_readerThread->isRunning()) {
		m_readerThread->requestInterruption();
		m_readerThread->quit();
		m_readerThread->wait();
	}
	if (m_readerThread) {
		delete m_readerThread;
	}
	if (m_controllers.size() > 0) {
		m_controllers.clear();
	}
}

void Ad74413r::setupToolView()
{
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

	m_toolView = adiscope::gui::ToolViewBuilder(recipe, m_monitorChannelManager, m_widget).build();

	gui::GenericMenu *settingsMenu = createSettingsMenu("General settings", new QColor("Red"));
	m_toolView->setGeneralSettingsMenu(settingsMenu, true);

	m_toolView->addFixedCentralWidget(m_plotHandler->getPlotWidget(), 0, 0, 0, 0);
}

void Ad74413r::setupConnections()
{
	connect(m_toolView->getRunBtn(), &QPushButton::toggled, this, &Ad74413r::onRunBtnPressed);
	connect(m_swiotAdLogic, &BufferLogic::chnlsChanged, m_readerThread, &ReaderThread::onChnlsChange);
	connect(this, &Ad74413r::plotChnlsChanges, m_plotHandler, &BufferPlotHandler::onPlotChnlsChanges);
	connect(m_readerThread, &ReaderThread::bufferRefilled, m_plotHandler, &BufferPlotHandler::onBufferRefilled, Qt::QueuedConnection);
	connect(m_readerThread, &ReaderThread::finished, this, &Ad74413r::onReaderThreadFinished, Qt::QueuedConnection);
	//general settings connections
	connect(m_timespanSpin, &PositionSpinButton::valueChanged, m_plotHandler, &BufferPlotHandler::onTimespanChanged);
	connect(m_samplingFreqOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), m_swiotAdLogic, &BufferLogic::onSamplingFreqChanged);
}

void Ad74413r::initMonitorToolView()
{
	int mainChId = m_chnlsFunction.size();
	int chId = 0;
	bool first = true;

	QString deviceName(iio_device_get_name(m_iioDev));
	ChannelWidget *mainCh_widget =
			m_toolView->buildNewChannel(m_monitorChannelManager, nullptr, false, mainChId, false, false,
						    QColor("green"), deviceName, deviceName);
	std::vector<ChannelWidget*> channelWidgetList;
	for (int i = 0; i < m_chnlsFunction.size(); i++) {
		if (m_chnlsFunction[i].compare("high_z") != 0) {
			BufferMenuView *menu = new BufferMenuView(m_widget);
			menu->init(((deviceName + " - Channel ") + QString::number(i+1)) + (": " + m_chnlsFunction[i])
				   , m_chnlsFunction[i], new QColor(m_plotHandler->getCurveColor(chId)));

			struct iio_channel* iioChnl = m_swiotAdLogic->getIioChnl(i, true);
			BufferMenuModel* swiotModel = new BufferMenuModel(iioChnl);
			BufferMenuController* controller = new BufferMenuController(menu, swiotModel, i);

			controller->addMenuAttrValues();
			if (controller) {
				m_controllers.push_back(controller);
			}
			ChannelWidget *chWidget =
					m_toolView->buildNewChannel(m_monitorChannelManager, menu, false, chId, false, false,
								    m_plotHandler->getCurveColor(chId),QString::fromStdString("channel"),
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

void Ad74413r::initExportSettings(QWidget *parent)
{
	int plotChnlsNo = m_swiotAdLogic->getPlotChnlsNo();
	m_exportSettings = new ExportSettings(parent);
	m_exportSettings->enableExportButton(false);

	for (int i = 0; i < plotChnlsNo; i++){
		m_exportSettings->addChannel(i, QString("Channel") +
					     QString::number(i+1));
	}

	connect(m_exportSettings->getExportButton(), &QPushButton::clicked, this, [=](){
		Q_EMIT exportBtnClicked(m_exportSettings->getExportConfig());
	});
	connect(this, &Ad74413r::exportBtnClicked, m_plotHandler, &BufferPlotHandler::onBtnExportClicked);
	connect(this, &Ad74413r::activateExportButton,
		[=](){
		m_exportSettings->enableExportButton(true);
	});
}

adiscope::gui::GenericMenu* Ad74413r::createSettingsMenu(QString title, QColor* color)
{
	adiscope::gui::GenericMenu *menu = new adiscope::gui::GenericMenu(this);
	menu->initInteractiveMenu();
	menu->setMenuHeader(title,color,false);

	auto *generalSubsection = new adiscope::gui::SubsectionSeparator("Acquisition settings", false, this);
	generalSubsection->getContentWidget()->layout()->setSpacing(10);

	//channels sampling freq
	auto *samplingFreqLayout = new QHBoxLayout();
	m_samplingFreqOptions = new QComboBox(generalSubsection->getContentWidget());

	QStringList actualSamplingFreq = m_swiotAdLogic->readChnlsFrequencyAttr("sampling_frequency");
	QStringList samplingFreqValues = m_swiotAdLogic->readChnlsFrequencyAttr("sampling_frequency_available");
	int actualSamplingFreqIdx = 0;
	int valuesIdx = 0;
	for (QString val : samplingFreqValues) {
		m_samplingFreqOptions->addItem(val);
		if (val.compare(actualSamplingFreq[0]) == 0) {
			actualSamplingFreqIdx = valuesIdx;
		}
		valuesIdx++;
	}
	m_samplingFreqOptions->setCurrentIndex(actualSamplingFreqIdx);
	samplingFreqLayout->addWidget(new QLabel("Sampling frequency",generalSubsection->getContentWidget()));
	samplingFreqLayout->addWidget(m_samplingFreqOptions);

	//plot timespan
	auto *timespanLayout = new QHBoxLayout();
	m_timespanSpin = new PositionSpinButton({
							{"ms",1E-3},
							{"s", 1E0}},
						"Timespan",0,10,
						true, false, generalSubsection->getContentWidget());
	timespanLayout->addWidget(m_timespanSpin);

	//export section
	auto *exportLayout = new QHBoxLayout();

	initExportSettings(generalSubsection->getContentWidget());
	exportLayout->addWidget(m_exportSettings);
	generalSubsection->getContentWidget()->layout()->addItem(samplingFreqLayout);
	generalSubsection->getContentWidget()->layout()->addItem(timespanLayout);
	generalSubsection->getContentWidget()->layout()->addItem(exportLayout);

	menu->insertSection(generalSubsection);

	return menu;
}

void Ad74413r::connectChnlsWidgesToPlot(std::vector<ChannelWidget*> channelList)
{
	for (int i = 0; i < channelList.size(); i++) {
		connect(channelList[i], SIGNAL(enabled(bool)),
			SLOT(onChannelWidgetEnabled(bool)));
		connect(channelList[i], SIGNAL(selected(bool)),
			SLOT(onChannelWidgetSelected(bool)));
	}
}

void Ad74413r::onChannelWidgetEnabled(bool en)
{
	ChannelWidget *w = static_cast<ChannelWidget *>(QObject::sender());
	int id = w->id();
	int chnlIdx = m_controllers[id]->getChnlIdx();

	if (en) {
		m_enabledPlots[id] = true;
		m_enabledChannels[chnlIdx] = true;
		verifyChnlsChanges();
	}
	else {
		m_enabledPlots[id] = false;
		m_enabledChannels[chnlIdx] = false;
		verifyChnlsChanges();
	}
	Q_EMIT plotChnlsChanges(m_enabledPlots);
}

void Ad74413r::onChannelWidgetSelected(bool checked)
{
	ChannelWidget *w = static_cast<ChannelWidget *>(QObject::sender());
	int id = w->id();
	m_plotHandler->setPlotActiveAxis(id);

}

void Ad74413r::onRunBtnPressed()
{
	Q_EMIT activateExportButton();
	if (m_toolView->getRunBtn()->isChecked()) {
		m_samplingFreqOptions->setEnabled(false);
		verifyChnlsChanges();
		if (!m_readerThread->isRunning()) {
			m_plotHandler->resetPlot();
			m_readerThread->start();
		}
	} else {
		m_samplingFreqOptions->setEnabled(true);
		m_readerThread->requestInterruption();
	}
}

void Ad74413r::verifyChnlsChanges()
{
	bool changes = m_swiotAdLogic->verifyEnableChanges(m_enabledChannels);
	if (changes) {
		m_readerThread->requestInterruption();
	}
}

adiscope::gui::ToolView* Ad74413r::getToolView()
{
	return m_toolView;
}

void Ad74413r::setChannelsFunction(QVector<QString> chnlsFunction)
{
	m_chnlsFunction = chnlsFunction;
}

void Ad74413r::onReaderThreadFinished()
{
	qDebug(CAT_SWIOT_RUNTIME) << "reader thread finished";
	if (m_toolView->getRunBtn()->isChecked()) {
		m_plotHandler->resetPlot();
		m_readerThread->start();
	}
}
