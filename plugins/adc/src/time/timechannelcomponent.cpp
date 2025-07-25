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

#include "customSourceBlocks.h"
#include <pluginbase/preferences.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuheader.h>
#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetbuilder.h>
#include <blockManager.h>
#include <channel.h>
#include <sourceBlock.h>
#include <timechannelcomponent.h>
#include <timeplotcomponentchannel.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_TIMECHANNELCOMPONENT, "TimeChannelComponent");

using namespace scopy;
using namespace scopy::datasink;
using namespace scopy::adc;

TimeChannelComponent::TimeChannelComponent(IIOSourceBlock *sourceBlock, uint sourceChannel, uint outputChannel,
					   BlockManager *manager, TimePlotComponent *m_plot, QPen pen, QWidget *parent)
	: ChannelComponent(sourceBlock->name() + "_ch" + QString::number(sourceChannel), pen, parent)
{
	m_plotChannelCmpt = new TimePlotComponentChannel(this, m_plot, this);
	m_timePlotComponentChannel = dynamic_cast<TimePlotComponentChannel *>(m_plotChannelCmpt);
	connect(m_chData, &scopy::ChannelData::newData, m_timePlotComponentChannel,
		&TimePlotComponentChannel::onNewData);

	m_sourceBlock = sourceBlock;
	m_sourceChannel = sourceChannel;
	m_manager = manager;

	m_tch = new TimeChannelSigpath(sourceBlock->name() + "_sigpath", this, sourceBlock, sourceChannel,
				       outputChannel, manager, this);

	m_running = false;
	m_autoscaleEnabled = false;

	m_sourceBlock->populateChannelInfo(sourceChannel);
	m_scaleAvailable = m_sourceBlock->scaleAttributeAvailable(sourceChannel);
	// m_unit = m_sourceBlock->unit(); // implement this

	m_channelName = sourceBlock->name() + "_ch" + QString::number(sourceChannel);
	m_measureMgr = new TimeMeasureManager(this);
	m_measureMgr->initMeasure(m_pen);
	m_measureMgr->getModel()->setAdcBitCount(m_sourceBlock->getFmt(sourceChannel)->bits);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	auto m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);
	widget = createMenu(this);
	m_lay->addWidget(widget);
	setLayout(m_lay);
	createMenuControlButton(this);
}

TimeChannelComponent::~TimeChannelComponent() {}

void TimeChannelComponent::setRollingMode(bool mode) { m_tch->setRollingMode(mode); }

QWidget *TimeChannelComponent::createYAxisMenu(QWidget *parent)
{
	m_yaxisMenu = new MenuSectionCollapseWidget("Y-AXIS", MenuCollapseSection::MHCW_ONOFF,
						    MenuCollapseSection::MHW_BASEWIDGET, parent);
	m_yaxisMenu->contentLayout()->setSpacing(6);

	// Y-MODE
	m_ymodeCb = new MenuCombo("YMODE", m_yaxisMenu);
	InfoIconWidget::addHoveringInfoToWidget(m_ymodeCb->label(),
						"Set Y axis scaling mode\nThis does not affect the data", m_ymodeCb);
	auto cb = m_ymodeCb->combo();
	cb->addItem("ADC Counts", YMODE_COUNT);
	cb->addItem("% Full Scale", YMODE_FS);
	cb->addItem("Scale override", YMODE_SCALE_OVERRIDE);

	m_scaleWidget = nullptr;
	if(m_scaleAvailable) {
		cb->addItem(m_unit.name, YMODE_SCALE);
		m_scaleWidget = IIOWidgetBuilder(m_yaxisMenu)
					.device(m_sourceBlock->iioDev())
					.attribute(m_sourceBlock->getChScaleAttr(m_sourceChannel))
					.buildSingle();
	}

	m_scaleOverrideWidget = new QWidget(m_yaxisMenu);
	auto layout = new QVBoxLayout();
	m_scaleOverrideWidget->setLayout(layout);
	layout->setSpacing(0);
	layout->setMargin(0);
	QLabel *scaleLabel = new QLabel("Scale", this);
	Style::setStyle(scaleLabel, style::properties::label::subtle);
	m_scaleSpin = new QDoubleSpinBox(m_scaleOverrideWidget);
	m_scaleSpin->setRange(0, 1000);
	m_scaleSpin->setValue(1);
	layout->addWidget(scaleLabel);
	layout->addWidget(m_scaleSpin);

	m_yCtrl = new MenuPlotAxisRangeControl(m_timePlotComponentChannel->m_timePlotYAxis, m_yaxisMenu);
	m_autoscaleBtn = new MenuOnOffSwitch(tr("AUTOSCALE"), m_yaxisMenu, false);
	m_autoscaler = new PlotAutoscaler(this);
	m_autoscaler->addChannels(m_timePlotComponentChannel->m_timePlotCh);

	connect(m_autoscaler, &PlotAutoscaler::newMin, m_yCtrl, &MenuPlotAxisRangeControl::setMin);
	connect(m_autoscaler, &PlotAutoscaler::newMax, m_yCtrl, &MenuPlotAxisRangeControl::setMax);

	connect(m_yCtrl, &MenuPlotAxisRangeControl::intervalChanged, this, [=](double min, double max) {
		m_timePlotComponentChannel->m_xyPlotYAxis->setInterval(m_yCtrl->min(), m_yCtrl->max());
	});

	connect(m_yaxisMenu->collapseSection()->header(), &QAbstractButton::toggled, this, [=](bool b) {
		m_yLock = b;
		m_timePlotComponentChannel->lockYAxis(!b);
	});

	connect(m_autoscaleBtn->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		m_yCtrl->setEnabled(!b);
		m_autoscaleEnabled = b;
		toggleAutoScale();
	});

	m_yaxisMenu->contentLayout()->addWidget(m_autoscaleBtn);
	m_yaxisMenu->contentLayout()->addWidget(m_yCtrl);
	m_yaxisMenu->contentLayout()->addWidget(m_ymodeCb);
	m_yaxisMenu->contentLayout()->addWidget(m_scaleOverrideWidget);
	if(m_scaleWidget)
		m_yaxisMenu->contentLayout()->addWidget(m_scaleWidget);

	connect(m_scaleSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
		[=](double value) { setYModeHelper(YMODE_SCALE_OVERRIDE); });

	connect(cb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		auto mode = cb->itemData(idx).toInt();
		setYMode(static_cast<YMode>(mode));
	});

	connect(this, &TimeChannelComponent::yModeChanged, this, [=]() {
		int idx = cb->currentIndex();
		int itemcount = cb->count();
		for(int i = 0; i < itemcount; i++) {
			if(cb->itemData(i) == m_ymode) {
				idx = i;
				break;
			}
		}
		cb->setCurrentIndex(idx);
	});

	return m_yaxisMenu;
}

QWidget *TimeChannelComponent::createCurveMenu(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget("CURVE", MenuCollapseSection::MHCW_NONE,
									   MenuCollapseSection::MHW_BASEWIDGET, parent);
	section->contentLayout()->setSpacing(10);

	m_curvemenu = new MenuPlotChannelCurveStyleControl(section);
	section->contentLayout()->addWidget(m_curvemenu);
	return section;
}

QPushButton *TimeChannelComponent::createSnapshotButton(QWidget *parent)
{
	QPushButton *snapBtn = new QPushButton("Snapshot", parent);
	StyleHelper::BasicButton(snapBtn);

	connect(snapBtn, &QPushButton::clicked, this, [=]() {
		std::vector<float> x, y;
		auto data = m_timePlotComponentChannel->m_timePlotCh->curve()->data();
		for(int i = 0; i < data->size(); i++) {
			x.push_back(data->sample(i).x());
			y.push_back(data->sample(i).y());
		}
		SnapshotRecipe rec{x, y, m_timePlotComponentChannel->m_plotComponent, "REF - " + m_channelName};
		// AcqTreeNode *treeRoot = m_node->treeRoot();
		// ImportFloatChannelNode *snap = new ImportFloatChannelNode(rec, treeRoot);
		// treeRoot->addTreeChild(snap);
		// TODO
	});

	snapBtn->setEnabled(false);
	return snapBtn;
}

QWidget *TimeChannelComponent::createMenu(QWidget *parent)
{
	ChannelComponent::initMenu(parent);
	QWidget *yaxismenu = createYAxisMenu(m_menu);
	QWidget *curvemenu = createCurveMenu(m_menu);
	QWidget *attrmenu = createAttrMenu(m_menu);
	QWidget *measuremenu = m_measureMgr->createMeasurementMenu(m_menu);
	m_snapBtn = createSnapshotButton(m_menu);

	m_menu->add(yaxismenu, "yaxis");
	m_menu->add(curvemenu, "curve");
	m_menu->add(attrmenu, "attr");
	m_menu->add(measuremenu, "measure");
	m_menu->add(m_snapBtn, "snap", MenuWidget::MA_BOTTOMLAST);

	return m_menu;
}

QWidget *TimeChannelComponent::createAttrMenu(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget("ATTRIBUTES", MenuCollapseSection::MHCW_NONE,
									   MenuCollapseSection::MHW_BASEWIDGET, parent);
	// QList<IIOWidget *> attrWidgets = IIOWidgetBuilder(section).channel(m_src->channel()).buildAll();
	// TODO

	auto layout = new QVBoxLayout();
	layout->setSpacing(10);
	layout->setMargin(0);
	layout->setContentsMargins(0, 0, 0, 10); // bottom margin

	// for(auto w : attrWidgets) {
	// 	layout->addWidget(w);
	// }
	// TODO

	section->contentLayout()->addLayout(layout);
	section->setCollapsed(true);
	return section;
}

void TimeChannelComponent::onStart()
{
	m_running = true;
	// m_tch->enable();  // This replaces m_grtch->m_signalPath->setEnabled(true);
	toggleAutoScale();
	// m_manager->start();
}

// Replace onStop() method
void TimeChannelComponent::onStop()
{
	m_running = false;
	// m_tch->disable();  // This replaces m_grtch->m_signalPath->setEnabled(false);
	toggleAutoScale();
	if(m_autoscaleEnabled) {
		m_autoscaler->autoscale();
	}
	// m_manager->stop();
}

void TimeChannelComponent::toggleAutoScale()
{
	if(m_running && m_autoscaleEnabled) {
		m_autoscaler->start();
	} else {
		m_autoscaler->stop();
	}
}

void TimeChannelComponent::setYModeHelper(YMode mode)
{
	double scale = 1;
	double offset = 0;
	double ymin = -1;
	double ymax = 1;
	bool is_signed = m_sourceBlock->getFmt(m_sourceChannel)->is_signed;
	int bits = m_sourceBlock->getFmt(m_sourceChannel)->bits;

	switch(mode) {
	case YMODE_COUNT:
		if(m_scaleAvailable) {
			m_scaleWidget->setVisible(false);
		}
		scale = 1;
		if(is_signed) {
			ymin = -(float)((int64_t)1 << (bits - 1));
			ymax = (float)((int64_t)1 << (bits - 1));
		} else {
			ymin = 0;
			ymax = 1 << bits;
		}
		m_timePlotComponentChannel->m_timePlotYAxis->setUnits("");
		m_timePlotComponentChannel->m_timePlotYAxis->scaleDraw()->setFloatPrecision(3);
		m_timePlotComponentChannel->m_timePlotYAxis->getFormatter()->setTwoDecimalMode(false);
		break;
	case YMODE_FS:
		if(m_scaleAvailable) {
			m_scaleWidget->setVisible(false);
		}
		scale = 1.0 / ((float)((uint64_t)1 << bits));
		if(is_signed) {
			ymin = -0.5;
			ymax = 0.5;
		} else {
			ymin = 0;
			ymax = 1;
		}
		m_timePlotComponentChannel->m_timePlotYAxis->setUnits("");
		m_timePlotComponentChannel->m_timePlotYAxis->scaleDraw()->setFloatPrecision(3);
		m_timePlotComponentChannel->m_timePlotYAxis->getFormatter()->setTwoDecimalMode(false);
		break;
	case YMODE_SCALE:
		if(m_scaleAvailable) {
			scale = m_scaleWidget->read().first.toDouble();
			m_scaleWidget->setVisible(true);
		}
		if(is_signed) {
			ymin = -(float)((int64_t)1 << (bits - 1));
			ymax = (float)((int64_t)1 << (bits - 1));
		} else {
			ymin = 0;
			ymax = (1 << bits);
		}
		scale = scale / 1000.0; // target value is in mV
		ymin = ymin * scale;
		ymax = ymax * scale;
		m_timePlotComponentChannel->m_timePlotYAxis->setUnits(unit().symbol);
		m_timePlotComponentChannel->m_timePlotYAxis->scaleDraw()->setFloatPrecision(3);
		m_timePlotComponentChannel->m_timePlotYAxis->getFormatter()->setTwoDecimalMode(true);
		break;
	default:
		break;
	}
	m_yCtrl->setMin(ymin);
	m_yCtrl->setMax(ymax);
	m_tch->setScale(scale);
	m_tch->setOffset(offset);
}

void TimeChannelComponent::addChannelToPlot()
{
	m_yCtrl->addAxis(m_timePlotComponentChannel->m_timePlotYAxis);
	m_curvemenu->addChannels(m_timePlotComponentChannel->m_timePlotCh);
	m_curvemenu->addChannels(m_timePlotComponentChannel->m_xyPlotCh);
	m_autoscaler->addChannels(m_timePlotComponentChannel->m_timePlotCh);
}

void TimeChannelComponent::removeChannelFromPlot()
{
	m_yCtrl->removeAxis(m_timePlotComponentChannel->m_timePlotYAxis);
	m_curvemenu->removeChannels(m_timePlotComponentChannel->m_timePlotCh);
	m_curvemenu->removeChannels(m_timePlotComponentChannel->m_xyPlotCh);
	m_autoscaler->removeChannels(m_timePlotComponentChannel->m_timePlotCh);
}

IIOUnit TimeChannelComponent::unit() const { return m_unit; }

void TimeChannelComponent::enable()
{
	ChannelComponent::enable();
	m_tch->enable();
	// Q_EMIT sigpath()->requestRebuild();
}

void TimeChannelComponent::disable()
{
	ChannelComponent::disable();
	m_tch->disable();
	// Q_EMIT sigpath()->requestRebuild();
}

bool TimeChannelComponent::scaleAvailable() const { return m_scaleAvailable; }

bool TimeChannelComponent::yLock() const { return m_yLock; }

double TimeChannelComponent::yMin() const { return m_yCtrl->min(); }

double TimeChannelComponent::yMax() const { return m_yCtrl->max(); }

MeasureManagerInterface *TimeChannelComponent::getMeasureManager() { return m_measureMgr; }

SignalPath *TimeChannelComponent::sigpath()
{
	return m_tch->sigpath(); // This replaces return m_grtch->sigpath();
}

QVBoxLayout *TimeChannelComponent::menuLayout() { return m_layScroll; }

void TimeChannelComponent::onInit()
{
	// Defaults
	addChannelToPlot();

	m_yaxisMenu->setCollapsed(true);
	m_yCtrl->setMin(-1.0);
	m_yCtrl->setMax(1.0);

	m_ymode = static_cast<YMode>(-1);
	auto v = Preferences::get("adc_default_y_mode").toInt();
	m_ymodeCb->combo()->setCurrentIndex(v);
	setYMode(static_cast<YMode>(v));
}

void TimeChannelComponent::onDeinit() {}

void TimeChannelComponent::onNewData(const float *xData, const float *yData, size_t size, bool copy)
{
	// m_tch->onNewData(xData, yData, size, copy); // This replaces m_grtch->onNewData(...)
	auto model = m_measureMgr->getModel();
	model->setDataSource(yData, size);
	model->measure();
	m_snapBtn->setEnabled(true);
}

bool TimeChannelComponent::sampleRateAvailable() { return m_sourceBlock->sampleRateAvailable(m_sourceChannel); }

double TimeChannelComponent::sampleRate() { return m_sourceBlock->readSampleRate(); }

void TimeChannelComponent::setSamplingInfo(SamplingInfo p)
{
	ChannelComponent::setSamplingInfo(p);
	m_measureMgr->getModel()->setSampleRate(p.sampleRate);
}

YMode TimeChannelComponent::ymode() const { return m_ymode; }

void TimeChannelComponent::setYMode(YMode newYmode)
{
	if(m_ymode == newYmode)
		return;
	m_ymode = newYmode;
	setYModeHelper(newYmode);
	Q_EMIT yModeChanged();
}
