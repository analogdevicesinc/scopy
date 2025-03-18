#ifndef PKGMANAGER_H
#define PKGMANAGER_H

#include <QFileInfoList>
#include <QMap>
#include <QObject>

namespace scopy {
class PkgManager : public QObject
{
	Q_OBJECT
protected:
	explicit PkgManager(QObject *parent = nullptr);
	~PkgManager();

public:
	// singleton
	PkgManager(PkgManager &other) = delete;
	void operator=(const PkgManager &) = delete;
	static PkgManager *GetInstance();
	static void init(const QStringList &paths);
	void _init(const QStringList &paths);

	static bool install(const QString &zipPath = "", bool performRestart = true);
	bool _install(const QString &zipPath = "", bool performRestart = true);
	static bool uninstall(const QString &pkgName, bool performRestart = true);
	bool _uninstall(const QString &pkgName = "", bool performRestart = true);
	static bool update(const QString &pkgName);

	static bool preview(const QString &zipPath = "");
	bool _preview(const QString &zipPath = "");

	static QFileInfoList listFilesInfo(const QStringList &filter);
	static QStringList listFilesPath(const QStringList &filter);

	static QStringList getInstalledPkgs();
	static QList<QVariantMap> getPkgsMeta();
Q_SIGNALS:
	void pkgLoaded(QVariantMap metadata);
	void pkgExists(const QString &zipPath, const QString &pkgName);

	void pkgInstalled(bool restart);
	void pkgUninstalled(bool restart);

	// void validationFailed(const QString &msg);
	// void validationSucceed(const QString &msg);
	void zipMetadata(QVariantMap metadata);

private:
	static QFileInfoList getFilesInfo(const QString &path, const QStringList &filterList);
	static QStringList getFilesPath(const QString &path, const QStringList &filterList);
	void loadPkg(const QFileInfo &fileInfo);
	void createPkgDirectory();

	static QString packagesPath_;
	static PkgManager *pinstance_;
	static QMap<QString, QVariantMap> validPackages_;
};
} // namespace scopy

#endif // PKGMANAGER_H
