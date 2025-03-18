#ifndef INFOHEADERWIDGET_H
#define INFOHEADERWIDGET_H

#include "toolbuttons.h"
#include <scopy-gui_export.h>
#include <QLineEdit>
#include <QWidget>
#include <baseheader.h>

namespace scopy {
class SCOPY_GUI_EXPORT InfoHeaderWidget : public QWidget, public BaseHeader
{
	Q_OBJECT
public:
	InfoHeaderWidget(QString title, QWidget *parent = nullptr);
	~InfoHeaderWidget();

	void setTitle(QString title);
	QString title();

	void setDescription(const QString &newDescription);

	InfoBtn *infoBtn() const;

	QLineEdit *titleEdit() const;

private:
	QLineEdit *m_titleEdit;
	InfoBtn *m_infoBtn;
	QString m_description;
	void setupDescriptionButton();
};
} // namespace scopy

#endif // INFOHEADERWIDGET_H
