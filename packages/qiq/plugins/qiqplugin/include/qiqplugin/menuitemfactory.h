#ifndef MENUITEMFACTORY_H
#define MENUITEMFACTORY_H

#include <QWidget>
#include <QVariantMap>
#include <QString>

namespace scopy::qiqplugin {

class MenuItemFactory : public QObject
{
	Q_OBJECT

public:
	MenuItemFactory(QObject *parent = nullptr);
	~MenuItemFactory();

	QWidget *createWidget(const QString &field, const QVariantMap &params);
	QVariantMap toVariantMap();

Q_SIGNALS:
	void itemParamChanged(const QString &field, const QVariant &value);

private:
	void onItemParamChanged(const QString &field, const QVariant &value);

	QWidget *createSpinBox(const QString &field, const QVariantMap &params);
	QWidget *createLineEdit(const QString &field, const QVariantMap &params);
	QWidget *createCombo(const QString &field, const QVariantMap &params);
	QWidget *createCheckBox(const QString &field, const QVariantMap &params);

	QVariantMap m_currentParams;
};

} // namespace scopy::qiqplugin

#endif // MENUITEMFACTORY_H
