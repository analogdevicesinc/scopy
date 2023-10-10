#ifndef REGISTERSIMPLEWIDGETFACTORY_HPP
#define REGISTERSIMPLEWIDGETFACTORY_HPP

#include <QObject>

namespace scopy::regmap {
class RegisterModel;
class RegisterSimpleWidget;

class RegisterSimpleWidgetFactory : public QObject
{
	Q_OBJECT
public:
	explicit RegisterSimpleWidgetFactory(QObject *parent = nullptr);

	RegisterSimpleWidget *buildWidget(RegisterModel *model);
Q_SIGNALS:
};
} // namespace scopy::regmap
#endif // REGISTERSIMPLEWIDGETFACTORY_HPP
