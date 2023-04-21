#include "bitfielddetailedwidgetfactory.hpp"

#include "bitfielddetailedwidget.hpp"
#include "bitfieldmodel.hpp"

BitFieldDetailedWidgetFactory::BitFieldDetailedWidgetFactory(QObject *parent)
	: QObject{parent}
{

}

BitFieldDetailedWidget *BitFieldDetailedWidgetFactory::buildWidget(BitFieldModel *model)
{
	return new BitFieldDetailedWidget(model->getName(),
					  model->getDefaultValue(),
					  model->getDescription(),
					  model->getWidth(),
					  model->getNotes(),
					  model->getRegOffset(),
					  model->getOptions());
}
