#pragma once

#include "core/result.h"
#include <QObject>

namespace scopy {

class Command : public QObject
{
	Q_OBJECT
public:
    enum Type
    {
		AttrRead,
		AttrWrite,
		RegRead,
		RegWrite,
		SetTrigger,
		BufferCreate,
		BufferDestroy,
		BufferCancel,
		BufferRefill,
		BufferPush,
		ChnEnable,
		ChnDisable,
		Ping
	};

	Command(Type type, void *resource, QObject *parent = nullptr)
		: QObject(parent)
		, m_type(type)
		, m_resource(resource)
    {
    }
	virtual ~Command() = default;

	virtual void execute() = 0;
	virtual QString toString() const = 0;

	// NB: no result() on the base — the payload type varies per command, so each
	// concrete command exposes its own typed Result<T> result() (see the commands
	// in iio-utils commands/). The base stays non-templated so the executors keep
	// handling Command* polymorphically.
	Type type() const { return m_type; }
	void *resource() const { return m_resource; }
	void cancel() { m_cancelled = true; }
	bool isCancelled() const { return m_cancelled; }

Q_SIGNALS:
	void started(Command *cmd);
	void finished(Command *cmd);

protected:
	Type m_type;
	void *m_resource;
	bool m_cancelled = false;
};

} // namespace scopy
