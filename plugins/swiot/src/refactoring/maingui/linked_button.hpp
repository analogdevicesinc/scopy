#ifndef LINKEDBUTTON_HPP
#define LINKEDBUTTON_HPP

#include <QPushButton>

namespace adiscope {

class LinkedButton : public QPushButton
{
	Q_OBJECT

public:
	explicit LinkedButton(QWidget* parent = Q_NULLPTR);

	void setUrl(QString url);

private:
	QString url;

public Q_SLOTS:
	void openUrl();
};
}

#endif // LINKEDBUTTON_HPP
