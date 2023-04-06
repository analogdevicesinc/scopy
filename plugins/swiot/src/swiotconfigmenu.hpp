#ifndef SWIOTCONFIGMENU_HPP
#define SWIOTCONFIGMENU_HPP

#include <QObject>
#include "ui_swiotconfig.h"

namespace adiscope{
class SwiotConfigMenu: public QWidget
{
	Q_OBJECT
public:
	SwiotConfigMenu(QWidget* parent = nullptr);
	~SwiotConfigMenu();

	void addWidget(QWidget* widget);
private:
	Ui::SwiotConfigMenu* m_ui;
};

}

#endif // SWIOTCONFIGMENU_HPP
