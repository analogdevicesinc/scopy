#ifndef SCOPY_IIOSTANDARDITEMMODEL_H
#define SCOPY_IIOSTANDARDITEMMODEL_H

#include <QWidget>

namespace scopy::iiodebugplugin {
class IIOStandardItemModel : public QWidget
{
	Q_OBJECT
public:
	enum IIOType
	{
		Undefined = 1000,
		Context = 1001,
		Device = 1002,
		Channel = 1003,
		ContextAttribute = 1004,
		DeviceAttribute = 1005,
		ChannelAttribute = 1006,
	};

	explicit IIOStandardItemModel(IIOType type, QWidget *parent = nullptr);

protected:
	IIOStandardItemModel::IIOType m_type;
	QStringList m_details;
};
} // namespace scopy::iiodebugplugin

#endif // SCOPY_IIOSTANDARDITEMMODEL_H
