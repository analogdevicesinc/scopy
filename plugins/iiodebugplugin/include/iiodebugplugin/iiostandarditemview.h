#ifndef SCOPY_IIOSTANDARDITEMVIEW_H
#define SCOPY_IIOSTANDARDITEMVIEW_H

#include <QWidget>
#include "iiostandarditemtypes.h"

namespace scopy::iiodebugplugin {
class IIOStandardItemView : public QWidget {
	Q_OBJECT
public:
	explicit IIOStandardItemView(IIOStandardItemType type, QWidget *parent = nullptr);

private:
	IIOStandardItemType m_type;
};
}

#endif // SCOPY_IIOSTANDARDITEMVIEW_H
