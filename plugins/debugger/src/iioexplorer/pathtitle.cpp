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

	for(QString segment : segmentStrings) {
		QPushButton *btn = new QPushButton(segment, this);
		StyleHelper::GrayButton(btn, "SegmentButton");
		auxPath.append(segment);
		connect(btn, &QPushButton::clicked, this, [this, auxPath]() { Q_EMIT pathSelected(auxPath); });
		layout()->addWidget(btn);
	}
}

QString PathTitle::title() const { return m_titlePath; }

void PathTitle::setupUi()
{
	setObjectName("PathTitle");
	setLayout(new QHBoxLayout(this));
}
