#ifndef BITFIELDSIMPLEWIDGETFACTORY_HPP
#define BITFIELDSIMPLEWIDGETFACTORY_HPP

#include <QObject>

namespace scopy::regmap{
class BitFieldModel;

namespace gui {
class BitFieldSimpleWidget;

class BitFieldSimpleWidgetFactory : public QObject
{
	Q_OBJECT
public:
	explicit BitFieldSimpleWidgetFactory(QObject *parent = nullptr);

	BitFieldSimpleWidget *buildWidget(BitFieldModel *model, int streach);
Q_SIGNALS:

};
}
}
#endif // BITFIELDSIMPLEWIDGETFACTORY_HPP
