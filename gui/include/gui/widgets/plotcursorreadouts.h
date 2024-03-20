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

	bool horizIsVisible();
	bool vertIsVisible();

public Q_SLOTS:
	void setV1(double val);
	void setV2(double val);
	void setH1(double val);
	void setH2(double val);
	void horizSetVisible(bool visible);
	void vertSetVisible(bool visible);
	void update();

	void setVertUnits(QString unit);
	void setHorizUnits(QString unit);

	void setHorizFromatter(PrefixFormatter *formatter);
	void setVertFromatter(PrefixFormatter *formatter);

private:
	QWidget *vert_contents;
	QWidget *horiz_contents;
	double v1, v2, h1, h2;
	QLabel *V1_val, *V2_val, *H1_val, *H2_val;
	QLabel *deltaV_val, *deltaH_val;
	QLabel *invDeltaH_val;
	PrefixFormatter *hFormatter, *vFormatter;
	QString hUnit, vUnit;

	void initContent();
};
} // namespace scopy

#endif // PLOTCURSORREADOUTS_H
