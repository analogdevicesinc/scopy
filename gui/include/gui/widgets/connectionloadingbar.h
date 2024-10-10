#ifndef CONNECTIONLOADINGBAR_H
#define CONNECTIONLOADINGBAR_H

#include "scopy-gui_export.h"
#include <QLabel>
#include <QProgressBar>
#include <QWidget>

namespace scopy {
class SCOPY_GUI_EXPORT ConnectionLoadingBar : public QWidget
{
	Q_OBJECT
public:
	explicit ConnectionLoadingBar(QWidget *parent = nullptr);
	void setCurrentPlugin(QString pluginName);
	void addProgress(int progress);
	void setProgressBarMaximum(int maximum);

private:
	void setupUi();

	QString m_currentPluginName;
	QLabel *m_loadingLabel;
	QLabel *m_pluginCountLabel;
	QProgressBar *m_progressBar;
	int m_pluginCount;
	int m_noPlugins;
};
} // namespace scopy

#endif // CONNECTIONLOADINGBAR_H
