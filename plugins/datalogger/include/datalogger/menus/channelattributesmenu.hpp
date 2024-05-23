#ifndef CHANNELATTRIBUTESMENU_HPP
#define CHANNELATTRIBUTESMENU_HPP

#include <QBoxLayout>
#include <QWidget>
#include <datamonitor/dmmdatamonitormodel.hpp>
#include "scopy-datalogger_export.h"

namespace scopy {
namespace datamonitor {
class SCOPY_DATALOGGER_EXPORT ChannelAttributesMenu : public QWidget
{
	Q_OBJECT
public:
	explicit ChannelAttributesMenu(DataMonitorModel *model, QWidget *parent = nullptr);
};
} // namespace datamonitor
} // namespace scopy
#endif // CHANNELATTRIBUTESMENU_HPP
