#ifndef BASEHEADERWIDGET_H
#define BASEHEADERWIDGET_H

#include <QLineEdit>
#include <baseheader.h>
#include <scopy-gui_export.h>
#include <QVBoxLayout>

namespace scopy {
class SCOPY_GUI_EXPORT BaseHeaderWidget : public QWidget, public BaseHeader
{
	Q_OBJECT
public:
	BaseHeaderWidget(QString title, QWidget *parent = nullptr);
	~BaseHeaderWidget();

	void setTitle(QString title) override;
	QString title() override;

protected:
	QLineEdit *m_label;
	QVBoxLayout *m_lay;
};
} // namespace scopy

#endif // BASEHEADERWIDGET_H
