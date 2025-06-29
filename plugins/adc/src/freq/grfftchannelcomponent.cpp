/*
 * Copyright (c) 2024 Analog Devices Inc.
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
 *
 */

#include "grfftchannelcomponent.h"
#include <pluginbase/preferences.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuheader.h>

#include <gr-util/griiofloatchannelsrc.h>
#include <gr-util/grsignalpath.h>

#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetbuilder.h>
#include <freq/fftplotcomponentchannel.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>
#include <QSpinBox>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_GRFFTChannelComponent, "GRFFTChannelComponent");

using namespace scopy;
using namespace scopy::grutil;
using namespace scopy::adc;

GRFFTChannelComponent::GRFFTChannelComponent(GRIIOFloatChannelNode *node_I, GRIIOFloatChannelNode *node_Q,
					     FFTPlotComponent *m_plot, GRFFTSinkComponent *grtsc, QPen pen,
					     QWidget *parent)
	: ChannelComponent(node_I->name() + node_Q->name(), pen, parent)

{

	m_plotChannelCmpt = new FFTPlotComponentChannel(this, m_plot, this);

	m_fftPlotComponentChannel = dynamic_cast<FFTPlotComponentChannel *>(m_plotChannelCmpt);
	connect(m_chData, &ChannelData::newData, m_fftPlotComponentChannel, &FFTPlotComponentChannel::onNewData);

	m_node = node_I;
	m_channelName = node_I->name() + "-" + node_Q->name();
	m_src_I = node_I->src();
	m_src_Q = node_Q->src();

	GRIIOComplexChannelSrc *m_src_complex = new GRIIOComplexChannelSrc(
		m_channelName, m_src_I->getDeviceSrc(), m_src_I->getChannelName(), m_src_Q->getChannelName(), this);
	m_src = m_src_complex;

	m_grtch = new GRFFTComplexChannelSigpath(grtsc->name(), this, m_node->top()->src(), m_src_complex,
						 this); // change prototype here (?)
	connect(this, &GRFFTChannelComponent::powerOffsetChanged, this,
		[=](double v) { dynamic_cast<GRFFTComplexChannelSigpath *>(m_grtch)->setPowerOffset(v); });

	connect(this, &GRFFTChannelComponent::windowChanged, this,
		[=](int w) { dynamic_cast<GRFFTComplexChannelSigpath *>(m_grtch)->setWindow(w); });

	connect(this, &GRFFTChannelComponent::windowCorrectionChanged, this,
		[=](bool b) { dynamic_cast<GRFFTComplexChannelSigpath *>(m_grtch)->setWindowCorrection(b); });

	m_complex = true;
	_init();
}

GRFFTChannelComponent::GRFFTChannelComponent(GRIIOFloatChannelNode *node, FFTPlotComponent *m_plot,
					     GRFFTSinkComponent *grtsc, QPen pen, QWidget *parent)
	: ChannelComponent(node->name(), pen, parent)

{
	m_plotChannelCmpt = new FFTPlotComponentChannel(this, m_plot, this);

	m_fftPlotComponentChannel = dynamic_cast<FFTPlotComponentChannel *>(m_plotChannelCmpt);
	connect(m_chData, &ChannelData::newData, m_fftPlotComponentChannel, &FFTPlotComponentChannel::onNewData);

	m_node = node;
	m_src = node->src();
	m_channelName = node->name();
	m_grtch = new GRFFTChannelSigpath(grtsc->name(), this, m_node->top()->src(), node->src(), this);

	m_complex = false;

	connect(this, &GRFFTChannelComponent::powerOffsetChanged, this,
		[=](double v) { dynamic_cast<GRFFTChannelSigpath *>(m_grtch)->setPowerOffset(v); });

	connect(this, &GRFFTChannelComponent::windowChanged, this,
		[=](int w) { dynamic_cast<GRFFTChannelSigpath *>(m_grtch)->setWindow(w); });

	connect(this, &GRFFTChannelComponent::windowCorrectionChanged, this,
		[=](bool b) { dynamic_cast<GRFFTChannelSigpath *>(m_grtch)->setWindowCorrection(b); });
	_init();
}

void GRFFTChannelComponent::_init()
{
	m_running = false;
	m_scaleAvailable = m_src->scaleAttributeAvailable(); // query from GRIIOFloatChannel;
	m_powerOffset = 0;
	m_window = 1;
	m_windowCorrection = true;

	/*	m_measureMgr = new TimeMeasureManager(this);
		m_measureMgr->initMeasure(m_pen);
		m_measureMgr->getModel()->setAdcBitCount(m_src->getFmt()->bits);*/

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	auto m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);
	widget = createMenu(this);
	m_lay->addWidget(widget);
	setLayout(m_lay);

	m_fftPlotComponentChannel->markerController()->setComplex(m_complex);
	createMenuControlButton(this);
}
GRFFTChannelComponent::~GRFFTChannelComponent() {}

MeasureManagerInterface *GRFFTChannelComponent::getMeasureManager() { return nullptr; }

PlotMarkerController *GRFFTChannelComponent::markerController()
{
	return m_fftPlotComponentChannel->markerController();
}

QWidget *GRFFTChannelComponent::createYAxisMenu(QWidget *parent)
{
	m_yaxisMenu = new MenuSectionCollapseWidget("Y-AXIS", MenuCollapseSection::MHCW_ONOFF,
						    MenuCollapseSection::MHW_BASEWIDGET, parent);
	m_yCtrl = new MenuPlotAxisRangeControl(m_fftPlotComponentChannel->m_fftPlotYAxis, m_yaxisMenu);

	connect(m_yaxisMenu->collapseSection()->header(), &QAbstractButton::toggled, this,
		[=](bool b) { m_fftPlotComponentChannel->lockYAxis(!b); });

	m_yaxisMenu->contentLayout()->addWidget(m_yCtrl);

	return m_yaxisMenu;
}

QWidget *GRFFTChannelComponent::createCurveMenu(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget("CURVE", MenuCollapseSection::MHCW_NONE,
									   MenuCollapseSection::MHW_BASEWIDGET, parent);
	section->contentLayout()->setSpacing(10);

	m_curvemenu = new MenuPlotChannelCurveStyleControl(section);
	section->contentLayout()->addWidget(m_curvemenu);
	return section;
}

QPushButton *GRFFTChannelComponent::createSnapshotButton(QWidget *parent)
{
	QPushButton *snapBtn = new QPushButton("Snapshot", parent);
	StyleHelper::BasicButton(snapBtn);

	/*connect(snapBtn, &QPushButton::clicked, this, [=]() {
		std::vector<float> x, y;
		auto data = m_fftPlotComponentChannel->m_fftPlotCh->curve()->data();
		for(int i = 0; i < data->size(); i++) {
			x.push_back(data->sample(i).x());
			y.push_back(data->sample(i).y());
		}
		SnapshotRecipe rec{x, y, m_fftPlotComponentChannel->m_fftComponent, "REF - " + m_channelName};
		AcqTreeNode *treeRoot = m_node->treeRoot();
		ImportFloatChannelNode *snap = new ImportFloatChannelNode(rec, treeRoot);
		treeRoot->addTreeChild(snap);
	});*/

	snapBtn->setEnabled(false);
	return snapBtn;
}

QWidget *GRFFTChannelComponent::createMenu(QWidget *parent)
{
	ChannelComponent::initMenu(parent);
	QWidget *yaxismenu = createYAxisMenu(m_menu);
	QWidget *curvemenu = createCurveMenu(m_menu);
	QWidget *markerMenu = createMarkerMenu(m_menu);
	QWidget *avgMenu = createAveragingMenu(m_menu);
	m_menu->add(yaxismenu, "yaxis");
	m_menu->add(markerMenu, "marker");
	m_menu->add(avgMenu, "average");
	m_menu->add(curvemenu, "curve");

	if(dynamic_cast<GRIIOComplexChannelSrc *>(m_src) != nullptr) {
		auto src = dynamic_cast<GRIIOComplexChannelSrc *>(m_src);
		QWidget *attrmenui = createChAttrMenu(m_src_I->channel(), "ATTRIBUTES - I CHANNEL", m_menu);
		m_menu->add(attrmenui, "attr");
		QWidget *attrmenuq = createChAttrMenu(m_src_Q->channel(), "ATTRIBUTES - Q CHANNEL", m_menu);
		m_menu->add(attrmenuq, "attr");
	} else {
		auto src = dynamic_cast<GRIIOFloatChannelSrc *>(m_src);
		QWidget *attrmenui = createChAttrMenu(src->channel(), "ATTRIBUTES", m_menu);
		m_menu->add(attrmenui, "attr");
	}
	// QWidget *measuremenu = m_measureMgr->createMeasurementMenu(m_menu);

	m_snapBtn = createSnapshotButton(m_menu);

	// m_menu->add(measuremenu, "measure");
	m_menu->add(m_snapBtn, "snap", MenuWidget::MA_BOTTOMLAST);

	return m_menu;
}

QWidget *GRFFTChannelComponent::createAveragingMenu(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget("AVERAGING", MenuCollapseSection::MHCW_ONOFF,
									   MenuCollapseSection::MHW_BASEWIDGET, parent);
	auto layout = new QVBoxLayout();
	layout->setSpacing(0);
	layout->setMargin(0);

	QLabel *avgLabel = new QLabel("Size", this);
	Style::setStyle(avgLabel, style::properties::label::subtle);
	QSpinBox *avgSpinBox = new QSpinBox(this);
	avgSpinBox->setRange(2, 1000);
	avgSpinBox->setValue(2);

	layout->addWidget(avgLabel);
	layout->addWidget(avgSpinBox);

	connect(section->collapseSection()->header(), &QAbstractButton::toggled, this, [=](bool b) {
		auto ch = dynamic_cast<FFTChannel *>(m_grtch);
		if(ch) {
			int size = b ? avgSpinBox->value() : 1;
			ch->setAveragingSize(size);
		}
	});

	connect(avgSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int value) {
		auto ch = dynamic_cast<FFTChannel *>(m_grtch);
		if(ch) {
			ch->setAveragingSize(value);
		}
	});

	section->contentLayout()->addLayout(layout);
	section->setCollapsed(true);
	return section;
}

QWidget *GRFFTChannelComponent::createMarkerMenu(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget("MARKER", MenuCollapseSection::MHCW_ONOFF,
									   MenuCollapseSection::MHW_BASEWIDGET, parent);

	auto layout = new QVBoxLayout();
	layout->setSpacing(10);
	layout->setMargin(0);

	MenuCombo *markerCb = new MenuCombo("Marker Type", section);
	markerCb->combo()->addItem("Peak", PlotMarkerController::MC_PEAK);
	markerCb->combo()->addItem("Fixed", PlotMarkerController::MC_FIXED);
	markerCb->combo()->addItem("Single Tone", PlotMarkerController::MC_SINGLETONE);
	if(m_complex) {
		markerCb->combo()->addItem("Image", PlotMarkerController::MC_IMAGE);
	}

	markerCb->combo()->setCurrentIndex(0);

	MenuOnOffSwitch *fixedMarkerEditBtn = new MenuOnOffSwitch("Marker editable", section);
	connect(fixedMarkerEditBtn->onOffswitch(), &QAbstractButton::toggled, this,
		[=](bool b) { m_fftPlotComponentChannel->markerController()->setFixedHandleVisible(b); });
	fixedMarkerEditBtn->onOffswitch()->setChecked(true);
	fixedMarkerEditBtn->setVisible(false);

	MenuSpinbox *markerCnt = new MenuSpinbox("Marker count", 5, "markers", 0, 9, true, false, section);
	markerCnt->setIncrementMode(MenuSpinbox::IS_FIXED);
	markerCnt->setScaleRange(1, 10);
	markerCnt->setValue(5);

	connect(markerCnt, &MenuSpinbox::valueChanged, this, [=](double cnt) {
		m_fftPlotComponentChannel->markerController()->setNrOfMarkers(cnt);
		m_fftPlotComponentChannel->markerController()->computeMarkers();
	});

	connect(section->collapseSection()->header(), &QAbstractButton::toggled, this, [=](bool b) {
		if(b) {
			auto markerType = static_cast<PlotMarkerController::MarkerTypes>(
				markerCb->combo()->currentData().toInt());
			m_fftPlotComponentChannel->markerController()->setFixedHandleVisible(
				markerType == PlotMarkerController::MC_FIXED &&
				fixedMarkerEditBtn->onOffswitch()->isChecked());
			m_fftPlotComponentChannel->markerController()->setMarkerType(markerType);
		} else {
			m_fftPlotComponentChannel->markerController()->setFixedHandleVisible(false);
			m_fftPlotComponentChannel->markerController()->setMarkerType(PlotMarkerController::MC_NONE);
		}
	});

	connect(markerCb->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		auto markerType =
			static_cast<PlotMarkerController::MarkerTypes>(markerCb->combo()->currentData().toInt());
		if(markerType == PlotMarkerController::MC_SINGLETONE) {
			markerCnt->setMinValue(2);
			if(markerCnt->value() < 2)
				markerCnt->setValue(2);
		} else {
			markerCnt->setMinValue(0);
		}
		m_fftPlotComponentChannel->markerController()->setFixedHandleVisible(
			markerType == PlotMarkerController::MC_FIXED && fixedMarkerEditBtn->onOffswitch()->isChecked());
		m_fftPlotComponentChannel->markerController()->setMarkerType(markerType);
		fixedMarkerEditBtn->setVisible(markerCb->combo()->currentData().toInt() ==
					       PlotMarkerController::MC_FIXED);
	});

	layout->addWidget(markerCb);
	layout->addWidget(markerCnt);
	layout->addWidget(fixedMarkerEditBtn);

	section->contentLayout()->addLayout(layout);
	section->setCollapsed(true);
	return section;
}

QWidget *GRFFTChannelComponent::createChAttrMenu(iio_channel *ch, QString title, QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(title, MenuCollapseSection::MHCW_NONE,
									   MenuCollapseSection::MHW_BASEWIDGET, parent);
	QList<IIOWidget *> attrWidgets = IIOWidgetBuilder(section).channel(ch).buildAll();

	auto layout = new QVBoxLayout();
	layout->setSpacing(10);
	layout->setMargin(0);
	layout->setContentsMargins(0, 0, 0, 10); // bottom margin

	for(auto w : attrWidgets) {
		layout->addWidget(w);
	}

	section->contentLayout()->addLayout(layout);
	section->setCollapsed(true);
	return section;
}

void GRFFTChannelComponent::onStart()
{
	m_running = true;
	m_grtch->sigpath()->setEnabled(true);
	// m_measureMgr->getModel()->setSampleRate(m_plotSampleRate);
}

void GRFFTChannelComponent::onStop()
{
	m_running = false;
	m_grtch->sigpath()->setEnabled(false);
}

void GRFFTChannelComponent::addChannelToPlot()
{
	m_yCtrl->addAxis(m_fftPlotComponentChannel->m_fftPlotYAxis);
	m_curvemenu->addChannels(m_fftPlotComponentChannel->m_fftPlotCh);
}

void GRFFTChannelComponent::removeChannelFromPlot()
{
	m_yCtrl->removeAxis(m_fftPlotComponentChannel->m_fftPlotYAxis);
	m_curvemenu->removeChannels(m_fftPlotComponentChannel->m_fftPlotCh);
}

bool GRFFTChannelComponent::enabled() const { return m_enabled && !(m_complex ^ m_samplingInfo.complexMode); }

void GRFFTChannelComponent::setSamplingInfo(SamplingInfo p)
{
	ChannelComponent::setSamplingInfo(p);
	bool b = !(m_complex ^ p.complexMode); // hide if they are different
	m_ctrl->setVisible(b);
	if(enabled()) {
		m_plotChannelCmpt->enable();
	} else {
		m_plotChannelCmpt->disable();
	}

	// dont care (yet) about rest of sampling info - could be useful for measurements
}

void GRFFTChannelComponent::enable()
{
	ChannelComponent::enable();
	m_ctrl->checkBox()->setChecked(true);
	if(m_running) {
		m_grtch->sigpath()->setEnabled(true);
	}
	Q_EMIT m_node->top()->src()->requestRebuild(); // sigpath()->requestRebuild();
}

void GRFFTChannelComponent::disable()
{
	ChannelComponent::disable();
	m_ctrl->checkBox()->setChecked(false);
	if(m_running) {
		m_grtch->sigpath()->setEnabled(false);
	}
	Q_EMIT m_node->top()->src()->requestRebuild(); // sigpath()->requestRebuild();
}

// MeasureManagerInterface *GRFFTChannelComponent::getMeasureManager() { return m_measureMgr; }

GRSignalPath *GRFFTChannelComponent::sigpath() { return m_grtch->sigpath(); }

QVBoxLayout *GRFFTChannelComponent::menuLayout() { return m_layScroll; }

void GRFFTChannelComponent::onInit()
{
	// Defaults
	addChannelToPlot();

	m_yaxisMenu->setCollapsed(true);
	m_yCtrl->setMin(-140.0);
	m_yCtrl->setMax(20.0);
}

void GRFFTChannelComponent::onDeinit() {}

void GRFFTChannelComponent::onNewData(const float *xData, const float *yData, size_t size, bool copy)
{
	m_grtch->onNewData(xData, yData, size, copy);
	/*auto model = m_measureMgr->getModel();
	model->setDataSource(yData, size);
	model->measure();*/
	m_snapBtn->setEnabled(true);
}

bool GRFFTChannelComponent::sampleRateAvailable() { return m_src->samplerateAttributeAvailable(); }

double GRFFTChannelComponent::sampleRate() { return m_src->readSampleRate(); }

double GRFFTChannelComponent::powerOffset() { return m_powerOffset; }

void GRFFTChannelComponent::setPowerOffset(double newPowerOffset)
{
	if(m_powerOffset == newPowerOffset)
		return;
	m_powerOffset = newPowerOffset;
	Q_EMIT powerOffsetChanged(m_powerOffset);
}

int GRFFTChannelComponent::window() const { return m_window; }

void GRFFTChannelComponent::setWindow(int newWindow)
{
	if(m_window == newWindow)
		return;
	m_window = newWindow;
	Q_EMIT windowChanged(newWindow);
}

// this cannot be implemented since averaging size can only be changed from within the channel signalpath
void GRFFTChannelComponent::setAveragingSize(int size) {}

bool GRFFTChannelComponent::windowCorrection() const { return m_windowCorrection; }

void GRFFTChannelComponent::setWindowCorrection(bool newWindowCorr)
{
	if(m_windowCorrection == newWindowCorr)
		return;
	m_windowCorrection = newWindowCorr;
	Q_EMIT windowCorrectionChanged(newWindowCorr);
}
