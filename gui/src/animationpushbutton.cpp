#include "animationpushbutton.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_GUI_ANIMATION_BTN, "AnimationPushButton")

using namespace scopy;

AnimationPushButton::AnimationPushButton(QWidget *parent)
	: QPushButton(parent)
	, m_animation(nullptr)
{}

AnimationPushButton::~AnimationPushButton()
{
	if (m_animation) {
		disconnect(m_animation, &QMovie::frameChanged, this, &AnimationPushButton::setBtnIcon);
	}
}

void AnimationPushButton::setAnimation(QMovie *animation)
{
	if (animation->isValid()) {
		if (m_animation) {
			disconnect(m_animation, &QMovie::frameChanged, this, &AnimationPushButton::setBtnIcon);
		}
		m_animation = animation;
		connect(m_animation, &QMovie::frameChanged, this, &AnimationPushButton::setBtnIcon);
	} else {
		qWarning(CAT_GUI_ANIMATION_BTN) << "The animation " << animation->fileName() + " is not valid!";
	}
}

void AnimationPushButton::startAnimation()
{
	m_currentText = this->text();
	m_currentIcon = this->icon();
	if (m_animation->isValid() && (m_animation->state() == QMovie::NotRunning)) {
		m_animation->start();
		this->setText("");
		this->setEnabled(false);
	}
}

void AnimationPushButton::stopAnimation()
{
	if (m_animation->isValid() && (m_animation->state() == QMovie::Running)) {
		m_animation->stop();
		this->setIcon(m_currentIcon);
		this->setText(m_currentText);
		this->setEnabled(true);
	}
}

void AnimationPushButton::setBtnIcon()
{
	this->setIcon(m_animation->currentPixmap());
}

#include "moc_animationpushbutton.cpp"
