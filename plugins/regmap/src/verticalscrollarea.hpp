#ifndef VERTICALSCROLLAREA_HPP
#define VERTICALSCROLLAREA_HPP

#include <qscrollarea.h>
#include "scopyregmap_export.h"

class SCOPYREGMAP_EXPORT VerticalScrollArea: public QScrollArea
{
    Q_OBJECT
public:
   explicit VerticalScrollArea(QWidget *parent = 0);
    virtual bool eventFilter(QObject *o, QEvent *e);
};

#endif // VERTICALSCROLLAREA_HPP
