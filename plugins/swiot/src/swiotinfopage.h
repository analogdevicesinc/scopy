#ifndef SCOPY_SWIOTINFOPAGE_H
#define SCOPY_SWIOTINFOPAGE_H

#include <QWidget>
#include <infopage.h>
#include "ui_swiotInfoPage.h"

namespace scopy::swiot {
class SwiotInfoPage : public QWidget {
	Q_OBJECT
public:
	explicit SwiotInfoPage(QWidget* parent = nullptr);
	~SwiotInfoPage();

	QPushButton* getCtxAttrsButton() const; // TODO: rename this
	void setText(const QString& text);

private:
	Ui::SWIOTInfoPage *ui;
};
}

#endif //SCOPY_SWIOTINFOPAGE_H
