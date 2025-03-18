#include "pkggridwidget.h"

using namespace scopy;

PkgGridWidget::PkgGridWidget(QWidget *parent)
	: QWidget(parent)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_layout = new QGridLayout(this);
	m_layout->setMargin(0);
}

PkgGridWidget::~PkgGridWidget() {}

void PkgGridWidget::addPkg(PkgItemWidget *pkgItem)
{
	m_layout->addWidget(pkgItem, m_gridSize / MAX_COL, m_gridSize % MAX_COL);
	m_pkgMap.insert(pkgItem->name(), pkgItem);
	m_gridSize++;
}

void PkgGridWidget::removePkg(const QString &pkgName)
{
	if(!m_pkgMap.contains(pkgName)) {
		return;
	}
	QWidget *w = m_pkgMap[pkgName];
	m_layout->removeWidget(w);
}

void PkgGridWidget::searchPkg(const QString &pkgName)
{
	// Hide all the packages
	for(auto it = m_pkgMap.begin(); it != m_pkgMap.end(); ++it) {
		it.value()->hide();
	}

	// Reorganize the widgets within the layout
	int counter = 0;
	for(auto it = m_pkgMap.begin(); it != m_pkgMap.end(); ++it) {
		if(it.key().contains(pkgName, Qt::CaseInsensitive)) {
			m_layout->addWidget(it.value(), counter / MAX_COL, counter % MAX_COL);
			it.value()->show();
			counter++;
			if(counter > m_gridSize) {
				break;
			}
		}
	}
}
