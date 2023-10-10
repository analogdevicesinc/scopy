#ifndef ANIMATIONPUSHBUTTON_H
#define ANIMATIONPUSHBUTTON_H

#include "scopy-gui_export.h"

#include <QMovie>
#include <QPushButton>

namespace scopy {
class SCOPY_GUI_EXPORT AnimationPushButton : public QPushButton
{
	Q_OBJECT
public:
	explicit AnimationPushButton(QWidget *parent = Q_NULLPTR);
	~AnimationPushButton();

	void setAnimation(QMovie *animation);

public Q_SLOTS:
	void startAnimation();
	void stopAnimation();

private Q_SLOTS:
	void setBtnIcon();

private:
	QMovie *m_animation;
	QString m_currentText;
	QIcon m_currentIcon;
};
} // namespace scopy

#endif // ANIMATIONPUSHBUTTON_H
