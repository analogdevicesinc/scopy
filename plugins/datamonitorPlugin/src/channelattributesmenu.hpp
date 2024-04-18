#ifndef CHANNELATTRIBUTESMENU_HPP
#define CHANNELATTRIBUTESMENU_HPP

#include <QBoxLayout>
#include <QWidget>
#include <dmmdatamonitormodel.hpp>
#include "scopy-datamonitorplugin_export.h"

namespace scopy {
namespace datamonitor {
class SCOPY_DATAMONITORPLUGIN_EXPORT ChannelAttributesMenu : public QWidget
{
	Q_OBJECT
public:
	explicit ChannelAttributesMenu(DataMonitorModel *model, QWidget *parent = nullptr);
};
} // namespace datamonitor
} // namespace scopy
#endif // CHANNELATTRIBUTESMENU_HPP
