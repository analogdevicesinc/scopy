#ifndef ANALYSISMENU_H
#define ANALYSISMENU_H

#include <QWidget>
#include <QVariantMap>
#include "menuitemfactory.h"

namespace scopy::qiqplugin {
class AnalysisMenu : public QWidget
{
	Q_OBJECT

public:
	AnalysisMenu(QWidget *parent = nullptr);
	~AnalysisMenu();

	QVariantMap getAnalysisConfig();
	void createMenu(const QVariantMap &params);

public Q_SLOTS:
	void updateAnalysisConfig(const QString &field, const QVariant &value);

Q_SIGNALS:
	void applyPressed();

private:
	MenuItemFactory *m_factory;
	QVariantMap m_analysisConfig;

	void setupUI();
};
} // namespace scopy::qiqplugin

#endif // ANALYSISMENU_H
