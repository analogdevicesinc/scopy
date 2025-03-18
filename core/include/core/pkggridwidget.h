#ifndef PKGGRIDWIDGET_H
#define PKGGRIDWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QMap>
#include <pkgitemwidget.h>

namespace scopy {

class PkgGridWidget : public QWidget
{
	Q_OBJECT
public:
	PkgGridWidget(QWidget *parent = nullptr);
	~PkgGridWidget();

	void addPkg(PkgItemWidget *pkgItem);
	void removePkg(const QString &pkgName);

public Q_SLOTS:
	void searchPkg(const QString &pkgName);

private:
	QMap<QString, PkgItemWidget *> m_pkgMap;
	QGridLayout *m_layout;
	int m_gridSize = 0;
	const int MAX_COL = 3;
};
} // namespace scopy

#endif // PKGGRIDWIDGET_H
