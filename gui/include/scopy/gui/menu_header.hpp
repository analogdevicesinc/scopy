#ifndef MENU_HEADER_HPP
#define MENU_HEADER_HPP

#include <QWidget>

namespace Ui {
class MenuHeader;
}

namespace scopy {
namespace gui {

class MenuHeader : public QWidget
{
	Q_OBJECT

public:
	explicit MenuHeader(QWidget* parent = nullptr);
	explicit MenuHeader(const QString& label = nullptr, const QColor* color = new QColor("#6E6E6F"),
			    QWidget* parent = nullptr);
	~MenuHeader();

private:
	Ui::MenuHeader* m_ui;

public:
	void setLabel(const QString& text);
	void setLineColor(const QColor* color);
};
} // namespace gui
} // namespace scopy

#endif // MENU_HEADER_HPP
