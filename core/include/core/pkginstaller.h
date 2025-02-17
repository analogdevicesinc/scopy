#ifndef PKGINSTALLER_H
#define PKGINSTALLER_H

#include "scopy-core_export.h"
#include <QObject>
#include <QMap>
#include <kzip.h>

namespace scopy {
class SCOPY_CORE_EXPORT PkgInstaller : public QObject
{
	Q_OBJECT
protected:
	explicit PkgInstaller(QObject *parent = nullptr);
	~PkgInstaller();

public:
	// singleton
	PkgInstaller(PkgInstaller &other) = delete;
	void operator=(const PkgInstaller &) = delete;
	static PkgInstaller *GetInstance();

	static void init();

public:
	static bool install(const QString &zipPath = "");
	bool _install(const QString &zipPath = "");
	static bool uninstall(const QString &pkgName);
	static bool update(const QString &pkgName);

	static bool preview(const QString &zipPath = "");
	bool _preview(const QString &zipPath = "");

	QStringList installedPkg() const;

	bool extractFromStore(const QString &pluginName, const QString &destPath);

public Q_SLOTS:
	void onPkgMetadata(QVariantMap metadata);

Q_SIGNALS:
	void pkgMetadata(QVariantMap metadata);
	void zipMetadata(QVariantMap metadata);

private:
	static PkgInstaller *pinstance_;
	const QStringList m_requiredFields{"name", "version", "license", "author", "scopy_compatibility", "category"};

	bool validatePkg(QJsonObject &metadata);
	// bool pkgInstalled(const QString &pkgName);
	// QStringList &filters);
};
} // namespace scopy

#endif // PKGINSTALLER_H
