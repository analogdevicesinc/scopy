#include "csvfilestrategy.h"
#include "dac_logging_categories.h"
#include "dacutils.h"

#include <QString>
#include <QVector>
#include <QFile>

using namespace scopy;
using namespace scopy::dac;
CSVFileStrategy::CSVFileStrategy(QString filename, QWidget *parent)
	: QObject(parent)
{
	m_filename = filename;
	m_separator = ",";
	m_recipe = {.scale = 0.0, .scaled = false};
}

QVector<QVector<short>> CSVFileStrategy::data()
{
	qDebug(CAT_DAC_DATASTRATEGY) << "Retrieve data.";
	return m_dataConverted;
}

void CSVFileStrategy::recipeUpdated(DataBufferRecipe recipe)
{
	qDebug(CAT_DAC_DATASTRATEGY) << "Recipe update in CSV file strategy.";
	m_recipe = recipe;
	applyConversion();
}

void CSVFileStrategy::loadData()
{
	double max = 0.0;
	QVector<QVector<QString>> raw_data;
	bool dataOk = true;
	QFile file(m_filename);
	if(!file.open(QIODevice::ReadOnly)) {
		qDebug(CAT_DAC_DATASTRATEGY) << "Can't open selected file";
		Q_EMIT loadFailed();
		return;
	}

	QTextStream in(&file);

	while(!in.atEnd()) {
		QVector<QString> line_data;
		QString line = in.readLine();
		if(line.isEmpty()) {
			continue;
		}
		QStringList list = line.split(m_separator, Qt::SkipEmptyParts);
		for(const QString &list_item : qAsConst(list)) {
			line_data.push_back(list_item);
		}
		if(line_data.size() > 0) {
			raw_data.push_back(line_data);
		}
	}

	// suppose no m_data in the file
	m_data.resize(raw_data.size());
	m_dataConverted.resize(raw_data.size());
	for(int i = 0; i < m_data.size(); ++i) {
		m_data[i].resize(raw_data[i].size());
		m_dataConverted[i].resize(raw_data[i].size());
	}

	for(int i = 0; i < raw_data.size(); ++i) {
		for(int j = 0; j < raw_data[i].size(); ++j) {
			double tmp = raw_data[i][j].toDouble(&dataOk);
			if(!dataOk) {
				qDebug(CAT_DAC_DATASTRATEGY) << "File is corrupted";
				Q_EMIT loadFailed();
				return;
			}
			m_data[i][j] = tmp;
			m_max = (tmp > m_max) ? tmp : m_max;
		}
	}
	applyConversion();
	Q_EMIT loadFinished();
}

void CSVFileStrategy::applyConversion()
{
	for(int i = 0; i < m_data.size(); i++) {
		for(int j = 0; j < m_data[i].size(); j++) {
			m_dataConverted[i][j] = convert(m_data[i][j]);
		}
	}
	Q_EMIT dataUpdated();
	qDebug(CAT_DAC_DATASTRATEGY) << "Apply conversion on all samples";
}

unsigned short CSVFileStrategy::convert(double value)
{
	double scale = 0.0;
	double offset = 0.0;
	double full_scale = DacUtils::dbFullScaleConvert(m_recipe.scale, false);
	if(!m_recipe.scaled) {
		scale = 16.0;
	}
	if(scale == 0.0) {
		scale = 32767.0 * full_scale / m_max;
	}

	return (value * scale + offset);
}
