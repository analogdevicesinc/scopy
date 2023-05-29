#include "configcontroller.h"
#include "qdebug.h"

using namespace scopy::swiot;

ConfigController::ConfigController(ConfigChannelView *channelView,
				   ConfigModel *model,
				   int channelId) :
	m_channelsView(channelView), m_model(model), m_channelId(channelId)
{
	this->initChannelView();
	this->connectSignalsAndSlots();
}

ConfigController::~ConfigController() {
	if (m_channelsView) {
		delete m_channelsView;
		m_channelsView = nullptr;
	}

	if (m_model) {
		delete m_model;
		m_model = nullptr;
	}
}

void ConfigController::connectSignalsAndSlots() {
	QObject::connect(m_channelsView, &ConfigChannelView::enabledChanged, this, [this] (int index, bool value) {
		if (m_channelId == index) {
			if (value) {
				m_model->writeEnabled("1");
			} else {
				m_model->writeEnabled("0");
			}
		}
	});

	QObject::connect(m_channelsView, &ConfigChannelView::deviceChanged, this, [this] (int index, const QString& device) {
		if (m_channelId == index) {
			m_model->writeDevice(device);

			QStringList functionsAvailable = m_model->readFunctionAvailable();
			m_channelsView->setFunctionAvailable(functionsAvailable);

			QString selectedFunction = m_model->readFunction();
			m_channelsView->setSelectedFunction(selectedFunction);
		}
	});

	QObject::connect(m_channelsView, &ConfigChannelView::functionChanged, this, [this] (int index, const QString& function) {
		if (m_channelId == index) {
			m_model->writeFunction(function);
		}
	});
}

void ConfigController::initChannelView() {
	QString enabled = m_model->readEnabled();
	m_channelsView->setChannelEnabled((enabled == "1"));

	QStringList devicesAvailable = m_model->readDeviceAvailable();
	m_channelsView->setDeviceAvailable(devicesAvailable);

	QString selectedDevice = m_model->readDevice();
	m_channelsView->setSelectedDevice(selectedDevice);

	QStringList functionsAvailable = m_model->readFunctionAvailable();
	m_channelsView->setFunctionAvailable(functionsAvailable);

	QString selectedFunction = m_model->readFunction();
	m_channelsView->setSelectedFunction(selectedFunction);
}
