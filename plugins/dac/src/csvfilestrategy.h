#ifndef CSVFILESTRATEGY_H
#define CSVFILESTRATEGY_H

#include <QWidget>
#include "databufferstrategyinterface.h"
#include "scopy-dac_export.h"
#include "dac_logging_categories.h"
namespace scopy {
namespace dac {
class SCOPY_DAC_EXPORT CSVFileStrategy : public QObject, public DataBufferStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::dac::DataBufferStrategyInterface)
public:
	explicit CSVFileStrategy(QString filename, QWidget *parent = nullptr);
	~CSVFileStrategy(){};
	QVector<QVector<short>> data() override;

public Q_SLOTS:
	void recipeUpdated(DataBufferRecipe) override;
	void loadData() override;

Q_SIGNALS:
	void loadFinished() override;
	void loadFailed() override;
	void dataUpdated() override;

private:
	double m_max;
	QString m_filename;
	QString m_separator;
	QVector<QVector<double>> m_data;
	QVector<QVector<short>> m_dataConverted;
	DataBufferRecipe m_recipe;
	unsigned short convert(double value);
	void applyConversion();
};
} // namespace dac
} // namespace scopy
#endif // CSVFILESTRATEGY_H
