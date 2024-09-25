#include "toolmenuitem.h"
#include "dynamicWidget.h"

#include <QEvent>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <utils.h>

#include <pluginbase/toolmenuentry.h>

using namespace scopy;

ToolMenuItem::ToolMenuItem(QString uuid, QString name, QString icon, QWidget *parent)
	: QWidget(parent)
	, m_uuid(uuid)
	, m_name(name)
	, m_icon(icon)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QVBoxLayout *lay = new QVBoxLayout(this);
	setLayout(lay);
	setFixedHeight(50);
	lay->setSpacing(0);
	lay->setContentsMargins(0, 0, 0, 0);

	QWidget *toolOption = new QWidget(this);
	toolOption->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QHBoxLayout *toolLay = new QHBoxLayout(toolOption);
	toolLay->setSpacing(0);
	toolLay->setContentsMargins(0, 0, 0, 0);
	m_toolBtn = new QPushButton(m_name);
	m_toolBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	m_toolRunBtn = new CustomPushButton(toolOption);
	toolLay->addWidget(m_toolBtn, Qt::AlignLeft);
	toolLay->addWidget(m_toolRunBtn);

	setDynamicProperty(m_toolRunBtn, "stopButton", true);
	m_toolRunBtn->setMaximumSize(32, 32);
	m_toolBtn->setMinimumHeight(42);

	m_toolBtn->setIcon(QIcon::fromTheme(m_icon));
	m_toolBtn->setCheckable(true);
	m_toolBtn->setIconSize(QSize(32, 32));

	m_toolRunBtn->setCheckable(true);
	m_toolRunBtn->setText("");

	m_toolBtn->setFlat(true);
	m_toolRunBtn->setFlat(true);

	lay->addWidget(toolOption);

	// Load stylesheets
	setStyleSheet(Util::loadStylesheetFromFile(":/gui/stylesheets/toolMenuItem.qss"));
	setAttribute(Qt::WA_StyledBackground, true);
#ifdef __ANDROID__
	setDynamicProperty(this, "allowHover", false);
#else
	setDynamicProperty(this, "allowHover", true);
	enableDoubleClick(true);
#endif
}

ToolMenuItem::~ToolMenuItem() {}

QPushButton *ToolMenuItem::getToolBtn() const { return m_toolBtn; }

QPushButton *ToolMenuItem::getToolRunBtn() const { return m_toolRunBtn; }

void ToolMenuItem::enableDoubleClick(bool enable)
{
	if(enable) {
		m_toolBtn->installEventFilter(this);
	} else {
		m_toolBtn->removeEventFilter(this);
		removeEventFilter(this);
	}
}

bool ToolMenuItem::eventFilter(QObject *watched, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonDblClick) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
		if(mouseEvent->button() == Qt::LeftButton) {
			if(isEnabled()) {
				Q_EMIT doubleclick();
				return true;
			}
		}
	}

	return QObject::event(event);
}

void ToolMenuItem::setName(QString str)
{
	m_name = str;
	m_toolBtn->setText(m_name);
}

void ToolMenuItem::hideText(bool hidden)
{
	if(hidden) {
		m_toolBtn->setText("");
	} else {
		m_toolBtn->setText(m_name);
	}
}

void ToolMenuItem::setDisabled(bool disabled) { setDisabled(disabled); }

void ToolMenuItem::updateItem()
{
	ToolMenuEntry *tme = dynamic_cast<ToolMenuEntry *>(QObject::sender());
	Q_ASSERT(tme);
	QSignalBlocker sb(m_toolRunBtn);
	setVisible(tme->visible());
	setEnabled(tme->enabled());
	setName(tme->name());
	m_toolRunBtn->setEnabled(tme->runEnabled());
	m_toolRunBtn->setEnabled(tme->runBtnVisible());
	m_toolRunBtn->setChecked(tme->running());
}

void ToolMenuItem::enterEvent(QEvent *event)
{
#ifndef __ANDROID__
	setDynamicProperty(this, "allowHover", true);
	event->accept();
#endif
}

void ToolMenuItem::leaveEvent(QEvent *event)
{
#ifndef __ANDROID__
	setDynamicProperty(this, "allowHover", false);
	event->accept();
#endif
}

QString ToolMenuItem::getId() const { return m_uuid; }

void ToolMenuItem::setSelected(bool en)
{
	if(!en) {
		m_toolBtn->setChecked(false);
	}
	setDynamicProperty(this, "selected", en);
}

#include "moc_toolmenuitem.cpp"
