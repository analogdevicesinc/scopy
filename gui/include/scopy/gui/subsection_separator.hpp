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
	explicit SubsectionSeparator(const QString& text, const bool buttonVisible = false, QWidget* parent = nullptr);
	~SubsectionSeparator();

private:
	Ui::SubsectionSeparator* m_ui;

public:
	QPushButton* getButton();
	QLabel* getLabel();
};
} // namespace gui
} // namespace scopy

#endif // SUBSECTIONSEPARATOR_H
