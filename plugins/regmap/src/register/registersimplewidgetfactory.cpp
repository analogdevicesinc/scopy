#include "registersimplewidgetfactory.hpp"

#include "bitfield/bitfieldmodel.hpp"
#include "bitfield/bitfieldsimplewidget.hpp"
#include "bitfield/bitfieldsimplewidgetfactory.hpp"
#include "registermodel.hpp"
#include "registersimplewidget.hpp"
#include "registermaptemplate.hpp"

#include <regmapstylehelper.hpp>
#include <utils.hpp>

using namespace scopy;
using namespace regmap;

RegisterSimpleWidgetFactory::RegisterSimpleWidgetFactory(QObject *parent)
	: QObject{parent}
{}

RegisterSimpleWidget *RegisterSimpleWidgetFactory::buildWidget(RegisterModel *model)
{
	QVector<BitFieldSimpleWidget *> *bitFields = new QVector<BitFieldSimpleWidget *>;

	BitFieldSimpleWidgetFactory bitFieldSimpleWidgetFactory;
	int remaingSpaceOnRow = model->registerMapTemaplate()->bitsPerRow();
	for(int i = 0; i < model->getBitFields()->size(); ++i) {
		BitFieldModel *modelBitField = model->getBitFields()->at(i);
		int width = modelBitField->getWidth();
		while(width > remaingSpaceOnRow) {
			bitFields->push_back(bitFieldSimpleWidgetFactory.buildWidget(model->getBitFields()->at(i),
										     remaingSpaceOnRow));
			width = width - remaingSpaceOnRow;
			remaingSpaceOnRow = model->registerMapTemaplate()->bitsPerRow();
		}

		bitFields->push_back(bitFieldSimpleWidgetFactory.buildWidget(model->getBitFields()->at(i), width));
		remaingSpaceOnRow -= width;
		if(remaingSpaceOnRow == 0) {
			remaingSpaceOnRow = model->registerMapTemaplate()->bitsPerRow();
		}
	}

	RegisterSimpleWidget *rsw = new RegisterSimpleWidget(model, bitFields);

	RegmapStyleHelper::RegisterSimpleWidgetStyle(rsw);
	return rsw;
}
