#include "logicgroupitem.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

#include "basemenu.h"

using namespace adiscope;

LogicGroupItem::LogicGroupItem(const QString &name, QWidget *parent):
	BaseMenuItem(parent),
	m_nameLabel(new QLabel(name, this))
{
	buildUi();
}

QString LogicGroupItem::getName() const
{
	return m_nameLabel->text();
}

void LogicGroupItem::setName(const QString &name)
{
	if (m_nameLabel->text() != name) {
		m_nameLabel->setText(name);
	}
}

void LogicGroupItem::buildUi()
{
	QWidget *groupItem = new QWidget(this);
	QVBoxLayout *vbox = new QVBoxLayout(groupItem);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->setSpacing(0);
	QHBoxLayout *layout = new QHBoxLayout(this);
	groupItem->setLayout(vbox);

	QSpacerItem *spacerTop = new QSpacerItem(40, 5, QSizePolicy::Expanding, QSizePolicy::Fixed);
	QSpacerItem *spacerBot = new QSpacerItem(40, 5, QSizePolicy::Expanding, QSizePolicy::Fixed);
	vbox->insertItem(0, spacerTop);
	vbox->insertItem(1, layout);
	vbox->insertItem(2, spacerBot);

	layout->setSpacing(6);
	layout->setContentsMargins(0, 0, 0, 0);

	QWidget *dragWidget = new QWidget(this); // |||
	QVBoxLayout *layoutDragWidget = new QVBoxLayout(dragWidget);
	dragWidget->setLayout(layoutDragWidget);

	layoutDragWidget->setContentsMargins(0, 0, 0, 0);
	layoutDragWidget->setSpacing(4);

	dragWidget->setMaximumSize(QSize(16, 16));
	dragWidget->setMinimumSize(QSize(16, 16));

	// add 3 lines one on top of the other like ||| but flipped vertically
	for (int i = 0; i < 3; ++i) {
		QFrame *frame = new QFrame(dragWidget);
		frame->setMaximumSize(QSize(16, 2));
		frame->setStyleSheet(QString::fromUtf8("border: 2px solid rgba(255, 255, 255, 70)"));
		frame->setFrameShadow(QFrame::Plain);
		frame->setLineWidth(2);
		frame->setFrameShape(QFrame::HLine);
		layoutDragWidget->addWidget(frame);
	}

	QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	QPushButton *deleteBtn = new QPushButton(this);
	deleteBtn->setFlat(true);
	deleteBtn->setIcon(QIcon(":/icons/close.svg"));
	deleteBtn->setMaximumSize(QSize(16, 16));
	connect(deleteBtn, &QPushButton::clicked,
		this, &LogicGroupItem::deleteBtnClicked);
	connect(deleteBtn, &QPushButton::clicked, [=](){
		BaseMenu *menu = getOwner();

		if (!menu) {
			return;
		}

		menu->removeMenuItem(this);
		deleteLater();
	});

	m_nameLabel->setMinimumHeight(16);

	layout->insertWidget(0, m_nameLabel);
	layout->insertWidget(1, deleteBtn);
	layout->insertSpacerItem(2, spacer);
	layout->insertWidget(3, dragWidget);

	setDragWidget(dragWidget);

	setWidget(groupItem);
}
