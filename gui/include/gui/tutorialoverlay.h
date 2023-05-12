#ifndef TUTORIALOVERLAY_H
#define TUTORIALOVERLAY_H

#include <QWidget>
#include "tintedoverlay.h"
#include "tutorialchapter.h"
#include "scopy-gui_export.h"
#include "ui_tutorial.h"

namespace scopy::gui {

class SCOPY_GUI_EXPORT TutorialOverlay : public QWidget
{
	Q_OBJECT
public:
	explicit TutorialOverlay(QWidget *parent = nullptr);
	~TutorialOverlay();

	TutorialChapter* addChapter(QList<QWidget*> subjects, QString description);
	TutorialChapter* addChapter(QWidget* subject, QString description);
	void addChapter(TutorialChapter* ch);

	const QString &getTitle() const;
	void setTitle(const QString &newTitle);

public Q_SLOTS:
	void start();
	void next();
	void finish();

private:

	void buildUi();

	QList<TutorialChapter*> chapter;
	QWidget *parent;
	TintedOverlay *overlay;
	QList<TintedOverlay*> highlights;
	QString title;
	int cnt;
	Ui_Tutorial *ui;

};
}


#endif // TUTORIALOVERLAY_H
