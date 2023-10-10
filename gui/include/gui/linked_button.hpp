#ifndef LINKEDBUTTON_HPP
#define LINKEDBUTTON_HPP

#include "scopy-gui_export.h"

#include <QPushButton>

namespace scopy {

class SCOPY_GUI_EXPORT LinkedButton : public QPushButton
{
	Q_OBJECT

public:
	explicit LinkedButton(QWidget *parent = Q_NULLPTR);

	void setUrl(QString url);

private:
	QString url;

public Q_SLOTS:
	void openUrl();
};
} // namespace scopy

#endif // LINKEDBUTTON_HPP
