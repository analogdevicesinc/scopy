#ifndef MENUSECTIONWIDGET_H
#define MENUSECTIONWIDGET_H
#include <scopy-gui_export.h>
#include <utils.h>
#include <QWidget>
#include <QVBoxLayout>

namespace scopy {
class SCOPY_GUI_EXPORT MenuSectionWidget : public QWidget {
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuSectionWidget(QWidget *parent = nullptr);
	~MenuSectionWidget();
	QVBoxLayout *contentLayout() const;

private:
	QVBoxLayout *m_layout;
};

}
#endif // MENUSECTIONWIDGET_H
