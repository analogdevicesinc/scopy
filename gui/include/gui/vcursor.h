#ifndef VCURSOR_H
#define VCURSOR_H
#include "plotaxis.h"
#include "plotwidget.h"
#include "scopy-gui_export.h"

#include <QObject>

#include <cursor.h>

namespace scopy {

class SCOPY_GUI_EXPORT VCursor : public Cursor
{
	Q_OBJECT
public:
	explicit VCursor(PlotWidget *p, PlotAxis *ax, bool right = true,
			 QPen pen = QPen(QColor(155, 155, 155), 1, Qt::DashLine));
	~VCursor();
	;

	double computePosition(int pos);
	double getPosition();
	void setPosition(double pos);
	void setCanLeavePlot(bool leave) override;

Q_SIGNALS:
	void update();
};
} // namespace scopy

#endif // VCURSOR_H
