#ifndef DETACHEDTOOLWINDOW_H
#define DETACHEDTOOLWINDOW_H

#include <QWidget>
#include <pluginbase/toolmenuentry.h>
#include "scopy-core_export.h"

namespace scopy {
class SCOPY_CORE_EXPORT DetachedToolWindow : public QWidget {
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
}
#endif // DETACHEDTOOLWINDOW_H
