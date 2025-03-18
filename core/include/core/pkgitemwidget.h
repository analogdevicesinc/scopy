#ifndef PKGITEMWIDGET_H
#define PKGITEMWIDGET_H

#include <QLabel>
#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <animationpushbutton.h>
#include <toolbuttons.h>

namespace scopy {
class PkgItemWidget : public QFrame
{
	Q_OBJECT
public:
	PkgItemWidget(QWidget *parent = nullptr);
	~PkgItemWidget();

	QString name() const;

	QString zipPath() const;
	void setZipPath(const QString &newZipPath);

	void setSingleVersion(bool en);
	void installFinished(bool installed);
	void uninstallFinished(bool uninstalled);
	void setPreview(bool en);

public Q_SLOTS:
	void fillMetadata(QVariantMap metadata, bool installed = false);

Q_SIGNALS:
	void installClicked();
	void uninstallClicked();

private:
	QWidget *createTitleW(QWidget *parent = nullptr);

	QString m_zipPath;
	QLabel *m_title;
	QComboBox *m_versCb;
	QLabel *m_description;
	InstallBtn *m_installBtn;
	UninstallBtn *m_uninstallBtn;
	QWidget *m_btnsW;
	QVariantMap m_metadata;
};
} // namespace scopy

#endif // PKGITEMWIDGET_H
