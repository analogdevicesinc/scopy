#ifndef VERTICALSCROLLAREA_HPP
#define VERTICALSCROLLAREA_HPP

#include "scopy-regmap_export.h"

#include <qscrollarea.h>

class SCOPY_REGMAP_EXPORT VerticalScrollArea : public QScrollArea
{
	Q_OBJECT
public:
	explicit VerticalScrollArea(QWidget *parent = 0);
	virtual bool eventFilter(QObject *o, QEvent *e);
};

#endif // VERTICALSCROLLAREA_HPP
