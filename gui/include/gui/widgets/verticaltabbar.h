#ifndef VERTICALTABBAR_H
#define VERTICALTABBAR_H

#include <scopy-gui_export.h>
#include <QTabBar>

namespace scopy {

class SCOPY_GUI_EXPORT VerticalTabBar : public QTabBar
{
	Q_OBJECT

public:
	explicit VerticalTabBar(QWidget *parent = nullptr);

protected:
	void paintEvent(QPaintEvent *event) override;
};

} // namespace scopy

#endif // VERTICALTABBAR_H
