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

#include "ad74413r/ad74413r.h"

#include "ad74413r/buffermenumodel.h"
#include "ad74413r/buffermenuview.h"
#include "swiot_logging_categories.h"

#include <iio.h>

#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuheader.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/stylehelper.h>
#include <gui/widgets/verticalchannelmanager.h>

#include <iioutil/connectionprovider.h>

using namespace scopy::swiotrefactor;
using namespace scopy;

Ad74413r::Ad74413r(QString uri, ToolMenuEntry *tme, QWidget *parent)
	: QWidget(parent)
	, m_uri(uri)
	, m_tme(tme)
	, m_statusLabel(new QLabel(this))
	, m_swiotAdLogic(nullptr)
	, m_widget(this)
	, m_readerThread(nullptr)
	, m_statusContainer(new QWidget(this))
	, m_currentChannelSelected(0)
{
	this->setupToolTemplate();
	m_conn = ConnectionProvider::open(m_uri);
	connect(m_conn, &Connection::aboutToBeDestroyed, this, &Ad74413r::handleConnectionDestroyed);
	m_ctx = m_conn->context();
	m_cmdQueue = m_conn->commandQueue();

	createDevicesMap(m_ctx);
	if(m_iioDevicesMap.contains(AD_NAME) && m_iioDevicesMap.contains(SWIOT_DEVICE_NAME)) {
		char mode[64];
		ssize_t result = iio_device_attr_read(m_iioDevicesMap[SWIOT_DEVICE_NAME], "mode", mode, 64);
		if((result >= 0) && (strcmp(mode, "runtime") == 0)) {
			m_enabledChannels = std::vector<bool>(MAX_CURVES_NUMBER, false);

			m_swiotAdLogic = new BufferLogic(m_iioDevicesMap, m_cmdQueue);
			m_readerThread = new ReaderThread(true, m_cmdQueue, this);
			m_readerThread->addBufferedDevice(m_iioDevicesMap[AD_NAME]);
			m_plotHandler = new BufferPlotHandler(this, m_swiotAdLogic->getPlotChnlsNo());

			setupConnections();
			m_swiotAdLogic->initAd74413rChnlsFunctions();
		}
	}
	this->initTutorialProperties();
}

Ad74413r::~Ad74413r()
{
	if(m_readerThread) {
		m_readerThread->forcedStop();
		delete m_readerThread;
	}
	if(m_controllers.size() > 0) {
		m_controllers.clear();
	}
	ConnectionProvider::close(m_uri);
}

void Ad74413r::handleConnectionDestroyed()
{
	qDebug(CAT_SWIOT_AD74413R) << "Ad74413R connection destroyed slot";
	m_ctx = nullptr;
	m_cmdQueue = nullptr;
	m_conn = nullptr;
}

void Ad74413r::setupConnections()
{
	connect(m_conn, &Connection::aboutToBeDestroyed, m_readerThread, &ReaderThread::handleConnectionDestroyed);
	connect(m_backBtn, &QPushButton::pressed, this, &Ad74413r::onBackBtnPressed);
	connect(m_runBtn, &QPushButton::toggled, this, &Ad74413r::onRunBtnPressed);
	connect(m_swiotAdLogic, &BufferLogic::chnlsChanged, m_readerThread, &ReaderThread::onChnlsChange);
	connect(m_swiotAdLogic, &BufferLogic::samplingFrequencyComputed, m_plotHandler,
		&BufferPlotHandler::onSamplingFrequencyComputed);
	connect(m_swiotAdLogic, &BufferLogic::samplingFrequencyComputed, this, [=, this](double frequency) {
		m_frequency = frequency;
		refreshSampleRate();
	});
	connect(m_swiotAdLogic, &BufferLogic::samplingFrequencyComputed, m_readerThread,
		&ReaderThread::onSamplingFrequencyComputed);
	connect(this, &Ad74413r::activateRunBtns, this, &Ad74413r::onActivateRunBtns);

	connect(m_readerThread, SIGNAL(bufferRefilled(QMap, int)), m_plotHandler,
		SLOT(onBufferRefilled(QVector<QVector<double>>, int)), Qt::QueuedConnection);
	connect(m_readerThread, &ReaderThread::readerThreadFinished, this, &Ad74413r::onReaderThreadFinished,
		Qt::QueuedConnection);

	connect(m_singleBtn, &QPushButton::toggled, this, &Ad74413r::onSingleBtnPressed);
	connect(m_plotHandler, &BufferPlotHandler::singleCaptureFinished, this, &Ad74413r::onSingleCaptureFinished);
	connect(this, &Ad74413r::timespanChanged, m_plotHandler, &BufferPlotHandler::onTimespanChanged);
	connect(this, &Ad74413r::timespanChanged, this, &Ad74413r::refreshSampleRate);

	connect(m_swiotAdLogic, &BufferLogic::channelFunctionDetermined, this, &Ad74413r::setupChannel);

	connect(m_tme, &ToolMenuEntry::runToggled, m_runBtn, &QPushButton::setChecked);
	connect(m_plotHandler, &BufferPlotHandler::bufferDataReady, this, &Ad74413r::onBufferRefilled);

	//	connect(m_toolView->getPrintBtn(), &QPushButton::clicked, m_plotHandler,
	//&BufferPlotHandler::onPrintBtnClicked);
}

void Ad74413r::onChannelWidgetEnabled(int chnlIdx, bool en)
{
	if(en) {
		m_enabledChannels[chnlIdx] = true;
		verifyChnlsChanges();
	} else {
		m_enabledChannels[chnlIdx] = false;
		verifyChnlsChanges();
	}

	if(std::find(m_enabledChannels.begin(), m_enabledChannels.end(), true) == m_enabledChannels.end()) {
		Q_EMIT activateRunBtns(false);

	} else {
		Q_EMIT activateRunBtns(true);
	}

	Q_EMIT channelWidgetEnabled(chnlIdx, en);
}

void Ad74413r::onActivateRunBtns(bool enable)
{
	if(!enable) {
		if(m_runBtn->isChecked()) {
			m_runBtn->setChecked(false);
		}
		if(m_singleBtn->isChecked()) {
			m_singleBtn->setChecked(false);
		}
		m_runBtn->setEnabled(false);
		m_singleBtn->setEnabled(false);
		m_tme->setRunEnabled(false);
	} else {
		m_runBtn->setEnabled(true);
		m_singleBtn->setEnabled(true);
		m_tme->setRunEnabled(true);
	}
}

void Ad74413r::onRunBtnPressed(bool toggled)
{
	Q_EMIT thresholdControlEnable(!toggled);
	if(toggled) {
		m_singleBtn->setChecked(false);
		m_singleBtn->setEnabled(false);
		verifyChnlsChanges();
		if(!m_readerThread->isRunning()) {
			m_plotHandler->setSingleCapture(false);
			m_plotHandler->resetPlotParameters();
			m_readerThread->startCapture();
		}
		if(!m_tme->running()) {
			m_tme->setRunning(toggled);
		}
	} else {
		m_singleBtn->setEnabled(true);
		m_readerThread->requestStop();
		if(m_tme->running()) {
			m_tme->setRunning(toggled);
		}
	}
}

void Ad74413r::onSingleBtnPressed(bool toggled)
{
	bool runBtnChecked = m_runBtn->isChecked();
	if(toggled) {
		Q_EMIT activateExportButton();
		Q_EMIT thresholdControlEnable(toggled);
		verifyChnlsChanges();
		if(runBtnChecked) {
			m_runBtn->setChecked(false);
		}
		m_plotHandler->setSingleCapture(true);
		if(!m_readerThread->isRunning()) {
			m_plotHandler->resetPlotParameters();
			int bufNumber = m_plotHandler->getRequiredBuffersNumber();
			m_readerThread->startCapture(bufNumber);
		}
		m_singleBtn->setEnabled(false);
	}
}

void Ad74413r::onSingleCaptureFinished()
{
	bool runBtnChecked = m_runBtn->isChecked();
	if(!runBtnChecked) {
		if(m_tme->running()) {
			m_tme->setRunning(false);
		}
		Q_EMIT thresholdControlEnable(true);
		m_runBtn->setEnabled(true);
	}
	m_readerThread->requestStop();
	m_runBtn->setChecked(false);
}

void Ad74413r::verifyChnlsChanges()
{
	bool changes = m_swiotAdLogic->verifyChannelsEnabledChanges(m_enabledChannels);
	if(changes) {
		m_readerThread->requestStop();
		m_swiotAdLogic->applyChannelsEnabledChanges(m_enabledChannels);
	}
}

void Ad74413r::createDevicesMap(iio_context *ctx)
{
	int devicesCount = iio_context_get_devices_count(ctx);
	for(int i = 0; i < devicesCount; i++) {
		struct iio_device *iioDev = iio_context_get_device(ctx, i);
		if(iioDev) {
			QString deviceName = QString(iio_device_get_name(iioDev));
			if((deviceName.compare(AD_NAME) && deviceName.compare(SWIOT_DEVICE_NAME)) == 0) {
				m_iioDevicesMap[deviceName] = iioDev;
			}
		}
	}
}

void Ad74413r::onReaderThreadFinished()
{
	bool singleCaptureOn = m_plotHandler->singleCapture();
	if(singleCaptureOn) {
		m_plotHandler->setSingleCapture(false);
	}
	int nbRequiredBuffers = 0;
	if(m_runBtn->isChecked() || !m_singleBtn->isEnabled()) {
		m_plotHandler->resetPlotParameters();
		if(m_singleBtn->isChecked()) {
			m_plotHandler->setSingleCapture(true);
			nbRequiredBuffers = m_plotHandler->getRequiredBuffersNumber();
		}
		m_readerThread->startCapture(nbRequiredBuffers);
	}
}

void Ad74413r::externalPowerSupply(bool ps)
{
	if(ps) {
		m_statusContainer->hide();
	} else {
		m_statusContainer->show();
		m_statusLabel->show();
	}
}

void Ad74413r::onSamplingFrequencyUpdated(int channelId, int value)
{
	m_readerThread->requestStop();
	m_swiotAdLogic->applySamplingFrequencyChanges(channelId, value);
}

void Ad74413r::onDiagnosticFunctionUpdated()
{
	m_readerThread->requestStop();
	m_swiotAdLogic->applyChannelsEnabledChanges(m_enabledChannels);
}

void Ad74413r::onBackBtnPressed()
{
	bool runBtnChecked = m_runBtn->isChecked();
	bool singleBtnChecked = m_singleBtn->isChecked();

	if(runBtnChecked) {
		m_runBtn->setChecked(false);
	}
	if(singleBtnChecked) {
		m_singleBtn->setChecked(false);
	}
	Q_EMIT backBtnPressed();
}

void Ad74413r::initTutorialProperties()
{
	// initialize components that might be used for the AD74413R tutorial
	m_plot->setProperty("tutorial_name", "AD74413R_PLOT");
	m_singleBtn->setProperty("tutorial_name", "SINGLE_BUTTON");
	m_runBtn->setProperty("tutorial_name", "RUN_BUTTON");
	//	m_toolView->getGeneralSettingsBtn()->setProperty("tutorial_name", "AD74413R_SETTINGS");
	m_backBtn->setProperty("tutorial_name", "CONFIG_BUTTON");
}

void Ad74413r::initPlotData()
{
	// to investigate
	m_xTime.clear();
	for(int i = 0; i < m_sampleRate; i++) {
		m_xTime.push_back(-(i / m_sampleRate));
	}
	for(int i = 0; i < MAX_CURVES_NUMBER; i++) {
		m_yValues[i] = std::vector<double>(MAX_SAMPLE_RATE, 0);
	}
}

void Ad74413r::initPlot()
{
	m_plot->xAxis()->setInterval(-1, 0);
	m_plot->xAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	m_plot->xAxis()->scaleDraw()->setFloatPrecision(2);
	m_plot->leftHandlesArea()->setVisible(true);
	m_plot->replot();
}

PlotAxis *Ad74413r::createYChnlAxis(QPen pen, QString unitType, int yMin, int yMax)
{
	PlotAxis *chYAxis = new PlotAxis(QwtAxis::YRight, m_plot, pen);
	chYAxis->setVisible(false);
	chYAxis->setInterval(yMin, yMax);
	chYAxis->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	chYAxis->scaleDraw()->setFloatPrecision(2);
	chYAxis->scaleDraw()->setUnitType(unitType);
	return chYAxis;
}

void Ad74413r::setupDeviceBtn()
{
	VerticalChannelManager *vcm = new VerticalChannelManager(this);
	m_tool->leftStack()->add("vcm", vcm);
	// for devBtn I have to make a connection with the general settings menu
	m_devBtn = new CollapsableMenuControlButton(this);
	m_devBtn->getControlBtn()->setName("AD74413R");
	m_devBtn->getControlBtn()->setCheckable(false);
	m_devBtn->getControlBtn()->button()->setVisible(false);
	vcm->add(m_devBtn);
	m_chnlsBtnGroup = new QButtonGroup(this);
}

void Ad74413r::setupChannelsMenuBtn(MenuControlButton *btn, QString name)
{
	btn->setName(name);
	btn->setOpenMenuChecksThis(true);
	btn->setDoubleClickToOpenMenu(true);
	btn->checkBox()->setVisible(false);
	btn->setChecked(true);
}

void Ad74413r::setupChannel(int chnlIdx, QString function)
{
	if(function.compare("no_config") != 0) {
		QString chnlId(function + " " + QString::number(chnlIdx + 1));
		QPen chPen = QPen(QColor(StyleHelper::getColor("CH" + QString::number(chnlIdx))), 1);

		QString unit = m_swiotAdLogic->getPlotChnlUnitOfMeasure(chnlIdx);
		auto yRange = m_swiotAdLogic->getPlotChnlRangeValues(chnlIdx);

		PlotAxis *chYAxis = createYChnlAxis(chPen, unit, yRange.first, yRange.second);
		PlotChannel *plotCh = new PlotChannel(chnlId, chPen, m_plot, m_plot->xAxis(), chYAxis, this);
		PlotAxisHandle *chHandle = new PlotAxisHandle(chPen, chYAxis, m_plot, QwtAxis::YLeft, this);
		plotCh->setHandle(chHandle);
		m_plot->addPlotAxisHandle(chHandle);
		m_plot->addPlotChannel(plotCh);

		QMap<QString, iio_channel *> chnlsMap = m_swiotAdLogic->getIioChnl(chnlIdx);
		iio_channel *menuChnl = nullptr;
		if(chnlsMap.size() > 1) {
			menuChnl = chnlsMap["output"];
		} else {
			menuChnl = chnlsMap.values()[0];
		}
		BufferMenuView *menu = new BufferMenuView(menuChnl, m_conn, this);
		menu->init(chnlId, function, chPen, unit, yRange.first, yRange.second);
		m_channelStack->add(chnlId, menu);

		BufferMenuModel *swiotModel = new BufferMenuModel(chnlsMap, m_cmdQueue);
		BufferMenuController *controller = new BufferMenuController(menu, swiotModel, chnlIdx);

		controller->createConnections();
		MenuControlButton *btn = new MenuControlButton(m_devBtn);
		m_devBtn->add(btn);
		m_chnlsBtnGroup->addButton(btn);
		setupChannelMenuControlButtonHelper(btn, plotCh, chnlId);
		plotCh->setEnabled(false);
		plotCh->curve()->setRawSamples(m_xTime.data(), m_yValues[chnlIdx].data(), m_xTime.size());

		chnlIdx++;
		connect(btn, &MenuControlButton::toggled, this, [=, this](bool en) {
			if(en) {
				m_plot->selectChannel(plotCh);
				m_channelStack->show(chnlId);
			}
		});

		connect(btn->checkBox(), &MenuControlButton::toggled, this, [=, this](bool en) {
			plotCh->setEnabled(en);
			onChannelWidgetEnabled(chnlIdx, en);
		});

		connect(menu, &BufferMenuView::setYMin, chYAxis, &PlotAxis::setMin);
		connect(chYAxis, &PlotAxis::minChanged, this, [=, this]() { Q_EMIT menu->minChanged(chYAxis->min()); });
		connect(menu, &BufferMenuView::setYMax, chYAxis, &PlotAxis::setMax);
		connect(chYAxis, &PlotAxis::maxChanged, this, [=, this]() { Q_EMIT menu->maxChanged(chYAxis->max()); });

		// old connections
		connect(controller, &BufferMenuController::samplingFrequencyUpdated, this,
			&Ad74413r::onSamplingFrequencyUpdated);
		connect(controller, &BufferMenuController::diagnosticFunctionUpdated, this,
			&Ad74413r::onDiagnosticFunctionUpdated);
		connect(controller, SIGNAL(broadcastThresholdReadForward(QString)), this,
			SIGNAL(broadcastReadThreshold(QString)));
		connect(this, SIGNAL(broadcastReadThreshold(QString)), controller,
			SIGNAL(broadcastThresholdReadBackward(QString)));
		connect(this, &Ad74413r::thresholdControlEnable, controller,
			&BufferMenuController::thresholdControlEnable);
	}
	m_currentChannelSelected++;
	if(m_currentChannelSelected == 4) {
		m_swiotAdLogic->initDiagnosticChannels();
	}
}

void Ad74413r::onBufferRefilled(QMap<int, std::vector<double>> bufferData)
{
	const QList<int> chList = m_yValues.keys();
	for(const int &ch : chList) {
		m_yValues[ch].clear();
		m_yValues[ch] = bufferData[ch];
	}
	m_plot->replot();
}

void Ad74413r::refreshSampleRate() { m_sampleRate = m_frequency * m_timespanSpin->value(); }

void Ad74413r::setupToolTemplate()
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout *layout = new QHBoxLayout(this);
	setLayout(layout);
	StyleHelper::GetInstance()->initColorMap();

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->centralContainer()->setVisible(true);
	m_tool->bottomContainer()->setVisible(true);
	m_tool->leftContainer()->setVisible(true);
	m_tool->rightContainer()->setVisible(true);

	m_tool->setLeftContainerWidth(200);
	m_tool->setRightContainerWidth(300);

	layout->addWidget(m_tool);

	m_plot = new PlotWidget(this);
	initPlot();
	setupDeviceBtn();
	m_tool->addWidgetToCentralContainerHelper(m_plot);

	m_settingsBtn = new GearBtn(this);
	m_runBtn = new RunBtn(this);
	m_runBtn->setChecked(false);
	m_singleBtn = new SingleShotBtn(this);
	m_singleBtn->setChecked(false);
	m_printBtn = new PrintBtn(this);
	m_backBtn = createBackBtn();

	MenuControlButton *chnlsMenuBtn = new MenuControlButton(this);
	setupChannelsMenuBtn(chnlsMenuBtn, "Channels");
	connect(chnlsMenuBtn->button(), &QAbstractButton::toggled, this, [=](bool b) {
		if(b)
			m_tool->requestMenu(channelsMenuId);
	});
	connect(chnlsMenuBtn, &QPushButton::toggled, dynamic_cast<MenuHAnim *>(m_tool->leftContainer()),
		&MenuHAnim::toggleMenu);

	QPushButton *openLastMenuBtn =
		new OpenLastMenuBtn(dynamic_cast<MenuHAnim *>(m_tool->rightContainer()), true, this);
	m_rightMenuBtnGrp = dynamic_cast<OpenLastMenuBtn *>(openLastMenuBtn)->getButtonGroup();
	m_rightMenuBtnGrp->addButton(chnlsMenuBtn->button());

	m_channelStack = new MapStackedWidget(this);
	m_tool->rightStack()->add(channelsMenuId, m_channelStack);

	QString settingsMenuId = "PlotSettings";
	m_tool->rightStack()->add(settingsMenuId, createSettingsMenu(this));
	connect(m_settingsBtn, &QPushButton::toggled, this, [=](bool b) {
		if(b)
			m_tool->requestMenu(settingsMenuId);
	});
	m_rightMenuBtnGrp->addButton(m_settingsBtn);

	m_tool->addWidgetToBottomContainerHelper(chnlsMenuBtn, TTA_LEFT);

	m_tool->addWidgetToTopContainerMenuControlHelper(openLastMenuBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerMenuControlHelper(m_settingsBtn, TTA_LEFT);

	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(m_singleBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(m_printBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_backBtn, TTA_LEFT);
}

void Ad74413r::setupChannelMenuControlButtonHelper(MenuControlButton *btn, PlotChannel *ch, QString chnlId)
{
	btn->setName(chnlId);
	btn->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	btn->setOpenMenuChecksThis(true);
	btn->setDoubleClickToOpenMenu(true);
	btn->setColor(ch->curve()->pen().color());
	btn->button()->setVisible(false);
	btn->setCheckable(true);
	connect(btn->checkBox(), &QCheckBox::toggled, this, [=](bool b) {
		if(b)
			ch->enable();
		else
			ch->disable();
	});
	btn->checkBox()->setChecked(false);
}

QPushButton *Ad74413r::createBackBtn()
{
	QPushButton *backBtn = new QPushButton();
	StyleHelper::BlueGrayButton(backBtn, "back_btn");
	backBtn->setFixedWidth(128);
	backBtn->setCheckable(false);
	backBtn->setText("Back");
	return backBtn;
}

void Ad74413r::showPlotLabels(bool b)
{
	m_plot->setShowXAxisLabels(b);
	m_plot->setShowYAxisLabels(b);
	m_plot->showAxisLabels();
}

QWidget *Ad74413r::createSettingsMenu(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setMargin(0);
	layout->setSpacing(10);

	MenuHeaderWidget *header = new MenuHeaderWidget("AD74413R", QPen(StyleHelper::getColor("ScopyBlue")), widget);
	MenuSectionWidget *plotSettingsContainer = new MenuSectionWidget(widget);
	MenuCollapseSection *plotTimespanSection =
		new MenuCollapseSection("PLOT", MenuCollapseSection::MHCW_NONE, widget);
	plotTimespanSection->setLayout(new QVBoxLayout());
	plotTimespanSection->contentLayout()->setSpacing(10);
	plotTimespanSection->contentLayout()->setMargin(0);

	// timespan
	m_timespanSpin = new PositionSpinButton({{"ms", 1E-3}, {"s", 1E0}}, "Timespan", 0.1, 10, true, false);
	m_timespanSpin->setStep(0.1);
	m_timespanSpin->setValue(1);
	connect(m_timespanSpin, &PositionSpinButton::valueChanged, this,
		[=, this](double value) { m_plot->xAxis()->setMin(-value); });
	connect(m_timespanSpin, &PositionSpinButton::valueChanged, this, &Ad74413r::timespanChanged);

	// show labels
	MenuOnOffSwitch *showLabels = new MenuOnOffSwitch("PLOT LABELS", plotTimespanSection);
	showPlotLabels(true);
	showLabels->onOffswitch()->setChecked(true);
	connect(showLabels->onOffswitch(), &QAbstractButton::toggled, this, &Ad74413r::showPlotLabels);

	plotTimespanSection->contentLayout()->addWidget(m_timespanSpin);
	plotTimespanSection->contentLayout()->addWidget(showLabels);

	plotSettingsContainer->contentLayout()->addWidget(plotTimespanSection);
	layout->addWidget(header);
	layout->addWidget(plotSettingsContainer);
	layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	return widget;
}
