#ifndef CUSTOMQWTSCALEDRAW_HPP
#define CUSTOMQWTSCALEDRAW_HPP

#include <qwt_text.h>
#include <qwt_interval.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_engine.h>
#include "plot_utils.hpp"
#include "scopy-gui_export.h"

namespace scopy {

class SCOPY_GUI_EXPORT CustomQwtScaleDraw : public QwtScaleDraw
{
public:
	CustomQwtScaleDraw();
	~CustomQwtScaleDraw();
	void setUnitOfMeasure(QString unitOfMeasure);

protected:
	virtual QwtText label(double value) const;

private:
	QString m_unitOfMeasure;

};
}
#endif // CUSTOMQWTSCALEDRAW_HPP
