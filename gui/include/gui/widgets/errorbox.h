#ifndef SCOPY_ERRORBOX_H
#define SCOPY_ERRORBOX_H

#include <QFrame>
#include "scopy-gui_export.h"

namespace scopy {
class SCOPY_GUI_EXPORT ErrorBox : public QFrame
{
	Q_OBJECT
public:
	typedef enum
	{
		Green,
		Yellow,
		Red,
		Transparent,
	} AvailableColors;

	explicit ErrorBox(QWidget *parent = nullptr);

public Q_SLOTS:
	void changeColor(AvailableColors color);
};
} // namespace scopy

#endif // SCOPY_ERRORBOX_H
