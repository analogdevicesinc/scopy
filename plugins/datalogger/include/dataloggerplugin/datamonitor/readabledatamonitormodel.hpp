#ifndef READABLEDATAMONITORMODEL_HPP
#define READABLEDATAMONITORMODEL_HPP

#include <QObject>
#include "../scopy-dataloggerplugin_export.h"
#include "readstrategy/ireadstrategy.hpp"
#include "datamonitormodel.hpp"

namespace scopy {
namespace datamonitor {
class SCOPY_DATALOGGERPLUGIN_EXPORT ReadableDataMonitorModel : public DataMonitorModel
{
	Q_OBJECT
public:
	ReadableDataMonitorModel(QObject *parent = nullptr);
	explicit ReadableDataMonitorModel(QString name, QColor color, UnitOfMeasurement *unitOfMeasure = nullptr,
					  IReadStrategy *readStrategy = nullptr, QObject *parent = nullptr);

	void read();
	IReadStrategy *readStrategy() const;
	void setReadStrategy(IReadStrategy *newReadStrategy);
	void resetMinMax();

signals:

private:
	IReadStrategy *m_readStrategy;
};
} // namespace datamonitor
} // namespace scopy
#endif // READABLEDATAMONITORMODEL_HPP
