#include "bitfieldmodel.hpp"
#include "bitfieldsimplewidget.hpp"
#include "bitfieldsimplewidgetfactory.hpp"

using namespace scopy;
using namespace regmap;
using namespace regmap::gui;

BitFieldSimpleWidgetFactory::BitFieldSimpleWidgetFactory(QObject *parent)
	: QObject{parent}
{

}

BitFieldSimpleWidget *BitFieldSimpleWidgetFactory::buildWidget(BitFieldModel *model, int streach)
{
	return new BitFieldSimpleWidget(model->getName(),
					model->getDefaultValue(),
					model->getDescription(),
					model->getWidth(),
					model->getNotes(),
					model->getRegOffset(),
					streach);
}
