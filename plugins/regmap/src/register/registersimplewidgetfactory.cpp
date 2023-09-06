#include "bitfield/bitfieldmodel.hpp"
#include "bitfield/bitfieldsimplewidget.hpp"
#include "bitfield/bitfieldsimplewidgetfactory.hpp"
#include "registermodel.hpp"
#include "registersimplewidget.hpp"
#include "registersimplewidgetfactory.hpp"

#include <regmapstylehelper.hpp>

using namespace scopy;
using namespace regmap;

RegisterSimpleWidgetFactory::RegisterSimpleWidgetFactory(QObject *parent)
	: QObject{parent}
{ }

RegisterSimpleWidget *RegisterSimpleWidgetFactory::buildWidget(RegisterModel *model)
{
	QVector<BitFieldSimpleWidget*> *bitFields = new QVector<BitFieldSimpleWidget*>;

	BitFieldSimpleWidgetFactory bitFieldSimpleWidgetFactory;
	int remaingSpaceOnRow = 8;
	for (int i = 0 ; i < model->getBitFields()->size(); ++i) {
		BitFieldModel *modelBitField = model->getBitFields()->at(i);
		int width = modelBitField->getWidth();
		while (width > remaingSpaceOnRow) {
			bitFields->push_back(bitFieldSimpleWidgetFactory.buildWidget(model->getBitFields()->at(i), remaingSpaceOnRow));
			width = width - remaingSpaceOnRow;
			remaingSpaceOnRow = 8;
		}

		bitFields->push_back(bitFieldSimpleWidgetFactory.buildWidget(model->getBitFields()->at(i), width));
		remaingSpaceOnRow-= width;
		if (remaingSpaceOnRow == 0 ) {
			remaingSpaceOnRow = 8;
		}
	}

    RegisterSimpleWidget *rsw = new RegisterSimpleWidget(model->getName(),
					QString::number(model->getAddress(),16),
					model->getDescription(),
					model->getNotes(),
					model->getWidth(),
                    bitFields) ;

    rsw->setStyleSheet(RegmapStyleHelper::simpleRegisterStyle(rsw, "rsw") + RegmapStyleHelper::frameBorderHover(rsw, "rsw")  );
    return rsw;
}

