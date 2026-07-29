#pragma once

#include "core/icmdexecutor.h"

#include <QObject>
#include <QString>
#include <QUuid>

namespace scopy::component {

// Identity component: the root of one connection's tree. Plain QObject — the
// whole subtree is composed under it via Qt parent-child, so disconnecting
// (deleting the Context) cascades destruction to every child.
//
// The Context owns the per-context ICmdExecutor (Qt ownership): the Controller
// creates the executor parented to the Context, hands it to the builder for
// injection into I/O leaves, and it dies with the Context on disconnect.
//
// Concrete backends subclass this (IIOContext, ...) to carry backend-specific
// identity fields. The base carries only what every backend shares. Shared
// identity is exposed through Q_PROPERTY so clients can bind to it uniformly.
class Context : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString uri READ uri CONSTANT)
	Q_PROPERTY(QString name READ name CONSTANT)
	Q_PROPERTY(QString description READ description CONSTANT)
public:
	explicit Context(QObject *parent = nullptr)
		: QObject(parent)
	{
	}
	~Context() override = default;

	QString uri() const { return m_uri; }
	QString name() const { return m_name; }
	QString description() const { return m_description; }

	void setUri(const QString &uri) { m_uri = uri; }
	void setName(const QString &name)
	{
		m_name = name;
		setObjectName(name);
	}
	void setDescription(const QString &description) { m_description = description; }

	// The executor driving all I/O in this context's subtree. Owned by the
	// Context (parented to it); the builder injects it into I/O leaves.
	ICmdExecutor *executor() const { return m_executor; }
	void setExecutor(ICmdExecutor *exec) { m_executor = exec; }

	// Cancellation, delegated to this context's executor. cancelCommandById cancels
	// one queued command by its QUuid (the id returned by readAsync/writeAsync);
	// cancelAllCommands cancels every command still pending in this context.
	// Cooperative and pre-execution only — a command already running on a worker
	// cannot be stopped.
	void cancelCommandById(const QUuid &id)
	{
		if(m_executor) {
			m_executor->cancelById(id);
		}
	}
	void cancelAllCommands()
	{
		if(m_executor) {
			m_executor->cancelAll();
		}
	}

protected:
	QString m_uri;
	QString m_name;
	QString m_description;
	ICmdExecutor *m_executor = nullptr;
};

} // namespace scopy::component
