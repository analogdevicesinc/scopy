#include"swiotcontroller.hpp"
#include "qdebug.h"

using namespace adiscope;

SwiotController::SwiotController(adiscope::gui::SwiotGenericMenu* genericMenu, SwiotAdModel* model):
	m_genericMenu(genericMenu)
      ,m_model(model)
{}

SwiotController::~SwiotController()
{}

void SwiotController::addMenuAttrValues()
{
	//get all the attributes from the iio_channel
	QMap<QString, QStringList> contextValues = m_model->getChnlAttrValues();
	if (!contextValues.empty()) {
		m_genericMenu->initAdvMenu(contextValues);
	}
}

void SwiotController::createConnections()
{
	adiscope::gui::SwiotAdvMenu* advMenu = m_genericMenu->getAdvMenu();
	connect(advMenu, &adiscope::gui::SwiotAdvMenu::attrValuesChanged,this,&SwiotController::attributesChanged);
}

void SwiotController::attributesChanged(QString attrName)
{
	adiscope::gui::SwiotAdvMenu* menu=m_genericMenu->getAdvMenu();
	QMap<QString, QStringList> attributes = menu->getAttrValues();
	m_model->updateChnlAttributes(attributes, attrName);

}


