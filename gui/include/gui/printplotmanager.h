#ifndef PRINTPLOTMANAGER_H
#define PRINTPLOTMANAGER_H

#include "scopy-gui_export.h"

#include <QObject>

#include <plotwidget.h>
#include <toolbuttons.h>

namespace scopy {
class SCOPY_GUI_EXPORT PrintPlotManager : public QObject
{
	Q_OBJECT
public:
	explicit PrintPlotManager(QObject *parent = nullptr);

	void printPlots(QList<PlotWidget *> plotList, QString toolName);
	void setPrintWithSymbols(bool printWithSymbols);
signals:

private:
	bool m_printWithSymbols = false;
};
} // namespace scopy
#endif // PRINTPLOTMANAGER_H
