#ifndef COMPOSITEHEADERWIDGET_H
#define COMPOSITEHEADERWIDGET_H

#include <QLineEdit>
#include <baseheader.h>
#include <compositewidget.h>
#include <scopy-gui_export.h>
#include <QVBoxLayout>
#include <baseheaderwidget.h>

namespace scopy {
class SCOPY_GUI_EXPORT CompositeHeaderWidget : public BaseHeaderWidget, public CompositeWidget
{
	Q_OBJECT
public:
	CompositeHeaderWidget(QString title, QWidget *parent = nullptr);
	~CompositeHeaderWidget();

	void add(QWidget *w) override;
	void remove(QWidget *w) override;
};
} // namespace scopy

#endif // COMPOSITEHEADERWIDGET_H
