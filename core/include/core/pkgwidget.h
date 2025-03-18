#ifndef PKGWIDGET_H
#define PKGWIDGET_H

#include <QLineEdit>
#include <QWidget>
#include <qboxlayout.h>
#include <menulineedit.h>
#include <menusectionwidget.h>
#include <scopy-core_export.h>
#include <searchbar.h>
#include <pkggridwidget.h>

namespace scopy {

class SCOPY_CORE_EXPORT PkgWidget : public QWidget
{
	Q_OBJECT
public:
	PkgWidget(QWidget *parent = nullptr);
	~PkgWidget();

	void fillPkgSection(QWidget *parent = nullptr);
	QWidget *pkgSection(QWidget *parent = nullptr);
	QWidget *pkgList(QWidget *parent = nullptr);

public Q_SLOTS:
	void showInstallDialog(const QString &zipPath, const QString &pkgPath);

private Q_SLOTS:
	void onInstall(const QString &zipPath);
	void onUninstall();

Q_SIGNALS:
	void pkgReinstalled(bool installed = true);

private:
	PkgGridWidget *m_pkgGrid;
	QVBoxLayout *m_layScroll;
	QSpacerItem *m_scrollSpacer;
	SearchBar *m_searchBar;

	QWidget *createRestartWidget(QWidget *parent);
};

} // namespace scopy

#endif // PKGWIDGET_H
