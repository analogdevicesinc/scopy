#ifndef MENU_HEADER_HPP
#define MENU_HEADER_HPP

#include <QPushButton>
#include <QWidget>

namespace Ui {
class MenuHeader;
}

namespace adiscope {
namespace gui {

class MenuHeader : public QWidget
{
	Q_OBJECT

public:
	explicit MenuHeader(QWidget* parent = nullptr);
	explicit MenuHeader(const QString& label = nullptr, const QColor* color = new QColor("#4A64FF"),
			    bool enableBtnVisible = false, QWidget* parent = nullptr);
	~MenuHeader();

private:
	Ui::MenuHeader* m_ui;

public Q_SLOTS:
	void setEnabledBtnState(bool state);

Q_SIGNALS:
	void enableBtnToggled(bool state);

public:
	void setLabel(const QString& text);
	void setLineColor(const QColor* color);
	void setEnableBtnVisible(bool visible);

	QPushButton* getEnableBtn();
};
} // namespace gui
} // namespace adiscope

#endif // MENU_HEADER_HPP
