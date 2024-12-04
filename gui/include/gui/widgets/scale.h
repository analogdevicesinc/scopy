/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

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

	Q_PROPERTY(QString unit READ unit WRITE setUnit NOTIFY unitChanged)
};

} // namespace gui
} // namespace scopy
#endif // SCALE_H
