#ifndef NETWORK_ANALYZER_GENERAL_MENU_HPP
#define NETWORK_ANALYZER_GENERAL_MENU_HPP

#include <QWidget>

namespace Ui {
class NetworkAnalyzerGeneralMenu;
}

namespace scopy {
namespace gui {

class NetworkAnalyzerGeneralMenu : public QWidget
{
	Q_OBJECT

public:
	explicit NetworkAnalyzerGeneralMenu(QWidget* parent = nullptr);
	~NetworkAnalyzerGeneralMenu();

private:
	void initUi();

private:
	Ui::NetworkAnalyzerGeneralMenu* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // NETWORK_ANALYZER_GENERAL_MENU_HPP
