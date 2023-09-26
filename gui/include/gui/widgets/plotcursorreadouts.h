#ifndef PLOTCURSORREADOUTS_H
#define PLOTCURSORREADOUTS_H

#include <QWidget>
#include <QVBoxLayout>
#include <scopy-gui_export.h>
#include <plot_utils.hpp>
#include <QLabel>
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

private:
	QWidget *vert_contents;
	QWidget *horiz_contents;
	double v1,v2,h1,h2;
	QLabel *V1_val,*V2_val,*H1_val,*H2_val;
	QLabel *deltaV_val, *deltaH_val;
	QLabel *invDeltaH_val;
	PrefixFormatter *formatter;

	void initContent();
};
}

#endif // PLOTCURSORREADOUTS_H
