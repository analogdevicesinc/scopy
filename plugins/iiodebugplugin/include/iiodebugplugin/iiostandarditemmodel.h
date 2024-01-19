#ifndef SCOPY_IIOSTANDARDITEMMODEL_H
#define SCOPY_IIOSTANDARDITEMMODEL_H

#include <QWidget>
#include "iiostandarditemtypes.h"

namespace scopy::iiodebugplugin {
class IIOStandardItemModel : public QWidget
{
	Q_OBJECT
public:
	explicit IIOStandardItemModel(IIOStandardItemType type, QWidget *parent = nullptr);

protected:
	IIOStandardItemType m_type;
	QStringList m_details;
};
} // namespace scopy::iiodebugplugin

#endif // SCOPY_IIOSTANDARDITEMMODEL_H
