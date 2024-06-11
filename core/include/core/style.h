#ifndef STYLEREPOSITORY_H
#define STYLEREPOSITORY_H

#include <QObject>
#include "scopy-core_export.h"

namespace scopy {
class SCOPY_CORE_EXPORT Style : public QObject
{
	Q_OBJECT
public:
	Style(QObject *parent = nullptr);
	~Style();

	void init();

	static QString getAttribute(char *key);
	static QColor getColor(char *key);
	static int getDimension(char *key);
	static void setStyle(QWidget *widget, char *style);

protected:
	void applyStyle();

private:
	static QJsonDocument *m_json;
	QString m_jsonPath;
	QString m_qssPath;
};
} // namespace scopy

#endif // STYLEREPOSITORY_H
