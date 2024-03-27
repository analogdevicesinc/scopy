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

#include "ad74413r/buffermenuview.h"
#include "swiot_logging_categories.h"

#include <iio.h>
#include <measurementlabel.h>
#include <plotinfo.h>

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
	, m_swiotAdLogic(nullptr)
	, m_readerThread(nullptr)
	, m_currentChannelSelected(0)
{
	initPlotData();
	setupToolTemplate();
	m_conn = ConnectionProvider::open(m_uri);
	if(m_conn) {
		connect(m_conn, &Connection::aboutToBeDestroyed, this, &Ad74413r::handleConnectionDestroyed);
		m_ctx = m_conn->context();
		m_cmdQueue = m_conn->commandQueue();
		createDevicesMap(m_ctx);
		init();
	}
	initTutorialProperties();
}

Ad74413r::~Ad74413r()
{
	if(m_readerThread) {
		m_readerThread->forcedStop();
		delete m_readerThread;
	}
	if(m_conn) {
		ConnectionProvider::close(m_uri);
	}
}

void Ad74413r::handleConnectionDestroyed()
{
	qDebug(CAT_SWIOT_AD74413R) << "Ad74413R connection destroyed slot";
	m_ctx = nullptr;
	m_cmdQueue = nullptr;
	m_conn = nullptr;
}

void Ad74413r::init()
{
	if(m_iioDevicesMap.contains(AD_NAME) && m_iioDevicesMap.contains(SWIOT_DEVICE_NAME)) {
		char mode[64];
		ssize_t result = iio_device_attr_read(m_iioDevicesMap[SWIOT_DEVICE_NAME], "mode", mode, 64);
		if((result >= 0) && (strcmp(mode, "runtime") == 0)) {
			m_enabledChannels = QVector<bool>(MAX_CURVES_NUMBER, false);

			m_swiotAdLogic = new BufferLogic(m_iioDevicesMap, m_cmdQueue);
			m_readerThread = new ReaderThread(true, m_cmdQueue, this);
			m_readerThread->addBufferedDevice(m_iioDevicesMap[AD_NAME]);
			m_acqHandler = new BufferAcquisitionHandler(this);
			m_rstAcqTimer = new QTimer(this);

			setupConnections();
			m_swiotAdLogic->initAd74413rChnlsFunctions();
		}
	}
}

void Ad74413r::setupConnections()
{
	connect(m_conn, &Connection::aboutToBeDestroyed, m_readerThread, &ReaderThread::handleConnectionDestroyed);
	connect(m_configBtn, &QPushButton::pressed, this, &Ad74413r::onConfigBtnPressed);
	connect(m_runBtn, &QPushButton::toggled, this, &Ad74413r::onRunBtnPressed);
	connect(m_singleBtn, &QPushButton::toggled, this, &Ad74413r::onSingleBtnPressed);

	connect(m_swiotAdLogic, &BufferLogic::chnlsChanged, m_readerThread, &ReaderThread::onChnlsChange);
	connect(m_swiotAdLogic, &BufferLogic::samplingFrequencyComputed, m_readerThread,
		&ReaderThread::onSamplingFrequencyComputed);
	connect(m_swiotAdLogic, &BufferLogic::samplingFrequencyComputed, m_acqHandler,
		&BufferAcquisitionHandler::onSamplingFrequencyComputed);

	connect(m_swiotAdLogic, &BufferLogic::samplingFrequencyComputed, this, &Ad74413r::onSamplingFreqComputed);
	connect(this, &Ad74413r::activateRunBtns, this, &Ad74413r::onActivateRunBtns);

	connect(m_swiotAdLogic, &BufferLogic::channelFunctionDetermined, this, &Ad74413r::setupChannel);
	connect(m_tme, &ToolMenuEntry::runToggled, m_runBtn, &QPushButton::setChecked);

	connect(m_readerThread, &ReaderThread::bufferRefilled, m_acqHandler,
		&BufferAcquisitionHandler::onBufferRefilled, Qt::QueuedConnection);
	connect(m_readerThread, &ReaderThread::readerThreadFinished, this, &Ad74413r::onReaderThreadFinished,
		Qt::QueuedConnection);
	connect(m_acqHandler, &BufferAcquisitionHandler::bufferDataReady, this, &Ad74413r::onBufferRefilled);
	connect(m_acqHandler, &BufferAcquisitionHandler::singleCaptureFinished, this,
		&Ad74413r::onSingleCaptureFinished, Qt::QueuedConnection);

	connect(m_timespanSpin, &PositionSpinButton::valueChanged, m_acqHandler,
		&BufferAcquisitionHandler::onTimespanChanged);

	connect(m_rstAcqTimer, &QTimer::timeout, this, [&]() {
		m_rstAcqTimer->stop();
		m_runBtn->setChecked(true);
	});
}

void Ad74413r::onChannelBtnChecked(int chnlIdx, bool en)
{
	m_enabledChannels[chnlIdx] = en;
	verifyChnlsChanges();
	bool activateBtns =
		std::find(m_enabledChannels.begin(), m_enabledChannels.end(), true) != m_enabledChannels.end();

	Q_EMIT activateRunBtns(activateBtns);
}

void Ad74413r::samplingFreqWritten(bool written)
{
	bool activateBtns =
		std::find(m_enabledChannels.begin(), m_enabledChannels.end(), true) != m_enabledChannels.end();
	Q_EMIT activateRunBtns(activateBtns && written);
}

void Ad74413r::onThresholdWritten(bool written)
{
	bool activateBtns =
		std::find(m_enabledChannels.begin(), m_enabledChannels.end(), true) != m_enabledChannels.end();
	if(written) {
		Q_EMIT broadcastThreshold();
	}
	Q_EMIT activateRunBtns(activateBtns && written);
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
	if(toggled) {
		m_singleBtn->setChecked(false);
		m_singleBtn->setEnabled(false);
		verifyChnlsChanges();
		if(!m_readerThread->isRunning()) {
			m_acqHandler->setSingleCapture(false);
			m_acqHandler->resetPlotParameters();
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
		verifyChnlsChanges();
		if(runBtnChecked) {
			m_runBtn->setChecked(false);
		}
		m_acqHandler->setSingleCapture(true);
		if(!m_readerThread->isRunning()) {
			m_acqHandler->resetPlotParameters();
			int bufNumber = m_acqHandler->getRequiredBuffersNumber();
			m_readerThread->startCapture(bufNumber);
		}
		m_singleBtn->setEnabled(false);
	}
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

void Ad74413r::onConfigBtnPressed()
{
	bool runBtnChecked = m_runBtn->isChecked();
	bool singleBtnChecked = m_singleBtn->isChecked();

	if(runBtnChecked) {
		m_runBtn->setChecked(false);
	}
	if(singleBtnChecked) {
		m_singleBtn->setChecked(false);
	}
	Q_EMIT configBtnPressed();
}

// TBD - The value of 500 is set so that the device reaches a stable state before the new acquisition
// It is possible that this problem can be solved in other way
void Ad74413r::onReaderThreadFinished()
{
	bool singleCaptureOn = m_acqHandler->singleCapture();
	if(singleCaptureOn) {
		m_acqHandler->setSingleCapture(false);
	}
	if(m_runBtn->isChecked()) {
		onRunBtnPressed(false);
		m_rstAcqTimer->start(500);
	}
	if(m_singleBtn->isChecked()) {
		m_singleBtn->setChecked(false);
	}
}

void Ad74413r::onSingleCaptureFinished()
{
	bool runBtnChecked = m_runBtn->isChecked();
	if(!runBtnChecked) {
		if(m_tme->running()) {
			m_tme->setRunning(false);
		}
		m_singleBtn->setEnabled(true);
	}
	m_readerThread->requestStop();
	m_singleBtn->setChecked(false);
}

void Ad74413r::updateXData(int dataSize)
{
	double timespanValue = m_timespanSpin->value();
	double plotSamples = m_currentSamplingInfo.sampleRate * timespanValue;
	if(m_xTime.size() == plotSamples && dataSize == plotSamples) {
		return;
	}
	m_xTime.clear();
	for(int i = dataSize - 1; i >= 0; i--) {
		m_xTime.push_back(-(i / plotSamples) * timespanValue);
	}
}

void Ad74413r::plotData(QVector<double> chnlData, int chnlIdx)
{
	int dataSize = chnlData.size();
	updateXData(dataSize);
	m_plotChnls[chnlIdx]->curve()->setSamples(m_xTime.data(), chnlData.data(), dataSize);
	m_currentSamplingInfo.plotSize = dataSize;
	m_info->update(m_currentSamplingInfo);
	m_plot->replot();
}

void Ad74413r::onBufferRefilled(QMap<int, QVector<double>> bufferData)
{
	QList<int> chnls = m_plotChnls.keys();
	int dataIdx = 0;
	for(int chnlIdx : chnls) {
		if(!m_enabledChannels[chnlIdx]) {
			continue;
		}
		plotData(bufferData[dataIdx], chnlIdx);
		m_labels[chnlIdx].last()->setValue(bufferData[dataIdx].last());
		dataIdx++;
	}
}

void Ad74413r::onSamplingFreqComputed(double freq)
{
	m_currentSamplingInfo.sampleRate = freq;
	m_info->update(m_currentSamplingInfo);
}

void Ad74413r::initPlotData()
{
	m_currentSamplingInfo.startingPoint = 0;
	m_currentSamplingInfo.plotSize = 0;
	m_currentSamplingInfo.sampleRate = MAX_SAMPLE_RATE;
	m_currentSamplingInfo.freqOffset = 0;

	m_xTime.clear();
	for(int i = m_currentSamplingInfo.sampleRate - 1; i >= 0; i--) {
		m_xTime.push_back(-(i / m_currentSamplingInfo.sampleRate));
	}
}

void Ad74413r::initPlot()
{
	m_plot->xAxis()->setInterval(-1, 0);
	m_plot->xAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	m_plot->xAxis()->scaleDraw()->setFloatPrecision(2);
	m_plot->xAxis()->setVisible(false);
	m_plot->yAxis()->setVisible(false);
	m_plot->replot();
}

void Ad74413r::showPlotLabels(bool b)
{
	m_plot->setShowXAxisLabels(b);
	m_plot->setShowYAxisLabels(b);
	m_plot->showAxisLabels();
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

void Ad74413r::setupChannelBtn(MenuControlButton *btn, PlotChannel *ch, QString chnlId, int chnlIdx)
{
	btn->setName(chnlId);
	btn->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	btn->setOpenMenuChecksThis(true);
	btn->setDoubleClickToOpenMenu(true);
	btn->setColor(ch->curve()->pen().color());
	btn->button()->setVisible(false);
	btn->setCheckable(true);
	btn->checkBox()->setChecked(false);

	connect(btn, &MenuControlButton::toggled, this, [=, this](bool en) {
		if(!en) {
			return;
		}
		if(btn->checkBox()->isChecked()) {
			m_plot->selectChannel(ch);
			m_plot->replot();
		}
		m_chnlsMenuBtn->button()->setChecked(en);
		m_channelStack->show(chnlId);
	});

	connect(
		btn->checkBox(), &QCheckBox::toggled, this,
		[=, this](bool en) {
			onChannelBtnChecked(chnlIdx, en);
			ch->handle()->handle()->setVisible(en);
			ch->setEnabled(en);
			if(en && btn->isChecked()) {
				m_plot->selectChannel(ch);
			}
			m_plot->replot();
		},
		Qt::DirectConnection);
}

void Ad74413r::setupChannel(int chnlIdx, QString function)
{
	if(function.compare("no_config") != 0) {
		QString chnlId(function + " " + QString::number(chnlIdx + 1));
		QPen chPen = QPen(QColor(StyleHelper::getColor("CH" + QString::number(chnlIdx))), 1);

		QString unit = m_swiotAdLogic->getPlotChnlUnitOfMeasure(chnlIdx);
		auto yRange = m_swiotAdLogic->getPlotChnlRangeValues(chnlIdx);

		PlotAxis *chYAxis = createYChnlAxis(chPen, unit, yRange.first, yRange.second);
		PlotChannel *plotCh = new PlotChannel(chnlId, chPen, m_plot->xAxis(), chYAxis, this);
		m_plot->addPlotChannel(plotCh);
		plotCh->setEnabled(false);

		PlotAxisHandle *chHandle = new PlotAxisHandle(m_plot, chYAxis);
		chHandle->handle()->setBarVisibility(BarVisibility::ON_HOVER);
		chHandle->handle()->setColor(chPen.color());
		chHandle->handle()->setHandlePos(HandlePos::SOUTH_EAST);
		connect(chHandle, &PlotAxisHandle::scalePosChanged, this, [=](double pos) {
			double min = chYAxis->min() - pos;
			double max = chYAxis->max() - pos;
			chYAxis->setInterval(min, max);
			m_plot->plot()->replot();
		});
		plotCh->setHandle(chHandle);
		m_plot->addPlotAxisHandle(chHandle);
		m_plotChnls.insert(chnlIdx, plotCh);

		QString unitPerDivLabel = unit + "/div";
		QString valueLabel = "Value(" + unit + ")";
		createMeasurementsLabel(chnlIdx, chPen, {unitPerDivLabel, valueLabel});
		updateMeasurements(chYAxis, chnlIdx);

		QMap<QString, iio_channel *> chnlsMap = m_swiotAdLogic->getIioChnl(chnlIdx);
		BufferMenuView *menu = new BufferMenuView(chnlsMap, m_conn, this);
		menu->init(chnlId, function, chPen, unit, yRange.first, yRange.second);
		std::pair<double, double> offsetScale = {0, 1};
		offsetScale = (chnlsMap.size() > 1) ? m_swiotAdLogic->getChnlOffsetScale(chnlIdx + MAX_INPUT_CHNLS_NO)
						    : m_swiotAdLogic->getChnlOffsetScale(chnlIdx);
		menu->getAdvMenu()->setOffsetScalePair(offsetScale);

		m_channelStack->add(chnlId, menu);

		MenuControlButton *btn = new MenuControlButton(m_devBtn);
		m_devBtn->add(btn);
		m_chnlsBtnGroup->addButton(btn);
		setupChannelBtn(btn, plotCh, chnlId, chnlIdx);

		connect(m_runBtn, &QPushButton::toggled, menu, &BufferMenuView::runBtnsPressed);
		connect(m_singleBtn, &QPushButton::toggled, menu, &BufferMenuView::runBtnsPressed);

		connect(menu, &BufferMenuView::setYMin, chYAxis, &PlotAxis::setMin);
		connect(chYAxis, &PlotAxis::minChanged, this, [=, this]() {
			updateMeasurements(chYAxis, chnlIdx);
			Q_EMIT menu->minChanged(chYAxis->min());
		});
		connect(menu, &BufferMenuView::setYMax, chYAxis, &PlotAxis::setMax);
		connect(chYAxis, &PlotAxis::maxChanged, this, [=, this]() {
			updateMeasurements(chYAxis, chnlIdx);
			Q_EMIT menu->maxChanged(chYAxis->max());
		});

		connect(menu, &BufferMenuView::samplingFrequencyUpdated, this,
			[=, this](int sr) { onSamplingFrequencyUpdated(chnlIdx, sr); });

		connect(menu, &BufferMenuView::diagSamplingFreqChange, this, [=, this](QString data) {
			onSamplingFrequencyUpdated(chnlIdx, data.toInt());
			Q_EMIT updateDiagSamplingFreq(data);
		});
		connect(menu, &BufferMenuView::diagnosticFunctionUpdated, this, &Ad74413r::onDiagnosticFunctionUpdated);
		connect(menu, &BufferMenuView::samplingFreqWritten, this, &Ad74413r::samplingFreqWritten);
		connect(menu, &BufferMenuView::thresholdWritten, this, &Ad74413r::onThresholdWritten);
		connect(this, &Ad74413r::broadcastThreshold, menu, &BufferMenuView::broadcastThreshold);
		connect(this, &Ad74413r::updateDiagSamplingFreq, menu, &BufferMenuView::updateDiagSamplingFreq);
	}
	m_currentChannelSelected++;
	if(m_currentChannelSelected == 4) {
		m_swiotAdLogic->initDiagnosticChannels();
	}
}

void Ad74413r::setupChannelsMenuControlBtn(MenuControlButton *btn, QString name)
{
	btn->setName(name);
	btn->setOpenMenuChecksThis(true);
	btn->setDoubleClickToOpenMenu(true);
	btn->checkBox()->setVisible(false);
	btn->button()->setChecked(true);
	btn->setChecked(true);
}

void Ad74413r::updateMeasurements(PlotAxis *axis, int chnlIdx)
{
	double numOfDivs = axis->divs();
	double unitsPerDivs = abs(axis->max() - axis->min()) / numOfDivs;
	m_labels[chnlIdx].first()->setValue(unitsPerDivs);
}

void Ad74413r::createMeasurementsLabel(int chnlIdx, QPen chPen, QStringList labels)
{
	for(const QString &label : labels) {
		MeasurementLabel *ml = new MeasurementLabel(this);
		ml->setColor(chPen.color());
		ml->setName(label);
		m_labels[chnlIdx].append(ml);
		m_measurePanel->addMeasurement(ml);
	}
}

void Ad74413r::setupDeviceBtn()
{
	VerticalChannelManager *vcm = new VerticalChannelManager(this);
	m_tool->leftStack()->add("vcm", vcm);
	m_devBtn = new CollapsableMenuControlButton(this);
	m_devBtn->getControlBtn()->setName("AD74413R");
	m_devBtn->getControlBtn()->setCheckable(false);
	m_devBtn->getControlBtn()->button()->setVisible(false);
	vcm->add(m_devBtn);
	m_chnlsBtnGroup = new QButtonGroup(this);
}

void Ad74413r::setupMeasureButtonHelper(MenuControlButton *btn)
{
	btn->setName("Measure");
	btn->checkBox()->setVisible(false);
	btn->button()->setVisible(false);
	btn->setChecked(true);
}

void Ad74413r::setupToolTemplate()
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout *layout = new QHBoxLayout(this);
	setLayout(layout);
	StyleHelper::GetInstance()->initColorMap();

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->topCentral()->setVisible(true);
	m_tool->centralContainer()->setVisible(true);
	m_tool->bottomContainer()->setVisible(true);
	m_tool->leftContainer()->setVisible(true);
	m_tool->rightContainer()->setVisible(true);

	m_tool->setTopContainerHeight(100);
	m_tool->setLeftContainerWidth(200);
	m_tool->setRightContainerWidth(300);

	layout->addWidget(m_tool);

	m_plot = new PlotWidget(this);
	m_info = new TimePlotInfo(m_plot, this);
	m_plot->addPlotInfoSlot(m_info);
	initPlot();
	setupDeviceBtn();
	m_tool->addWidgetToCentralContainerHelper(m_plot);

	m_infoBtn = new InfoBtn(this);
	m_infoBtn->installEventFilter(this);
	m_settingsBtn = new GearBtn(this);
	m_runBtn = new RunBtn(this);
	m_runBtn->setEnabled(false);
	m_runBtn->setChecked(false);
	m_singleBtn = new SingleShotBtn(this);
	m_singleBtn->setEnabled(false);
	m_singleBtn->setChecked(false);
	m_configBtn = createConfigBtn();

	MenuControlButton *measure = new MenuControlButton(this);
	setupMeasureButtonHelper(measure);
	m_measurePanel = new MeasurementsPanel(this);
	m_tool->topStack()->add(measureMenuId, m_measurePanel);
	connect(measure, &MenuControlButton::toggled, this, [&](bool en) {
		if(en)
			m_tool->requestMenu(measureMenuId);
		m_tool->openTopContainerHelper(en);
	});

	m_chnlsMenuBtn = new MenuControlButton(this);
	setupChannelsMenuControlBtn(m_chnlsMenuBtn, "Channels");
	connect(m_chnlsMenuBtn->button(), &QAbstractButton::toggled, this, [=, this](bool b) {
		if(b) {
			m_settingsBtn->setChecked(!b);
			m_tool->requestMenu(channelsMenuId);
		}
	});
	connect(m_chnlsMenuBtn, &QPushButton::toggled, dynamic_cast<MenuHAnim *>(m_tool->leftContainer()),
		&MenuHAnim::toggleMenu);

	QPushButton *openLastMenuBtn =
		new OpenLastMenuBtn(dynamic_cast<MenuHAnim *>(m_tool->rightContainer()), true, this);
	m_rightMenuBtnGrp = dynamic_cast<OpenLastMenuBtn *>(openLastMenuBtn)->getButtonGroup();
	m_rightMenuBtnGrp->addButton(m_chnlsMenuBtn->button());

	m_channelStack = new MapStackedWidget(this);
	m_tool->rightStack()->add(channelsMenuId, m_channelStack);

	QString settingsMenuId = "PlotSettings";
	m_tool->rightStack()->add(settingsMenuId, createSettingsMenu(this));
	connect(m_settingsBtn, &QPushButton::toggled, this, [=, this](bool b) {
		if(b)
			m_tool->requestMenu(settingsMenuId);
	});
	m_rightMenuBtnGrp->addButton(m_settingsBtn);

	m_tool->addWidgetToBottomContainerHelper(m_chnlsMenuBtn, TTA_LEFT);
	m_tool->addWidgetToBottomContainerHelper(measure, TTA_RIGHT);

	m_tool->addWidgetToTopContainerMenuControlHelper(openLastMenuBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerMenuControlHelper(m_settingsBtn, TTA_LEFT);

	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(m_singleBtn, TTA_RIGHT);

	m_tool->addWidgetToTopContainerHelper(m_infoBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_configBtn, TTA_LEFT);
}

QPushButton *Ad74413r::createConfigBtn()
{
	QPushButton *configBtn = new QPushButton();
	StyleHelper::BlueGrayButton(configBtn, "config_btn");
	configBtn->setFixedWidth(128);
	configBtn->setCheckable(false);
	configBtn->setText("Config");
	return configBtn;
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

bool Ad74413r::eventFilter(QObject *watched, QEvent *event)
{
	if(watched == (QObject *)m_infoBtn) {
		if(event->type() == QEvent::Enter) {
			auto enabledPlotsNo = std::count(m_enabledChannels.begin(), m_enabledChannels.end(), true);
			m_infoBtn->setToolTip(
				"sps = samples per second \n"
				"sps = sampling_frequency / enabled_channels \n"
				"Enabled channels = " +
				QString::number(enabledPlotsNo) + "\n" +
				"Samples per channel = " + QString::number(m_currentSamplingInfo.plotSize) + "\n" +
				"Sampling frequency = " + QString::number(m_currentSamplingInfo.sampleRate));
		}
		return false;
	} else {
		return QWidget::eventFilter(watched, event);
	}
}

void Ad74413r::initTutorialProperties()
{
	// initialize components that might be used for the AD74413R tutorial
	m_plot->setProperty("tutorial_name", "AD74413R_PLOT");
	m_singleBtn->setProperty("tutorial_name", "SINGLE_BUTTON");
	m_runBtn->setProperty("tutorial_name", "RUN_BUTTON");
	m_settingsBtn->setProperty("tutorial_name", "AD74413R_SETTINGS");
	m_configBtn->setProperty("tutorial_name", "CONFIG_BUTTON");
}
