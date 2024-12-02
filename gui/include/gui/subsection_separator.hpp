#ifndef SUBSECTIONSEPARATOR_H
#define SUBSECTIONSEPARATOR_H

#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include "scopy-gui_export.h"

namespace Ui {
class SubsectionSeparator;
}

namespace scopy {
namespace gui {
class SCOPY_GUI_EXPORT SubsectionSeparator : public QWidget
{
	Q_OBJECT

public:
	explicit SubsectionSeparator(QWidget* parent = nullptr);
	explicit SubsectionSeparator(const QString& text, const bool buttonVisible = false, QWidget* parent = nullptr);
	~SubsectionSeparator();

private:
	Ui::SubsectionSeparator* m_ui;

public:
	QPushButton* getButton();
	void setButtonVisible(bool buttonVisible);

	bool getButtonChecked();
	void setButtonChecked(bool checked);

	QLabel* getLabel();
	void setLabel(const QString& text);

	void setLabelVisible(bool visible);
	void setLineVisible(bool visible);

	void setContent(QWidget* content);
	QWidget* getContentWidget();

};
} // namespace gui
} // namespace scopy

#endif // SUBSECTIONSEPARATOR_H
