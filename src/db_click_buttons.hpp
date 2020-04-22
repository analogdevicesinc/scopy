#ifndef DB_CLICK_BUTTONS_HPP
#define DB_CLICK_BUTTONS_HPP

#include <QList>
#include <QPushButton>
#include <QString>
#include <QWidget>

namespace Ui {
class DbClickButtons;
}

namespace adiscope {

class DbClickButtons : public QWidget
{
	Q_OBJECT

public:
	explicit DbClickButtons(QWidget* parent = 0, int maxRowBtnCount = 5);
	~DbClickButtons();

	int selectedButton() const;
	void setSelectedButton(int btnId);

	bool buttonChecked(int btnId) const;
	void setButtonChecked(int btnId, bool checked);

	int buttonCount() const;
	void setButtonCount(int count);

Q_SIGNALS:
	void buttonSelected(int);
	void buttonToggled(int, bool);

private Q_SLOTS:
	void onButtonClicked();

private:
	void toggleButton(int btnId);

private:
	Ui::DbClickButtons* ui;
	QList<QPushButton*> btn_list;
	QList<bool> btn_states;
	int selected_btn;
	int max_row_btn_cnt;
	QList<QString> color_codes;
};

} // namespace adiscope

#endif // DB_CLICK_BUTTONS_HPP
