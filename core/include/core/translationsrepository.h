#ifndef TRANSLATIONS_CONTROLLER
#define TRANSLATIONS_CONTROLLER

#include <QObject>
#include "scopy-core_export.h"

namespace scopy {
class SCOPY_CORE_EXPORT TranslationsRepository : public QObject
{
	Q_OBJECT
protected:
	TranslationsRepository(QObject *parent = nullptr);
	~TranslationsRepository();

public:
	static TranslationsRepository *GetInstance();
	QString getTranslationsPath();
	QStringList getLanguages();
	void loadTranslations(QString language);

private:
	static TranslationsRepository * pinstance_;
};
}

#endif // TRANSLATIONS_CONTROLLER
