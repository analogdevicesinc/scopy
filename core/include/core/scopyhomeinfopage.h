#ifndef SCOPYHOMEINFOPAGE_H
#define SCOPYHOMEINFOPAGE_H

#include "scopy-core_export.h"

#include <QPushButton>
#include <QWidget>

#include <hoverwidget.h>

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
} // namespace scopy

#endif // SCOPYHOMEINFOPAGE_H
