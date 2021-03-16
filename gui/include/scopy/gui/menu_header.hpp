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
	explicit MenuHeader(QWidget* parent = nullptr, const QString& label = nullptr,
			    const QColor* color = new QColor("#6E6E6F"));
	~MenuHeader();

private:
	Ui::MenuHeader* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // MENU_HEADER_HPP
