#include "datalogger.hpp"
#include "gui/generic_menu.hpp"
#include "gui/customSwitch.hpp"
#include "gui/detachedwindowsmanager.h"
#include "gui/dynamicWidget.hpp"
#include <QtConcurrent>
#include <QFuture>

#include "gui/dataloggercontroller.hpp"
#include "gui/customSwitch.hpp"
#include "datalogger_api.h"
#include "gui/channelmonitorcomponent.hpp"
#include "dataloggergenericmenu.hpp"
#include "gui/customcolqgridlayout.hpp"

using namespace adiscope;

DataLogger::DataLogger(struct iio_context *ctx, Filter *filt,
							   ToolMenuItem *toolMenuItem,	QJSEngine *engine, ToolLauncher *parent):
	Tool(ctx, toolMenuItem, new DataLogger_API(this), "DataLogger", parent),
	m_timer(new QTimer(this)),
	m_context(contextOpen(ctx, ""))
{
	run_button = nullptr;

	m_colors = {QColor("#ff7200"),QColor("#9013fe"), QColor(Qt::green),QColor(Qt::cyan), QColor(Qt::magenta),
				QColor(Qt::yellow), QColor(Qt::gray), QColor(Qt::darkRed), QColor(Qt::darkGreen),
				QColor(Qt::darkBlue), QColor(Qt::darkGray),QColor(Qt::black)};

	scopy::gui::ToolViewRecipe recepie;
	recepie.helpBtnUrl = "";
	recepie.hasRunBtn = true;
	recepie.hasSingleBtn = true;
	recepie.hasPairSettingsBtn = true;
	recepie.hasPrintBtn = false;
	recepie.hasChannels = true;
	recepie.channelsPosition = scopy::gui::ChannelsPositionEnum::VERTICAL;

	m_monitorChannelManager = new scopy::gui::ChannelManager(recepie.channelsPosition);
	m_monitorChannelManager->setChannelIdVisible(false);
	m_monitorChannelManager->setToolStatus("Stopped");

	m_toolView = scopy::gui::ToolViewBuilder(recepie,m_monitorChannelManager,parent).build();

	m_generalSettingsMenu = generateMenu("General settings", new QColor("#4a64ff"));
	m_toolView->setGeneralSettingsMenu(m_generalSettingsMenu,true);

	m_dmmList = getDmmList(m_context);

	connect(m_toolView->getRunBtn(), &QPushButton::toggled, this, [=](bool toggled){
		toggleTimer(toggled);

		dataLogger->setIsRunningOn(m_toolView->getRunBtn()->isChecked());

		//update status and if needed start data logging
		if (!m_toolView->getRunBtn()->isChecked()) {
			m_monitorChannelManager->setToolStatus("Stopped");
			showAllSWitch->setEnabled(true);

		} else {
			if (dataLogger->isDataLoggerOn()) {
				m_monitorChannelManager->setToolStatus("Data Logging");
			} else {
				m_monitorChannelManager->setToolStatus("Running");
			}
		}
	});

	connect(m_toolView->getRunBtn(), SIGNAL(toggled(bool)),runButton(), SLOT(setChecked(bool)));
	connect(runButton(), SIGNAL(toggled(bool)), m_toolView->getRunBtn(),SLOT(setChecked(bool)));
	connect(m_toolView->getSingleBtn(), &QPushButton::toggled,this, &DataLogger::readChannelValues);
	connect(m_toolView->getSingleBtn(), &QPushButton::clicked,this, [=](bool checked){
		m_toolView->getSingleBtn()->setChecked(!checked);
		m_toolView->getRunBtn()->toggled(false);
	});


	m_customColGrid = new CustomColQGridLayout(100,this);

	m_toolView->addFixedCentralWidget(m_customColGrid,0,0,0,0);

	connect(m_timer, &QTimer::timeout, this, &DataLogger::readChannelValues);
	connect(this, &DataLogger::recordingIntervalChanged, this , [=](double interval){
		VALUE_READING_TIME_INTERVAL = interval;
		m_timer->setInterval(interval);
		recording_timer->setValue(interval/1000);
	});

	setCentralWidget(getToolView());

	initMonitorToolView();

	api->setObjectName(QString::fromStdString(Filter::tool_name(TOOL_DATALOGGER)));
	api->load(*settings);
	api->js_register(engine);
}

void DataLogger::initMonitorToolView()
{
	int chId = 1;

	bool first = true;
	setUpdatesEnabled(false);

	for (libm2k::analog::DMM* dmm : m_dmmList) {

		auto dmmName = dmm->getName();
		std::vector<ChannelWidget*> channelList;

		adiscope::gui::DataLoggerGenericMenu *menu = new adiscope::gui::DataLoggerGenericMenu(this);
		menu->init(QString::fromStdString(dmmName),new QColor("green"));

		ChannelWidget *mainCh_widget =
				m_toolView->buildNewChannel(m_monitorChannelManager, menu, false, chId, false, false, QColor("green"),
							    QString::fromStdString(dmmName), QString::fromStdString(dmmName));
		if (first) {
			mainCh_widget->menuButton()->click();
			first = false;
		}
		chId++;
		auto dmmList = dmm->readAll();
		for (const auto &channel : dmmList) {
			QColor channelColor = getChannelColor(chId);
			adiscope::gui::DataLoggerGenericMenu *channelMenu = new adiscope::gui::DataLoggerGenericMenu(this);
			channelMenu->init(QString::fromStdString(dmmName + ": " + channel.id),new QColor(channelColor));

			ChannelWidget *ch_widget =
					m_toolView->buildNewChannel(m_monitorChannelManager, channelMenu , false, chId, false, false,
								    channelColor, QString::fromStdString(channel.id), QString::fromStdString(channel.id));

			channelList.push_back(ch_widget);
			ch_widget->enableButton()->setChecked(false);

			adiscope::ChannelMonitorComponent* monitor = new adiscope::ChannelMonitorComponent();
			monitor->setID(chId);
			monitor->init(0,QString::fromStdString(channel.unit_name),QString::fromStdString(channel.unit_symbol),
				      QString::fromStdString(dmmName + ": " + channel.id),channelColor );
			monitor->setChannelId(channel.id);
			monitor->setRecordingInterval(VALUE_READING_TIME_INTERVAL/1000);
			monitor->setHistoryDuration(10);

			connect(this, &DataLogger::channelValueUpdated, monitor, [=](int chId, double value,QString nameOfUnitOfMeasure,QString symbolOfUnitOfMeasure){
				if (chId == monitor->getID()) {
					monitor->updateValue(value, nameOfUnitOfMeasure, symbolOfUnitOfMeasure);
				}
			});

			connect(monitor, &adiscope::ChannelMonitorComponent::contentChanged, m_customColGrid, [=](){
				m_customColGrid->itemSizeChanged();
			}, Qt::QueuedConnection);

			connect(this, &DataLogger::recordingIntervalChanged, monitor , [=](double interval){
				monitor->setRecordingInterval(interval/1000);
			});

			//connect menu switches to widget
			createConnections(menu,channelMenu,monitor);

			connect(this, &DataLogger::toggleAll, this, [=](bool toggled){
				if (ch_widget->enableButton()->isChecked() != toggled) {
					ch_widget->enableButton()->click();
				}
			});

			//add connections from general settings
			connect(this, &DataLogger::precisionChanged, this, [=](int precision){
				monitor->updateLcdNumberPrecision(precision);
			});

			connect(this, &DataLogger::disableActivateChannel,this, [=](bool toggled){
				ch_widget->enableButton()->setDisabled(toggled);
			});

			int widgetId = m_customColGrid->addQWidgetToList(monitor);

			// logic for enable/disable channels
			connect(ch_widget, &ChannelWidget::enabled,this, [=](bool enabled){
				if (enabled) {
					m_customColGrid->addWidget(widgetId);
					m_activeChannels[chId].first.dmmId = channel.id;
					m_activeChannels[chId].first.dmm = dmm;
					m_activeChannels[chId].second = monitor;

				} else {
					m_activeChannels.remove(chId);
					m_customColGrid->removeWidget(widgetId);
				}
			});
			ch_widget->enableButton()->click();
			chId++;
		}
		m_toolView->buildChannelGroup(m_monitorChannelManager, mainCh_widget,channelList);
	}
	setUpdatesEnabled(true);
}

adiscope::gui::ToolView* DataLogger::getToolView()
{
	return m_toolView;
}

int DataLogger::getPrecision()
{
	return precisionValue->text().toInt();
}

void DataLogger::setPrecision(int precision)
{
	precisionValue->setText(QString::number(precision));
}

int DataLogger::getValueReadingTimeInterval()
{
	return VALUE_READING_TIME_INTERVAL;
}

std::vector<libm2k::analog::DMM*> DataLogger::getDmmList(libm2k::context::Context* context)
{
	return context->getAllDmm() ;
}

void DataLogger::readChannelValues()
{
	if (!m_activeChannels.empty()) {
		for (auto ch : m_activeChannels.keys()) {
			QtConcurrent::run(this,&DataLogger::updateChannelWidget,ch);
		}
	}
}

void DataLogger::updateChannelWidget(int ch)
{
	//check if channel was closed before reading
	if (m_activeChannels.contains(ch)) {
		auto updatedRead = m_activeChannels[ch].first.dmm->readChannel(m_activeChannels[ch].first.dmmId);
		Q_EMIT channelValueUpdated(ch, updatedRead.value,QString::fromStdString(updatedRead.unit_name),
					   QString::fromStdString(updatedRead.unit_symbol));
		if (dataLogger->isDataLoggerOn()) {
			Q_EMIT updateValue(QString::fromStdString(m_activeChannels[ch].first.dmm->getName() + ":" + m_activeChannels[ch].first.dmmId),
					   QString::number(updatedRead.value));
		}
	}
}

void DataLogger::toggleTimer(bool enabled)
{
	if (enabled) {
		m_timer->start(VALUE_READING_TIME_INTERVAL);
	} else {
		m_timer->stop();
	}
}

void DataLogger::createConnections(adiscope::gui::DataLoggerGenericMenu* mainMenu,adiscope::gui::DataLoggerGenericMenu* menu,adiscope::ChannelMonitorComponent* monitor)
{
	connect(menu,&adiscope::gui::DataLoggerGenericMenu::togglePeakHolder, monitor, &adiscope::ChannelMonitorComponent::displayPeakHold);
	connect(mainMenu,&adiscope::gui::DataLoggerGenericMenu::togglePeakHolder, menu, &adiscope::gui::DataLoggerGenericMenu::peakHolderToggle);

	connect(menu, &adiscope::gui::DataLoggerGenericMenu::resetPeakHolder, monitor, &adiscope::ChannelMonitorComponent::resetPeakHolder);
	connect(mainMenu,&adiscope::gui::DataLoggerGenericMenu::resetPeakHolder, menu, &adiscope::gui::DataLoggerGenericMenu::peakHolderResetClicked);

	connect(menu, &adiscope::gui::DataLoggerGenericMenu::toggleScale, monitor, &adiscope::ChannelMonitorComponent::displayScale);
	connect(mainMenu,&adiscope::gui::DataLoggerGenericMenu::toggleScale, menu, &adiscope::gui::DataLoggerGenericMenu::scaleToggle);

	connect(menu, &adiscope::gui::DataLoggerGenericMenu::toggleHistory, monitor, &adiscope::ChannelMonitorComponent::displayHistory);
	connect(mainMenu,&adiscope::gui::DataLoggerGenericMenu::toggleHistory, menu, &adiscope::gui::DataLoggerGenericMenu::historyToggle);

	connect(menu, &adiscope::gui::DataLoggerGenericMenu::changeHistorySize, monitor, &adiscope::ChannelMonitorComponent::setHistoryDuration);
	connect(mainMenu,&adiscope::gui::DataLoggerGenericMenu::historySizeIndexChanged, menu, &adiscope::gui::DataLoggerGenericMenu::historySizeChanged);

	connect(menu, &adiscope::gui::DataLoggerGenericMenu::lineStyleChanged, monitor, &adiscope::ChannelMonitorComponent::setLineStyle);
	connect(mainMenu,&adiscope::gui::DataLoggerGenericMenu::lineStyleIndexChanged, menu, &adiscope::gui::DataLoggerGenericMenu::changeLineStyle);

	connect(menu, &adiscope::gui::DataLoggerGenericMenu::monitorColorChanged, monitor, &ChannelMonitorComponent::setMonitorColor);
}

QColor DataLogger::generateColor()
{
	if (m_color.size() < m_colors.size()) {
		return m_colors.at(m_color.size());
	}
	int red = rand() % 256;
	int blue = rand() % 256;
	int green = rand() % 256;
	return QColor(red,green,blue);
}

QColor DataLogger::getChannelColor(int chId)
{
	if (m_color.contains(chId)) {
		return m_color[chId];
	} else {
		if (m_color.isEmpty()) {
			m_color[chId]= generateColor();
			return m_color[chId];
		}
		QColor color;
		bool colorExists = true;
		while(colorExists){
			color = generateColor();
			for (auto it = m_color.begin(); it != m_color.end(); it++) {
				if (it.value() != color) {
					colorExists = false;
					break;
				}
			}
		}
		m_color[chId]=color;
		return color;
	}
}

adiscope::gui::GenericMenu* DataLogger::generateMenu(QString title, QColor* color)
{
	adiscope::gui::GenericMenu *menu = new adiscope::gui::GenericMenu(this);
	menu->initInteractiveMenu();
	menu->setMenuHeader(title,color,false);

	auto *showAllSection = new scopy::gui::SubsectionSeparator("Show all", false, this);

	QWidget *showAllWidget = new QWidget(this);
	auto *showAllLayout = new QHBoxLayout(showAllWidget);
	showAllSWitch = new CustomSwitch(showAllWidget);
	showAllSWitch->setChecked(true);

	showAllLayout->addWidget(new QLabel("Show all",showAllWidget));
	showAllLayout->addWidget(showAllSWitch);

	showAllSection->setContent(showAllWidget);

	connect(showAllSWitch, &CustomSwitch::toggled, this, [=](bool toggled){
		Q_EMIT DataLogger::toggleAll(toggled);
	});

	auto *precisionSection = new scopy::gui::SubsectionSeparator("Precision", false,this);

	QWidget *precisionWidget = new QWidget(this);
	auto *precisionLayout = new QHBoxLayout(precisionWidget);
	precisionLayout->setAlignment(precisionWidget,Qt::AlignLeft);
	auto *precisionBtn = new QPushButton("Set",precisionWidget);
	precisionBtn->setMinimumSize(50,30);
	precisionBtn->setProperty("blue_button",true);

	precisionValue = new QLineEdit(precisionWidget);
	precisionValue->setText("3");
	precisionLayout->addWidget(precisionBtn);
	precisionLayout->addItem(new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Fixed));
	precisionLayout->addWidget(precisionValue);

	connect(precisionBtn, &QPushButton::clicked,this, [=](){
		auto value = precisionValue->text().toInt();
		if (value < 0) {
			precisionValue->setText("0");
		}
		if (value > 10) {
			precisionValue->setText("9");
		}

		Q_EMIT precisionChanged(precisionValue->text().toInt());
	});

	precisionSection->setContent(precisionWidget);

	auto *recordingIntervalSection = new scopy::gui::SubsectionSeparator("Recording interval", false, this);
	auto recordingIntervalWidget = new QWidget(this);
	auto *recordingIntevlaLayout = new QVBoxLayout(recordingIntervalWidget);

	recording_timer = new PositionSpinButton({
												 {"s", 1},
												 {"min", 60},
												 {"h", 3600}
											 }, tr("Timer"), 0, 3600,
											 true, false, this);

	recording_timer->setValue(1);
	recordingIntevlaLayout->addWidget(recording_timer);
	connect(recording_timer, &PositionSpinButton::valueChanged, this, [=](){
		double interval = recording_timer->value() * 1000;
		if (interval < 100) {
			interval = 100;
			recording_timer->setValue(interval);

		}
		Q_EMIT recordingIntervalChanged(interval);
	});

	recordingIntervalSection->setContent(recordingIntervalWidget);

	auto dataLoggingSection = new scopy::gui::SubsectionSeparator("Data Logging",true,this);

	dataLogger = new DataLoggerController(true,true,false);
	dataLogger->setWarningMessage("* While data logging you won't be able to add/remove channels");
	dataLoggingSection->setContent(dataLogger->getWidget());

	connect(this, &DataLogger::updateValue, this , [=](QString name, QString value){
		dataLogger->receiveValue(name,value);
	});

	connect(dataLogger, &DataLoggerController::isDataLogging, this, [=](bool toggled){

		if (m_toolView->getRunBtn()->isChecked()) {
			showAllSWitch->setEnabled(!toggled);
		}

		if (toggled) {
			if (!m_activeChannels.empty()) {
				for (auto ch : m_activeChannels.keys()) {
					QString name = QString::fromStdString(m_activeChannels[ch].first.dmm->getName() + ":" + m_activeChannels[ch].first.dmmId);
					dataLogger->createChannel(name, adiscope::CHANNEL_DATA_TYPE::DOUBLE);
				}
			}
		}
		Q_EMIT disableActivateChannel(toggled);
	});

	menu->insertSection(showAllSection);
	menu->insertSection(precisionSection);
	menu->insertSection(recordingIntervalSection);
	menu->insertSection(dataLoggingSection);

	return menu;
}


DataLogger::~DataLogger()
{
	if (saveOnExit) {
		api->save(*settings);
	}
	if (m_timer) { delete m_timer; }
	if (m_toolView) { delete m_toolView; }
	delete api;
}
