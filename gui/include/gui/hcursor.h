#ifndef HCURSOR_H
#define HCURSOR_H
#include "plotaxis.h"
#include "plotwidget.h"
#include "scopy-gui_export.h"

#include <QObject>

#include <cursor.h>

namespace scopy {

class SCOPY_GUI_EXPORT HCursor : public Cursor
{
	Q_OBJECT
public:
	explicit HCursor(PlotWidget *p, PlotAxis *ax, bool bottom = true,
			 QPen pen = QPen(QColor(155, 155, 155), 1, Qt::DashLine));
	~HCursor();

	double computePosition(int pos);
	double getPosition();
	void setPosition(double pos);
	void setCanLeavePlot(bool leave) override;

Q_SIGNALS:
	void update();
};
} // namespace scopy

#endif // HCURSOR_H
