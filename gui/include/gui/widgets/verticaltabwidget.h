#ifndef VERTICALTABWIDGET_H
#define VERTICALTABWIDGET_H

#include <scopy-gui_export.h>
#include <QTabBar>
#include <QTabWidget>

namespace scopy {

class SCOPY_GUI_EXPORT VerticalTabWidget : public QTabWidget
{
	Q_OBJECT

public:
	explicit VerticalTabWidget(QWidget *parent = nullptr);
};

} // namespace scopy

#endif // VERTICALTABWIDGET_H
