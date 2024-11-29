#include "buffermenucontroller.hpp"
#include "qdebug.h"

using namespace scopy::swiot;

BufferMenuController::BufferMenuController(BufferMenuView* genericMenu, BufferMenuModel* model, int chnlIdx):
	m_chnlIdx(chnlIdx)
      ,m_genericMenu(genericMenu)
      ,m_model(model)
{}

BufferMenuController::~BufferMenuController()
{}

void BufferMenuController::addMenuAttrValues()
{
	//get all the attributes from the iio_channel
	QMap<QString, QStringList> contextValues = m_model->getChnlAttrValues();
	if (!contextValues.empty()) {
		m_genericMenu->initAdvMenu(contextValues);
	}
}

void BufferMenuController::createConnections()
{
	BufferMenu* advMenu = m_genericMenu->getAdvMenu();
	connect(advMenu, &BufferMenu::attrValuesChanged, this, &BufferMenuController::attributesChanged);
}

void BufferMenuController::attributesChanged(QString attrName)
{
	BufferMenu* menu=m_genericMenu->getAdvMenu();
	QMap<QString, QStringList> attributes = menu->getAttrValues();
	m_model->updateChnlAttributes(attributes, attrName);

}

int BufferMenuController::getChnlIdx()
{
	return m_chnlIdx;
}

