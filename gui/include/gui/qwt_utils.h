#ifndef QWT_UTILS_H
#define QWT_UTILS_H

#include <qevent.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <QWidget>
#include <QDockWidget>
#include <QMainWindow>
#include "scopygui_export.h"


class SCOPYGUI_EXPORT QwtDblClickPlotPicker: public QwtPlotPicker
{
public:
#if QWT_VERSION < 0x060100
    QwtDblClickPlotPicker(QwtPlotCanvas *);
#else /* QWT_VERSION < 0x060100 */
    QwtDblClickPlotPicker(QWidget *);
#endif /* QWT_VERSION < 0x060100 */

    ~QwtDblClickPlotPicker();

    virtual QwtPickerMachine * stateMachine(int) const;
};

class SCOPYGUI_EXPORT QwtPickerDblClickPointMachine: public QwtPickerMachine
{
public:
  QwtPickerDblClickPointMachine();
  ~QwtPickerDblClickPointMachine();

#if QWT_VERSION < 0x060000
  virtual CommandList
#else
  virtual QList<QwtPickerMachine::Command>
#endif
    transition( const QwtEventPattern &eventPattern,
				  const QEvent *e);
};

#endif // QWT_UTILS_H
