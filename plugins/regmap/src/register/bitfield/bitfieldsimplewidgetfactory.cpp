#include "bitfieldsimplewidgetfactory.hpp"

#include "bitfieldmodel.hpp"
#include "bitfieldsimplewidget.hpp"

#include <regmapstylehelper.hpp>

using namespace scopy;
using namespace regmap;

BitFieldSimpleWidgetFactory::BitFieldSimpleWidgetFactory(QObject *parent)
	: QObject{parent}
{}

BitFieldSimpleWidget *BitFieldSimpleWidgetFactory::buildWidget(BitFieldModel *model, int streach)
{
	BitFieldSimpleWidget *bfsw =
		new BitFieldSimpleWidget(model->getName(), model->getDefaultValue(), model->getDescription(),
					 model->getWidth(), model->getNotes(), model->getRegOffset(), streach);

	RegmapStyleHelper::BitFieldSimpleWidgetStyle(bfsw);

	return bfsw;
}
