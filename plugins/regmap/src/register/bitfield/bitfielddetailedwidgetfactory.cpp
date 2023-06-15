#include "bitfielddetailedwidgetfactory.hpp"

#include "bitfielddetailedwidget.hpp"
#include "bitfieldmodel.hpp"

using namespace scopy;
using namespace regmap;
using namespace regmap::gui;

BitFieldDetailedWidgetFactory::BitFieldDetailedWidgetFactory(QObject *parent)
	: QObject{parent}
{

}

BitFieldDetailedWidget *BitFieldDetailedWidgetFactory::buildWidget(BitFieldModel *model)
{
    return new BitFieldDetailedWidget(model->getName(),
                                      model->getAccess(),
                                      model->getDefaultValue(),
                                      model->getDescription(),
                                      model->getWidth(),
                                      model->getNotes(),
                                      model->getRegOffset(),
                                      model->getOptions());
}
