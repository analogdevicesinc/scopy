#ifndef DETACHEDTOOLWINDOW_H
#define DETACHEDTOOLWINDOW_H

#include "scopy-core_export.h"

#include <QWidget>

#include <pluginbase/toolmenuentry.h>

namespace scopy {
class SCOPY_CORE_EXPORT DetachedToolWindow : public QWidget
{
	Q_OBJECT
public:
	DetachedToolWindow(QWidget *parent, ToolMenuEntry *tme);
	~DetachedToolWindow();

	void saveToolGeometry(ToolMenuEntry *tme, QWidget *w);
	void loadToolGeometry(ToolMenuEntry *tme, QWidget *w);

private:
	ToolMenuEntry *tme;
	QWidget *w;

protected:
	void closeEvent(QCloseEvent *event) override;
};
} // namespace scopy
#endif // DETACHEDTOOLWINDOW_H
