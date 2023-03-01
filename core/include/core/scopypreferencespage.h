#ifndef SCOPYPREFERENCESPAGE_H
#define SCOPYPREFERENCESPAGE_H

#include <QTabWidget>
#include "scopycore_export.h"

namespace adiscope {
class SCOPYCORE_EXPORT ScopyPreferencesPage : public QTabWidget
{
	Q_OBJECT
public:
	ScopyPreferencesPage(QWidget *parent = nullptr);
	~ScopyPreferencesPage();
	void addHorizontalTab(QWidget *w, QString text);
};
}
#endif // SCOPYPREFERENCESPAGE_H
