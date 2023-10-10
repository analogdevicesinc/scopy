#ifndef QWT_UTILS_H
#define QWT_UTILS_H

#include "scopy-gui_export.h"

#include <QDockWidget>
#include <QMainWindow>
#include <QWidget>
#include <qevent.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_picker.h>

class SCOPY_GUI_EXPORT QwtDblClickPlotPicker : public QwtPlotPicker
{
public:
#if QWT_VERSION < 0x060100
	QwtDblClickPlotPicker(QwtPlotCanvas *);
#else  /* QWT_VERSION < 0x060100 */
	QwtDblClickPlotPicker(QWidget *);
#endif /* QWT_VERSION < 0x060100 */

	~QwtDblClickPlotPicker();

	virtual QwtPickerMachine *stateMachine(int) const;
};

class SCOPY_GUI_EXPORT QwtPickerDblClickPointMachine : public QwtPickerMachine
{
public:
	QwtPickerDblClickPointMachine();
	~QwtPickerDblClickPointMachine();

#if QWT_VERSION < 0x060000
	virtual CommandList
#else
	virtual QList<QwtPickerMachine::Command>
#endif
	transition(const QwtEventPattern &eventPattern, const QEvent *e);
};

#endif // QWT_UTILS_H
