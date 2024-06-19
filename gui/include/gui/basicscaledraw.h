#ifndef BASICSCALEDRAW_H
#define BASICSCALEDRAW_H

#include "scopy-gui_export.h"
#include <QColor>
#include <QwtScaleDraw>
#include "plot_utils.hpp"

namespace scopy {
/*
 * BasicScaleDraw class overrides the way the major values are being displayed.
 */
class SCOPY_GUI_EXPORT BasicScaleDraw : public QwtScaleDraw
{
public:
	BasicScaleDraw(const QString &unit_type = "");
	BasicScaleDraw(PrefixFormatter *, const QString &);
	QwtText label(double) const;

	void setFloatPrecision(unsigned int numDigits);
	unsigned int getFloatPrecison() const;

	void setUnitType(const QString &unit);
	QString getUnitType() const;

	void setColor(QColor color);

	void setDisplayScale(double value);
	void setFormatter(PrefixFormatter *formatter);

	void enableDeltaLabel(bool enable);
	void setUnitsEnabled(bool enable);

protected:
	virtual void draw(QPainter *, const QPalette &) const;

private:
	int m_floatPrecision;
	QString m_unit;
	PrefixFormatter *m_formatter;
	QColor m_color;
	double m_displayScale;
	mutable unsigned int m_nrTicks;
	mutable bool m_shouldDrawMiddleDelta;
	bool m_delta;
	bool m_unitsEn;
};
} // namespace scopy

#endif // BASICSCALEDRAW_H
