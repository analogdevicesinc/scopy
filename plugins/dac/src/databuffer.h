#ifndef DATABUFFER_H
#define DATABUFFER_H
#include "scopy-dac_export.h"
#include "databufferstrategyinterface.h"
#include "dataguistrategyinterface.h"

#include <QWidget>
namespace scopy {
namespace dac {
class SCOPY_DAC_EXPORT DataBuffer : public QObject
{
	Q_OBJECT
public:
	explicit DataBuffer(DataGuiStrategyInterface *guids, DataBufferStrategyInterface *ds,
			    QWidget *parent = nullptr);
	~DataBuffer();
	DataBufferStrategyInterface *getDataBufferStrategy();
	DataGuiStrategyInterface *getDataGuiStrategyInterface();

	QWidget *getParent();

public Q_SLOTS:
	void loadData();

Q_SIGNALS:
	void loadFinished();
	void loadFailed();
	void dataUpdated();

private:
	QWidget *m_parent;
	DataBufferStrategyInterface *m_dataStrategy;
	DataGuiStrategyInterface *m_guiStrategy;
};
} // namespace dac
} // namespace scopy
#endif // DATABUFFER_H
