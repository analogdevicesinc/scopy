#ifndef SCOPYPREFERENCESPAGE_H
#define SCOPYPREFERENCESPAGE_H

#include <QTabWidget>
#include <QVBoxLayout>
#include "scopy-core_export.h"

namespace scopy {
class SCOPY_CORE_EXPORT ScopyPreferencesPage : public QWidget
{
	Q_OBJECT
public:
	ScopyPreferencesPage(QWidget *parent = nullptr);
	~ScopyPreferencesPage();
	QWidget* buildGeneralPreferencesPage();
	void addHorizontalTab(QWidget *w, QString text);
private Q_SLOTS:
	void resetScopyPreferences();
private:
	QTabWidget *tabWidget;
	void initRestartWidget();
	QWidget *restartWidget;
	QWidget *buildSaveSessionPreference();
	QWidget *buildResetScopyDefaultButton();
	QVBoxLayout *layout;
	void removeIniFiles(bool backup = true);
	void initUI();
};
}
#endif // SCOPYPREFERENCESPAGE_H
