#ifndef INFOPAGE_H
#define INFOPAGE_H

#include "infopagekeyvaluewidget.h"
#include "scopy-gui_export.h"

#include <QMap>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

namespace scopy {

class SCOPY_GUI_EXPORT InfoPage : public QWidget
{
	Q_OBJECT
public:
	InfoPage(QWidget *parent = nullptr);
	~InfoPage();

	void update(QString key, QString value);
	void clear();

	void setAdvancedMode(bool newAdvancedMode);

private:
	void getKeyMap();

	bool m_advancedMode;
	QVBoxLayout *lay;
	QMap<QString, QString> keyMap;
	QMap<QString, QString> map;
	QMap<QString, InfoPageKeyValueWidget *> wmap;
};

} // namespace scopy
#endif // INFOPAGE_H
