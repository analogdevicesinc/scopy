#ifndef PKGITEMWIDGET_H
#define PKGITEMWIDGET_H

#include <QLabel>
#include <QWidget>
#include <QComboBox>
#include <QPushButton>

namespace scopy {
class PkgItemWidget : public QWidget
{
	Q_OBJECT
public:
	PkgItemWidget(QWidget *parent = nullptr);
	~PkgItemWidget();

	QString name() const;

	QString zipPath() const;
	void setZipPath(const QString &newZipPath);

	void setUninstalled();
	void setInstalled();

public Q_SLOTS:
	void fillMetadata(QVariantMap metadata, bool installed = false);

Q_SIGNALS:
	void installClicked();
	void uninstallClicked();

private:
	QString m_zipPath;
	QLabel *m_title;
	QComboBox *m_versCb;
	QLabel *m_description;
	QPushButton *m_installBtn;
	QPushButton *m_uninstallBtn;
	QVariantMap m_metadata;
};
} // namespace scopy

#endif // PKGITEMWIDGET_H
