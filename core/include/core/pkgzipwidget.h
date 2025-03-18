#ifndef PKGZIPWIDGET_H
#define PKGZIPWIDGET_H

#include <QWidget>
#include <filebrowserwidget.h>
#include <infoheaderwidget.h>
#include <menucollapsesection.h>

namespace scopy {
class PkgZipWidget : public QFrame
{
	Q_OBJECT
public:
	PkgZipWidget(QWidget *parent);
	~PkgZipWidget();

public Q_SLOTS:
	void onZipMetadata(QVariantMap metadata);
	void onInstallFinished();

Q_SIGNALS:
	void pkgPathSelected(const QString &path);
	void installClicked(const QString &zipPath);

private:
	MenuCollapseSection *createSection(QString title, QString description, QWidget *parent = nullptr);
	InfoHeaderWidget *getInfoHeader(MenuCollapseSection *section);
	QWidget *createBrowserSection(QWidget *parent = nullptr);

	FileBrowserWidget *m_fileBrowser;
	MenuCollapseSection *m_previewSection;
	QLabel *m_warningLabel;
	InstallBtn *m_installBtn;
};
} // namespace scopy

#endif // PKGZIPWIDGET_H
