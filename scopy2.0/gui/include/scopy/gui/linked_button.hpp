#ifndef LINKEDBUTTON_HPP
#define LINKEDBUTTON_HPP

#include <QPushButton>

namespace scopy {
namespace gui {

class LinkedButton : public QPushButton
{
	Q_OBJECT

public:
	explicit LinkedButton(QWidget* parent = Q_NULLPTR);

	void setUrl(QString url);

private:
	QString m_url;

public Q_SLOTS:
	void openUrl();
};

} // namespace gui
} // namespace scopy

#endif // LINKEDBUTTON_HPP
