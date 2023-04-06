#ifndef SWIOTCONTROLLER_HPP
#define SWIOTCONTROLLER_HPP

#include "qobject.h"
#include "swiotgenericmenu.hpp"
#include "swiotmodel.hpp"

namespace adiscope{


class SwiotController: public QObject{
	Q_OBJECT
public:
	explicit SwiotController(adiscope::gui::SwiotGenericMenu* genericMenu = nullptr, SwiotAdModel* model = nullptr, int chnlIdx = -1);
	~SwiotController();

	void addMenuAttrValues();
	void createConnections();

	int getChnlIdx();
public Q_SLOTS:
	void attributesChanged(QString attrName);

private:
	int m_chnlIdx;
	adiscope::gui::SwiotGenericMenu* m_genericMenu;
	SwiotAdModel* m_model;

};
}


#endif // SWIOTCONTROLLER_HPP
