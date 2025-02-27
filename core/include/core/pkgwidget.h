#ifndef PKGWIDGET_H
#define PKGWIDGET_H

#include <QLineEdit>
#include <QWidget>
#include <qboxlayout.h>
#include <menulineedit.h>
#include <menusectionwidget.h>
#include <scopy-core_export.h>

namespace scopy {

class SCOPY_CORE_EXPORT PkgWidget : public QWidget
{
	Q_OBJECT
public:
	PkgWidget(QWidget *parent = nullptr);
	~PkgWidget();

	void initZipSection(QWidget *parent = nullptr);
	QWidget *pkgSection(QWidget *parent = nullptr);
	QWidget *pkgList(QWidget *parent = nullptr);

public Q_SLOTS:
	void onZipMetadata(QVariantMap metadata);
	void showInstallDialog(const QString &pkgName, const QString &zipPath);

private:
	void initPkgList();
	void browseFile();
	void addPkgItem(QWidget *pkgItem);

private Q_SLOTS:
	void onInstall();
	void onUninstall();

Q_SIGNALS:
	void pkgReinstalled();

private:
	MenuSectionCollapseWidget *m_zipSection;
	QVBoxLayout *m_layScroll;
	QSpacerItem *m_scrollSpacer;
	MenuLineEdit *m_zipFileEdit;
	QWidget *createRestartWidget(QWidget *parent);
};

} // namespace scopy

#endif // PKGWIDGET_H
