#ifndef SCALE_H
#define SCALE_H

#include <QComboBox>
#include <QList>
#include <QString>
#include <scopy-gui_export.h>
#include <QObject>

namespace scopy {
namespace gui {

class SCOPY_GUI_EXPORT UnitPrefix
{
public:
	QString prefix;
	double scale;
	// enum type - metric, hour, logarithmic, etc
};

class SCOPY_GUI_EXPORT ScaleOption
{
public:
	QString option;
	double scale;
	// enum type - metric, hour, logarithmic, etc
};

class SCOPY_GUI_EXPORT Scale : public QObject
{
	Q_OBJECT
public:
	Scale(QString unit, double min, double max, bool hasPrefix = true);
	~Scale();

	double getScaleForPrefix(QString prefix, Qt::CaseSensitivity s);
	double getScaleForUnit(QString unit, Qt::CaseSensitivity s);
	double getScaleForSymbol(QString symbol);

	QList<UnitPrefix> scalePrefixes() const;
	void setScalePrefixes(const QList<UnitPrefix> &newScalePrefixes);

	QList<ScaleOption> scaleOptions() const;
	void setScaleOptions(const QList<ScaleOption> &newScaleOptions);

	bool scalingEnabled() const;
	void setScalingEnabled(bool newScalingEnabled);

	QComboBox *scaleCb() const;

	QString unit() const;
	void setUnit(const QString &newUnit);

	void computeScale(double val);

	bool hasPrefix() const;
	void setHasPrefix(bool newHasPrefix);

Q_SIGNALS:
	void scaleUpdated();
	void unitChanged(QString unit);
	void scaleDown(int newScaleIndex);

private:
	bool m_hasPrefix;
	bool m_scalingEnabled = true;
	double m_min, m_max;
	QString m_unit;
	QList<ScaleOption> m_scaleOptions;
	QList<UnitPrefix> m_scalePrefixes;
	QComboBox *m_scaleCb;
	void populateScaleCb();
};

} // namespace gui
} // namespace scopy
#endif // SCALE_H
