#ifndef SCOPY_SCOPYTITLEMANAGER_H
#define SCOPY_SCOPYTITLEMANAGER_H

#include <QWidget>
#include <mutex>

namespace adiscope {
class ScopyTitleManager : public QObject {
	Q_OBJECT
protected:
	ScopyTitleManager(QObject *parent = nullptr);
	~ScopyTitleManager();

	void buildTitle();

public:
	ScopyTitleManager(ScopyTitleManager &other) = delete;
	void operator=(const ScopyTitleManager &other) = delete;

	static ScopyTitleManager *GetInstance();

	static void setApplicationName(QString title);
	static void clearApplicationName();

	static void setScopyVersion(QString version);
	static void clearScopyVersion();

	static void setGitHash(QString hash);
	static void clearGitHash();

	static void setIniFileName(QString filename);
	static void clearIniFileName();

	static void clearAll();
	static QString getCurrentTitle();

	static void setMainWindow(QWidget *window);

private:
	static ScopyTitleManager *pinstance_;
	QString m_title;
	QString m_version;
	QString m_hash;
	QString m_filename;
	QString m_currentTitle;
	QWidget *m_mainWindow;
};
} // namespace scopy

#endif // SCOPY_SCOPYTITLEMANAGER_H
