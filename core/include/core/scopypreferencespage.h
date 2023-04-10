#ifndef SCOPYPREFERENCESPAGE_H
#define SCOPYPREFERENCESPAGE_H

#include <QTabWidget>
#include "scopycore_export.h"

namespace adiscope {
class SCOPYCORE_EXPORT ScopyPreferencesPage : public QTabWidget
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
