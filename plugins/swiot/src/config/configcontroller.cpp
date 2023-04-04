#include "configcontroller.hpp"
#include "qdebug.h"

using namespace adiscope::swiot;

ConfigController::ConfigController(ConfigChannelView* chnlsView,
                                   ConfigModel* model):
	m_chnlsView(chnlsView)
      ,m_model(model)
{
	createConnections();
}

ConfigController::~ConfigController()
{
	if (m_chnlsView) {
		delete m_chnlsView;
		m_chnlsView = nullptr;
	}

	if (m_model) {
		delete m_model;
		m_model = nullptr;
	}
}

void ConfigController::addMenuAttrValues()
{
	//get all the attributes from the iio_channel
	QVector<QMap<QString, QStringList>> chnlsValues = m_model->getChnlsAttrValues();

	if (!chnlsValues.empty()) {
		m_chnlsView->setChnlsAttr(chnlsValues);
	}
}

void ConfigController::createConnections()
{
	connect(m_chnlsView, &adiscope::swiot::ConfigChannelView::attrValueChanged, this, &ConfigController::attrChanged);
}

void ConfigController::attrChanged(QString attrName, int deviceIdx)
{
	QVector<QMap<QString, QStringList>> values = m_chnlsView->getChnlsAttr();
	m_model->updateChnlAttributes(values, attrName, deviceIdx);
}

QStringList ConfigController::getActiveFunctions()
{
	return m_model->getActiveFunctions();
}
