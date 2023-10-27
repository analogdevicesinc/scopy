#include "bitfielddetailedwidgetfactory.hpp"

#include "bitfielddetailedwidget.hpp"
#include "bitfieldmodel.hpp"

#include <regmapstylehelper.hpp>

using namespace scopy;
using namespace regmap;

BitFieldDetailedWidgetFactory::BitFieldDetailedWidgetFactory(QObject *parent)
	: QObject{parent}
{}

BitFieldDetailedWidget *BitFieldDetailedWidgetFactory::buildWidget(BitFieldModel *model)
{
	BitFieldDetailedWidget *bfdw = new BitFieldDetailedWidget(
		model->getName(), model->getAccess(), model->getDefaultValue(), model->getDescription(),
		model->getWidth(), model->getNotes(), model->getRegOffset(), model->getOptions());
	RegmapStyleHelper::BitFieldDetailedWidgetStyle(bfdw);
	return bfdw;
}
