#ifndef SUBSECTIONSEPARATOR_H
#define SUBSECTIONSEPARATOR_H

#include <QLabel>
#include <QPushButton>
#include <QWidget>

namespace Ui {
class SubsectionSeparator;
}

namespace scopy {
namespace gui {
class SubsectionSeparator : public QWidget
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
	void setButton(bool buttonVisible);

	QLabel* getLabel();
	void setLabel(const QString& text);
};
} // namespace gui
} // namespace scopy

#endif // SUBSECTIONSEPARATOR_H
