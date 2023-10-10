#ifndef BITFIELDSIMPLEWIDGETFACTORY_HPP
#define BITFIELDSIMPLEWIDGETFACTORY_HPP

#include <QObject>

namespace scopy::regmap {
class BitFieldModel;
class BitFieldSimpleWidget;

class BitFieldSimpleWidgetFactory : public QObject
{
	Q_OBJECT
public:
	explicit BitFieldSimpleWidgetFactory(QObject *parent = nullptr);

	BitFieldSimpleWidget *buildWidget(BitFieldModel *model, int streach);
Q_SIGNALS:
};
} // namespace scopy::regmap
#endif // BITFIELDSIMPLEWIDGETFACTORY_HPP
