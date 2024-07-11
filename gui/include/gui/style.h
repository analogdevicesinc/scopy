#ifndef STYLEREPOSITORY_H
#define STYLEREPOSITORY_H

#include <QColor>
#include <QObject>
#include "scopy-gui_export.h"
#include <style_properties.h>
#include <style_attributes.h>

namespace scopy {
class SCOPY_GUI_EXPORT Style : public QObject
{
	Q_OBJECT
private:
	Style(QObject *parent = nullptr);
	~Style();

	void applyStyle();
	void init(QString theme = "");

public:
	static Style *GetInstance();
	static QString getAttribute(const char *key);
	static QColor getColor(const char *key);
	static int getDimension(const char *key);
	static void setStyle(QWidget *widget, const char *style);
	static QPixmap getPixmap(QString pixmap, QColor color = QColor());
	QString getTheme();
	bool setTheme(QString theme);
	QStringList getThemeList();

protected:
	static QString replaceAttribute(QString style);

private:
	static Style *pinstance_;
	static QJsonDocument *m_global_json;
	static QJsonDocument *m_theme_json;
	QString m_globalJsonPath;
	QString m_themeJsonPath;
	QString m_qssPath;
};
} // namespace scopy

#endif // STYLEREPOSITORY_H
