#ifndef SWIOTCONTROLLER_HPP
#define SWIOTCONTROLLER_HPP

#include "qobject.h"
#include "buffermenuview.hpp"
#include "buffermenumodel.hpp"

namespace scopy::swiot {
class BufferMenuController : public QObject {
	Q_OBJECT
public:
	explicit BufferMenuController(BufferMenuView *genericMenu = nullptr,
				      BufferMenuModel *model = nullptr, int chnlIdx = -1);

	~BufferMenuController();

	void addMenuAttrValues();

	void createConnections();

	int getChnlIdx();

public Q_SLOTS:

	void attributesChanged(QString attrName);

private:
	int m_chnlIdx;
	BufferMenuView *m_genericMenu;
	BufferMenuModel *m_model;

};
}


#endif // SWIOTCONTROLLER_HPP
