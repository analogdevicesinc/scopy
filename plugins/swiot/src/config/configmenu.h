#ifndef CONFIGMENU_H
#define CONFIGMENU_H

#include <QObject>
#include "ui_swiotconfig.h"

namespace scopy::swiot {
class ConfigMenu: public QWidget
{
	Q_OBJECT
public:
	explicit ConfigMenu(QWidget* parent = nullptr);
	~ConfigMenu();

	void addWidget(QWidget* widget);
private:
	Ui::ConfigMenu* m_ui;
};

}

#endif // CONFIGMENU_H
