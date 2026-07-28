#pragma once

#include "core/result.h"
#include <QObject>
#include <QUuid>

namespace scopy {

class Command : public QObject
{
	Q_OBJECT
public:
	Command(void *resource, QObject *parent = nullptr)
		: QObject(parent)
		, m_id(QUuid::createUuid())
		, m_resource(resource)
	{
	}
	virtual ~Command() = default;

	virtual void execute()
	{
		Q_EMIT started(this);
		if(!m_cancelled) {
            run();
		}
		Q_EMIT finished(this);
	}

    virtual QString toString() const
    {
        return QString::fromLatin1(metaObject()->className()) + QStringLiteral("(id=") +
            m_id.toString(QUuid::WithoutBraces) + QLatin1Char(')');
    }

	QUuid id() const { return m_id; }
	void *resource() const { return m_resource; }
	void cancel() { m_cancelled = true; }
	bool isCancelled() const { return m_cancelled; }

Q_SIGNALS:
    void started(scopy::Command *cmd);
    void finished(scopy::Command *cmd);

protected:
    virtual void run() = 0;

	QUuid m_id;
	void *m_resource;
	bool m_cancelled = false;
};

} // namespace scopy
