#ifndef REGISTERSIMPLEWIDGETFACTORY_HPP
#define REGISTERSIMPLEWIDGETFACTORY_HPP

#include <QObject>

class RegisterSimpleWidget;
class RegisterModel;
class RegisterSimpleWidgetFactory : public QObject
{
	Q_OBJECT
public:
	explicit RegisterSimpleWidgetFactory(QObject *parent = nullptr);

	RegisterSimpleWidget *buildWidget(RegisterModel *model);
Q_SIGNALS:

};

#endif // REGISTERSIMPLEWIDGETFACTORY_HPP
