#include "pathtitle.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <gui/stylehelper.h>

// Currently there is no need for a custom split char, so it will be unchangeable
#define SPLIT_CHAR '/'

using namespace scopy;

PathTitle::PathTitle(QWidget *parent)
	: QWidget(parent)
{
	setupUi();
}

PathTitle::PathTitle(QString title, QWidget *parent)
	: QWidget(parent)
{
	setupUi();
	setTitle(title);
}

void PathTitle::setTitle(QString title)
{
	m_titlePath = title;

	// Delete all children
	while(QWidget *child = findChild<QWidget *>()) {
		delete child;
	}

	// Add new children
	QStringList segmentStrings = title.split(SPLIT_CHAR, Qt::SkipEmptyParts);
	QString auxPath;

	for(int i = 0; i < segmentStrings.size(); ++i) {
		QPushButton *btn = new QPushButton(segmentStrings[i], this);
		StyleHelper::GrayButton(btn, "SegmentButton");
		auxPath.append(segmentStrings[i]);
		connect(btn, &QPushButton::clicked, this, [this, auxPath]() { Q_EMIT pathSelected(auxPath); });
		layout()->addWidget(btn);

		if(i < segmentStrings.size() - 1) {
			auxPath.append("/");
		}
	}
}

QString PathTitle::title() const { return m_titlePath; }

void PathTitle::setupUi()
{
	setObjectName("PathTitle");
	setLayout(new QHBoxLayout(this));
}
