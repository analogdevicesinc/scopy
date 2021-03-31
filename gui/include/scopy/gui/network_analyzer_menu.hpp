#ifndef NETWORK_ANALYZER_MENU_HPP
#define NETWORK_ANALYZER_MENU_HPP

#include <QWidget>

namespace Ui {
class NetworkAnalyzerMenu;
}

namespace scopy {
namespace gui {

class NetworkAnalyzerMenu : public QWidget
{
	Q_OBJECT

public:
	explicit NetworkAnalyzerMenu(QWidget* parent = nullptr);
	~NetworkAnalyzerMenu();

private:
	Ui::NetworkAnalyzerMenu* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // NETWORK_ANALYZER_MENU_HPP
