#ifndef MENUSECTIONWIDGET_H
#define MENUSECTIONWIDGET_H
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

#include <scopy-gui_export.h>
#include <utils.h>

namespace scopy {
class SCOPY_GUI_EXPORT MenuSectionWidget : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuSectionWidget(QWidget *parent = nullptr);
	~MenuSectionWidget();
	QVBoxLayout *contentLayout() const;

private:
	QVBoxLayout *m_layout;
};

class SCOPY_GUI_EXPORT MenuVScrollArea : public QScrollArea
{
	Q_OBJECT
	//	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuVScrollArea(QWidget *parent = nullptr);
	~MenuVScrollArea();
	QVBoxLayout *contentLayout() const;

private:
	QVBoxLayout *m_layout;
};

} // namespace scopy
#endif // MENUSECTIONWIDGET_H
