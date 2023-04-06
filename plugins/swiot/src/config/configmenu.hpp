#ifndef SWIOTCONFIGMENU_HPP
#define SWIOTCONFIGMENU_HPP

#include <QObject>
#include "ui_swiotconfig.h"

namespace adiscope::swiot {
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

#endif // SWIOTCONFIGMENU_HPP
