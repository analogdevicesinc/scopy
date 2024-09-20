#ifndef BASEHEADERWIDGET_H
#define BASEHEADERWIDGET_H

#include <QLineEdit>
#include <baseheader.h>
#include <scopy-gui_export.h>

namespace scopy {
class SCOPY_GUI_EXPORT BaseHeaderWidget : public BaseHeader
{
	Q_OBJECT
public:
	BaseHeaderWidget(QString title, QWidget *parent = nullptr);
	~BaseHeaderWidget();

	void setTitle(QString title) override;
	QString title() override;

private:
	QLineEdit *m_label;
};
} // namespace scopy

#endif // BASEHEADERWIDGET_H
