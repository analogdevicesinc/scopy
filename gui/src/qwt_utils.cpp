#include "qwt_utils.h"
#include <sstream>
#include <string>
#include <algorithm>
#include <QDebug>
#include <QSizePolicy>

QwtPickerDblClickPointMachine::QwtPickerDblClickPointMachine()
#if QWT_VERSION < 0x060000
    : QwtPickerMachine ()
#else
    : QwtPickerMachine (PointSelection)
#endif
{
}

QwtPickerDblClickPointMachine::~QwtPickerDblClickPointMachine()
{
}

#if QWT_VERSION < 0x060000
#define CMDLIST_TYPE QwtPickerMachine::CommandList
#else
#define CMDLIST_TYPE QList<QwtPickerMachine::Command>
#endif
CMDLIST_TYPE
QwtPickerDblClickPointMachine::transition(const QwtEventPattern &eventPattern,
					  const QEvent *e)
{
  CMDLIST_TYPE cmdList;
  switch(e->type()) {
    case QEvent::MouseButtonDblClick:
      if ( eventPattern.mouseMatch(QwtEventPattern::MouseSelect1,
				   (const QMouseEvent *)e) ) {
	cmdList += QwtPickerMachine::Begin;
	cmdList += QwtPickerMachine::Append;
	cmdList += QwtPickerMachine::End;
      }
      break;
  default:
    break;
  }
  return cmdList;
}

#if QWT_VERSION < 0x060100
QwtDblClickPlotPicker::QwtDblClickPlotPicker(QwtPlotCanvas* canvas)
#else /* QWT_VERSION < 0x060100 */
QwtDblClickPlotPicker::QwtDblClickPlotPicker(QWidget* canvas)
#endif /* QWT_VERSION < 0x060100 */
  : QwtPlotPicker(canvas)
{
#if QWT_VERSION < 0x060000
  setSelectionFlags(QwtPicker::PointSelection);
#endif
}

QwtDblClickPlotPicker::~QwtDblClickPlotPicker()
{
}

QwtPickerMachine*
QwtDblClickPlotPicker::stateMachine(int n) const
{
  return new QwtPickerDblClickPointMachine;
}
