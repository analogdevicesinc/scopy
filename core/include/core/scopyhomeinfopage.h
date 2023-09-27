#ifndef SCOPYHOMEINFOPAGE_H
#define SCOPYHOMEINFOPAGE_H

#include <QWidget>
#include <QPushButton>
#include <hoverwidget.h>
#include "scopy-core_export.h"

namespace Ui {
class ScopyHomeInfoPage;
}

namespace scopy {
class SCOPY_CORE_EXPORT ScopyHomeInfoPage : public QWidget
{
Q_OBJECT

public:
	explicit ScopyHomeInfoPage(QWidget *parent = nullptr);
	~ScopyHomeInfoPage();

private:
	Ui::ScopyHomeInfoPage *ui;

	void initReportButton();
};
}

#endif // SCOPYHOMEINFOPAGE_H
