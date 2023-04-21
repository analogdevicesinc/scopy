#ifndef BUFFERMENUCONTROLLER_H
#define BUFFERMENUCONTROLLER_H

#include "qobject.h"
#include "buffermenuview.h"
#include "buffermenumodel.h"

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


#endif // BUFFERMENUCONTROLLER_H
