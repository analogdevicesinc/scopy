#include "datalogger.hpp"
#include "dataloggerplugin.hpp"
#include "src/gui/dataloggercontroller.hpp"
//#include "gui/generic_menu.hpp"
//#include "gui/customSwitch.hpp"
//#include "gui/detachedwindowsmanager.h"
//#include "gui/dynamicWidget.hpp"
#include <QtConcurrent>
#include <QFuture>
#include <tool_view_builder.hpp>

#include <libm2k/contextbuilder.hpp>
#include <src/datalogger_api.h>
#include <src/dataloggergenericmenu.hpp>

#include <libm2k/contextbuilder.hpp>

#include <src/gui/channelmonitorcomponent.hpp>
#include <src/gui/customcolqgridlayout.hpp>
//#include <libm2k/contextbuilder.hpp>

//#include "gui/dataloggercontroller.hpp"
//#include "gui/customSwitch.hpp"
//#include "datalogger_api.h"
//#include "gui/channelmonitorcomponent.hpp"
//#include "dataloggergenericmenu.hpp"
//#include "gui/customcolqgridlayout.hpp"

//using namespace scopy;

using gui::ToolViewBuilder;

DataLogger::DataLogger(struct iio_context *ctx, QWidget *parent):
	QWidget(parent),
//	Tool(ctx, toolMenuItem, new DataLogger_API(this), "DataLogger", parent),
	m_timer(new QTimer(this)),
	m_elapsed(new QElapsedTimer()),
	readerThread(new DataLoggerReaderThread())
//	m_context(libm2k::context::contextOpen(ctx, ""))
{
	//	run_button = nullptr;

	m_context = libm2k::context::contextOpen(ctx, "");
	m_colors = {QColor("#ff7200"),    QColor("#9013fe"),    QColor(Qt::green),
		    QColor(Qt::cyan),     QColor(Qt::magenta),  QColor(Qt::yellow),
		    QColor(Qt::gray),     QColor(Qt::darkRed),  QColor(Qt::darkGreen),
		    QColor(Qt::darkBlue), QColor(Qt::darkGray), QColor(Qt::black)};

	gui::ToolViewRecipe recipe;
	recipe.helpBtnUrl = "";
	recipe.hasRunBtn = true;
	recipe.hasSingleBtn = true;
	recipe.hasPairSettingsBtn = true;
	recipe.hasPrintBtn = false;
	recipe.hasChannels = true;
	recipe.channelsPosition = gui::ChannelsPositionEnum::VERTICAL;

	m_monitorChannelManager = new gui::ChannelManager(recipe.channelsPosition);
	m_monitorChannelManager->setChannelIdVisible(false);
	m_monitorChannelManager->setToolStatus("Stopped");

	m_toolView = ToolViewBuilder(recipe, m_monitorChannelManager, parent).build();

	m_timer->setSingleShot(true);
	// on timeout read on thread
	connect(m_timer, &QTimer::timeout, this, [=]() {
		if (!m_elapsed->isValid()) {
			m_elapsed->start();
		}
		readerThread->start();
	});

	connect(readerThread, &DataLoggerReaderThread::finished, this, [=]() {
		// after read is done check if another read is needed
		if (m_toolView->getRunBtn()->isChecked()) {
			int newInterval = VALUE_READING_TIME_INTERVAL - m_elapsed->elapsed();
			m_elapsed->invalidate();
			if (newInterval < 0) {
				newInterval = 0;
			}
			m_timer->start(newInterval);
		}

		if (m_toolView->getSingleBtn()->isChecked()) {
			m_toolView->getSingleBtn()->setChecked(false);
		}
	});

	m_generalSettingsMenu =
			generateMenu("General settings", new QColor("#4a64ff"));
	m_toolView->setGeneralSettingsMenu(m_generalSettingsMenu, true);

	m_dmmList = getDmmList(m_context);

	connect(this, &DataLogger::recordingIntervalChanged, this,
		[=](double interval) {
		VALUE_READING_TIME_INTERVAL = interval;
		recording_timer->setValue(interval / 1000);
	});

	connect(m_toolView->getRunBtn(), &QPushButton::toggled, this,
		[=](bool toggled) {
		dataLoggerController->setIsRunningOn(
					m_toolView->getRunBtn()->isChecked());

		// update status and if needed start data logging
		if (!m_toolView->getRunBtn()->isChecked()) {
			m_monitorChannelManager->setToolStatus("Stopped");
			showAllSWitch->setEnabled(true);
			readerThread->setDataLoggerStatus(false);
			dataLoggerController->stopLogger();
		} else {
			if (dataLoggerController->isDataLoggerOn()) {
				m_monitorChannelManager->setToolStatus("Data Logging");
				readerThread->setDataLoggerStatus(true);
			} else {
				m_monitorChannelManager->setToolStatus("Running");
			}
		}

		if (toggled) {
			m_timer->start(VALUE_READING_TIME_INTERVAL);
		} else {
			// if thread is running after current action on thread is done
			// stop thread
			if (readerThread->isRunning()) {
				readerThread->quit();
				readerThread->wait();
			}
		}
	});

	connect(m_toolView->getSingleBtn(), &QPushButton::toggled, this,
		[=](bool toggled) {
		if (toggled) {
			if (m_toolView->getRunBtn()->isChecked()) {
				m_toolView->getRunBtn()->toggled(false);
			}

			m_timer->start(0);
		}
	});

	m_customColGrid = new CustomColQGridLayout(100,this);
	m_toolView->addFixedCentralWidget(m_customColGrid,0,0,0,0);
//	setCentralWidget(getToolView());
	this->setLayout(new QVBoxLayout());
	this->layout()->addWidget(m_toolView);

	initMonitorToolView();

//	api->setObjectName(QString::fromStdString(Filter::tool_name(TOOL_DATALOGGER)));
//	api->load(*settings);
//	api->js_register(engine);


	qInfo(CAT_DATALOGGER_TOOL) << "Initialized";
//	Q_EMIT DataLogger::toggleAll(true);
}

void DataLogger::initMonitorToolView()
{
	int chId = 1;

	bool first = true;
	setUpdatesEnabled(false);

	for (libm2k::analog::DMM* dmm : m_dmmList) {

		auto dmmName = dmm->getName();
		std::vector<ChannelWidget*> channelList;

		scopy::gui::DataLoggerGenericMenu *menu = new scopy::gui::DataLoggerGenericMenu(this);
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
			scopy::gui::DataLoggerGenericMenu *channelMenu = new scopy::gui::DataLoggerGenericMenu(this);
			channelMenu->init(QString::fromStdString(dmmName + ": " + channel.id),new QColor(channelColor));

			ChannelWidget *ch_widget = m_toolView->buildNewChannel(m_monitorChannelManager, channelMenu , false, chId, false, false,
								    channelColor, QString::fromStdString(channel.id), QString::fromStdString(channel.id));

			channelList.push_back(ch_widget);
			ch_widget->enableButton()->setChecked(false);

			scopy::ChannelMonitorComponent* monitor = new scopy::ChannelMonitorComponent();
			monitor->setID(chId);
			monitor->init(0,QString::fromStdString(channel.unit_name),QString::fromStdString(channel.unit_symbol),
				      QString::fromStdString(dmmName + ": " + channel.id),channelColor );

			monitor->setChannelId(channel.id);
			monitor->setRecordingInterval(VALUE_READING_TIME_INTERVAL/1000);
			monitor->setHistoryDuration(10);

			connect(readerThread, &DataLoggerReaderThread::updateChannelData, monitor, [=](int chId, double value,QString nameOfUnitOfMeasure,QString symbolOfUnitOfMeasure){
				if (chId == monitor->getID()) {
					monitor->updateValue(value, nameOfUnitOfMeasure, symbolOfUnitOfMeasure);
				}
			});

			connect(monitor, &scopy::ChannelMonitorComponent::contentChanged, m_customColGrid, [=](){
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
				if(m_toolView->getRunBtn()->isChecked() && toggled)
					ch_widget->enableButton()->setDisabled(true);
				else
					ch_widget->enableButton()->setDisabled(false);

			});

			int widgetId = m_customColGrid->addQWidgetToList(monitor);

			readerThread->addChannel(chId, channel.id,dmm);

			// logic for enable/disable channels
			connect(ch_widget, &ChannelWidget::enabled,this, [=](bool enabled){

				readerThread->channelToggled(chId,enabled);
				if (enabled) {
					m_customColGrid->addWidget(widgetId);
					m_activeChannels[chId] = monitor;

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

void DataLogger::createConnections(scopy::gui::DataLoggerGenericMenu* mainMenu,scopy::gui::DataLoggerGenericMenu* menu,scopy::ChannelMonitorComponent* monitor)
{
	connect(menu,&scopy::gui::DataLoggerGenericMenu::togglePeakHolder, monitor, &scopy::ChannelMonitorComponent::displayPeakHold);
	connect(mainMenu,&scopy::gui::DataLoggerGenericMenu::togglePeakHolder, menu, &scopy::gui::DataLoggerGenericMenu::peakHolderToggle);

	connect(menu, &scopy::gui::DataLoggerGenericMenu::resetPeakHolder, monitor, &scopy::ChannelMonitorComponent::resetPeakHolder);
	connect(mainMenu,&scopy::gui::DataLoggerGenericMenu::resetPeakHolder, menu, &scopy::gui::DataLoggerGenericMenu::peakHolderResetClicked);

	connect(menu, &scopy::gui::DataLoggerGenericMenu::toggleScale, monitor, &scopy::ChannelMonitorComponent::displayScale);
	connect(mainMenu,&scopy::gui::DataLoggerGenericMenu::toggleScale, menu, &scopy::gui::DataLoggerGenericMenu::scaleToggle);

	connect(menu, &scopy::gui::DataLoggerGenericMenu::toggleHistory, monitor, &scopy::ChannelMonitorComponent::displayHistory);
	connect(mainMenu,&scopy::gui::DataLoggerGenericMenu::toggleHistory, menu, &scopy::gui::DataLoggerGenericMenu::historyToggle);

	connect(menu, &scopy::gui::DataLoggerGenericMenu::changeHistorySize, monitor, &scopy::ChannelMonitorComponent::setHistoryDuration);
	connect(mainMenu,&scopy::gui::DataLoggerGenericMenu::historySizeIndexChanged, menu, &scopy::gui::DataLoggerGenericMenu::historySizeChanged);

	connect(menu, &scopy::gui::DataLoggerGenericMenu::lineStyleChanged, monitor, &scopy::ChannelMonitorComponent::setLineStyle);
	connect(mainMenu,&scopy::gui::DataLoggerGenericMenu::lineStyleIndexChanged, menu, &scopy::gui::DataLoggerGenericMenu::changeLineStyle);

	connect(menu, &scopy::gui::DataLoggerGenericMenu::monitorColorChanged, monitor, &ChannelMonitorComponent::setMonitorColor);
}

scopy::gui::ToolView* DataLogger::getToolView()
{
	return m_toolView;
}
void DataLogger::setNativeDialogs(bool nativeDialogs)
{
//	Tool::setNativeDialogs(nativeDialogs);
	dataLoggerController->setUseNativeDialog(nativeDialogs);
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
	return context->getAllDmm();
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

scopy::gui::GenericMenu* DataLogger::generateMenu(QString title, QColor* color)
{
	scopy::gui::GenericMenu *menu = new scopy::gui::GenericMenu(this);
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
	precisionValue->setValidator( new QIntValidator(this) );
	precisionValue->setText("3");
	precisionLayout->addWidget(precisionBtn);
	precisionLayout->addItem(new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Fixed));
	precisionLayout->addWidget(precisionValue);

	connect(precisionBtn, &QPushButton::clicked,this, [=](){
		auto value = precisionValue->text().toInt();
		if (value < 0) {
			precisionValue->setText("0");
		}
		if (value >= 10) {
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

	dataLoggerController = new DataLoggerController(true,true,false);
	dataLoggerController->setWarningMessage("* While data logging you won't be able to add/remove channels");
	dataLoggingSection->setContent(dataLoggerController->getWidget());

	connect(readerThread, &DataLoggerReaderThread::updateDataLoggerValue, this , [=](QString name, QString value){
		dataLoggerController->receiveValue(name,value);
	});

	connect(dataLoggerController, &DataLoggerController::isDataLogging, this, [=](bool toggled){

		if (m_toolView->getRunBtn()->isChecked()) {
			showAllSWitch->setEnabled(!toggled);
		}

		if (toggled) {
			if (!m_activeChannels.empty()) {
				for (int ch : m_activeChannels.keys()) {
					QString name = m_activeChannels[ch]->getTitle();
					dataLoggerController->createChannel(name, scopy::CHANNEL_DATA_TYPE::DOUBLE);
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

	if (m_toolView->getRunBtn()->isChecked()) {
		m_toolView->getRunBtn()->setChecked(false);
	}
	if (readerThread->isRunning()) {
		readerThread->quit();
		readerThread->wait();
	}
	if (readerThread) {
		delete readerThread;
	}
//	if (saveOnExit) {
//		api->save(*settings);
//	}
	if (m_timer) {
		delete m_timer;
	}
	if (m_elapsed) {
		delete m_elapsed;
	}
	if (m_toolView) {
//		delete m_toolView;
	}
//	delete api;
}
