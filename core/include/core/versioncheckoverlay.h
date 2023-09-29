#ifndef SCOPY_VERSIONCHECKOVERLAY_H
#define SCOPY_VERSIONCHECKOVERLAY_H

#include <QWidget>
#include <popupwidget.h>
#include <QPushButton>

namespace scopy {
class VersionCheckOverlay : public PopupWidget {
	Q_OBJECT

public:
	explicit VersionCheckOverlay(QWidget* parent = nullptr);
	~VersionCheckOverlay();
	void showOverlay();
};
}

#endif //SCOPY_VERSIONCHECKOVERLAY_H
