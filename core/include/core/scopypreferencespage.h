#ifndef SCOPYPREFERENCESPAGE_H
#define SCOPYPREFERENCESPAGE_H

#include <QTabWidget>
#include "scopy-core_export.h"

namespace scopy {
class SCOPY_CORE_EXPORT ScopyPreferencesPage : public QTabWidget
{
	Q_OBJECT
public:
	ScopyPreferencesPage(QWidget *parent = nullptr);
	~ScopyPreferencesPage();
	QWidget* buildGeneralPreferencesPage();
	void addHorizontalTab(QWidget *w, QString text);
public Q_SLOTS:
	void showRestartWidget();
private:
	QWidget *createRestartWidget();
	QWidget *restartWidget;
};
}
#endif // SCOPYPREFERENCESPAGE_H
