#ifndef PLOTCURSORREADOUTS_H
#define PLOTCURSORREADOUTS_H

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

#include <plot_utils.hpp>
#include <scopy-gui_export.h>
#include <stylehelper.h>

namespace scopy {
class SCOPY_GUI_EXPORT PlotCursorReadouts : public QWidget
{
	Q_OBJECT
public:
	PlotCursorReadouts(QWidget *parent = nullptr);
	~PlotCursorReadouts();

	bool isXVisible();
	bool isYVisible();

public Q_SLOTS:
	void setY1(double val);
	void setY2(double val);
	void setX1(double val);
	void setX2(double val);
	void setXVisible(bool visible);
	void setYVisible(bool visible);
	void update();

	void setYUnits(QString unit);
	void setXUnits(QString unit);

	void setXFormatter(PrefixFormatter *formatter);
	void setYFormatter(PrefixFormatter *formatter);

private:
	QWidget *y_contents;
	QWidget *x_contents;
	double y1, y2, x1, x2;
	QLabel *y1_val, *y2_val, *x1_val, *x2_val;
	QLabel *deltaY_val, *deltaX_val;
	QLabel *invDeltaX_val;
	PrefixFormatter *xFormatter, *yFormatter;
	QString xUnit, yUnit;

	void initContent();
};
} // namespace scopy

#endif // PLOTCURSORREADOUTS_H
