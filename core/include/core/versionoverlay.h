#ifndef SCOPY_VERSIONOVERLAY_H
#define SCOPY_VERSIONOVERLAY_H

#include <QWidget>
#include <popupwidget.h>

namespace scopy {
class VersionOverlay : public PopupWidget {
	Q_OBJECT
public:
	explicit VersionOverlay(QWidget* parent = nullptr);
	~VersionOverlay();
	void showOverlay();
};
}

#endif //SCOPY_VERSIONOVERLAY_H
