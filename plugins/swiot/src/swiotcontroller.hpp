#ifndef SWIOTCONTROLLER_HPP
#define SWIOTCONTROLLER_HPP

#include "qobject.h"
#include "swiotgenericmenu.hpp"
#include "swiotmodel.hpp"

namespace adiscope{


class SwiotController: public QObject{
	Q_OBJECT
public:
	explicit SwiotController(adiscope::gui::SwiotGenericMenu* genericMenu = nullptr, SwiotAdModel* model = nullptr);
	~SwiotController();

	void addMenuAttrValues();
	void createConnections();
public Q_SLOTS:
	void attributesChanged(QString attrName);

private:
	adiscope::gui::SwiotGenericMenu* m_genericMenu;
	SwiotAdModel* m_model;

};
}


#endif // SWIOTCONTROLLER_HPP
