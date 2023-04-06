#include "swiotconfigcontroller.hpp"
#include "qdebug.h"

using namespace adiscope;

SwiotConfigController::SwiotConfigController(gui::SwiotConfigChnlView* chnlsView,
					     SwiotConfigModel* model):
	m_chnlsView(chnlsView)
      ,m_model(model)
{
	createConnections();
}

SwiotConfigController::~SwiotConfigController()
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

void SwiotConfigController::addMenuAttrValues()
{
	//get all the attributes from the iio_channel
	QVector<QMap<QString, QStringList>> chnlsValues = m_model->getChnlsAttrValues();

	if (!chnlsValues.empty()) {
		m_chnlsView->setChnlsAttr(chnlsValues);
	}
}

void SwiotConfigController::createConnections()
{
	connect(m_chnlsView, &adiscope::gui::SwiotConfigChnlView::attrValueChanged, this, &SwiotConfigController::attrChanged);
}

void SwiotConfigController::attrChanged(QString attrName, int deviceIdx)
{
	QVector<QMap<QString, QStringList>> values = m_chnlsView->getChnlsAttr();
	m_model->updateChnlAttributes(values, attrName, deviceIdx);
}

QStringList SwiotConfigController::getActiveFunctions()
{
	return m_model->getActiveFunctions();
}
