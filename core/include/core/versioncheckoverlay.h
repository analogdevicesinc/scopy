#ifndef SCOPY_VERSIONOVERLAY_H
#define SCOPY_VERSIONOVERLAY_H

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
	QPushButton getExitBtn();

};
}

#endif //SCOPY_VERSIONOVERLAY_H
