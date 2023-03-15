#ifndef SCOPYENVIRONMENT_H
#define SCOPYENVIRONMENT_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include "scopypluginbase_export.h"

namespace adiscope {

class SCOPYPLUGINBASE_EXPORT ScopyEnvironment: public QObject
{
	Q_OBJECT
protected:
	ScopyEnvironment(QObject *parent = nullptr);
	~ScopyEnvironment();

public:
	// singleton
	ScopyEnvironment(ScopyEnvironment &other) = delete;
	void operator=(const ScopyEnvironment &) = delete;
	static ScopyEnvironment *GetInstance();

	/**
	 * @brief init
	 * sets an environment value
	 */
	void set(QString, QVariant);

	/**
	 * @brief get
	 * @return gets preferences value
	 */
	QVariant get(QString);

private:
	QMap<QString,QVariant> p;
	static ScopyEnvironment * pinstance_;
};
}
#endif // SCOPYENVIRONMENT_H
