#include "grtimechanneladdon.h"
#include "grdeviceaddon.h"
#include "griiofloatchannelsrc.h"
#include "grscaleoffsetproc.h"

#include <QDebug>

using namespace scopy::grutil;

GRTimeChannelAddon::GRTimeChannelAddon(QString ch, GRDeviceAddon *dev, GRTimePlotAddon *plotAddon, QPen pen, QObject *parent)
	: QObject(parent), m_plotAddon(plotAddon), m_pen(pen) {

	m_signalPath = new GRSignalPath("time_" + dev->getName() + ch, this);
	m_grch = new GRIIOFloatChannelSrc(dev->src(),ch,m_signalPath);
	m_signalPath->append(m_grch);
	auto scOff = new GRScaleOffsetProc(m_signalPath);
	m_signalPath->append(scOff);
	scOff->setOffset(0);
	scOff->setScale(1);
	m_signalPath->setEnabled(false);

	setDevice(dev);

	name = m_grch->getChannelName();
	widget = new QLabel(name);
	auto plot = plotAddon->plot();;

	m_plotAxis = new PlotAxis(QwtAxis::YLeft, plot, this);
	m_plotAxis->setInterval(-1<<11,1<<11);
//	m_plotAxis->setInterval(0,1<<24);
	m_plotCh = new PlotChannel(name, pen, plot, plot->xAxis(), m_plotAxis,this);
	m_plotAxisHandle = new PlotAxisHandle(pen, m_plotAxis,plot,this);
	m_plotCh->setHandle(m_plotAxisHandle);
	plot->addPlotAxisHandle(m_plotAxisHandle);

	// create menu widget
	// - create ADCCount/VFS/V
	// - create VDIV scale

	// - create thickness/render mode / Color (?)  / match to Curve
}

GRTimeChannelAddon::~GRTimeChannelAddon() {}

QString GRTimeChannelAddon::getName() {return name;}

QWidget *GRTimeChannelAddon::getWidget() {return widget;}

void GRTimeChannelAddon::setDevice(GRDeviceAddon *d) { m_dev = d; d->registerChannel(this);}

GRDeviceAddon* GRTimeChannelAddon::getDevice() { return m_dev;}

void GRTimeChannelAddon::enable() {
	qInfo()<<name<<" enabled";
	m_plotCh->attach();
	m_plotAxisHandle->handle()->setVisible(true);
	m_plotAxisHandle->handle()->raise();
	m_signalPath->setEnabled(true);
	m_grch->setEnabled(true);
}


void GRTimeChannelAddon::disable() {
	qInfo()<<name<<" disabled";
	m_plotCh->detach();
	m_plotAxisHandle->handle()->setVisible(false);
	m_signalPath->setEnabled(false);
	m_grch->setEnabled(false);
}

void GRTimeChannelAddon::onStart() {}

void GRTimeChannelAddon::onStop() {}

void GRTimeChannelAddon::onAdd() {  }

void GRTimeChannelAddon::onRemove() {}

void GRTimeChannelAddon::onChannelAdded(ToolAddon *) {}

void GRTimeChannelAddon::onChannelRemoved(ToolAddon *) {}

PlotChannel *GRTimeChannelAddon::plotCh() const
{
	return m_plotCh;
}

GRSignalPath *GRTimeChannelAddon::signalPath() const
{
	return m_signalPath;
}

QPen GRTimeChannelAddon::pen() const {
	return m_pen;
}
