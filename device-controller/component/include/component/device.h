#pragma once

#include <QObject>
#include <QString>

namespace scopy::component {

// Identity component: a device within a context. Plain QObject — its attributes
// and channels are composed under it via Qt parent-child. Backend-agnostic:
// holds only shared identity (id/name/label). Concrete backends subclass this
// (IIODevice, ...) to carry backend handles; the base holds no backend types.
// Shared identity is exposed through Q_PROPERTY for uniform client binding.
class Device : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString id READ id CONSTANT)
	Q_PROPERTY(QString name READ name CONSTANT)
	Q_PROPERTY(QString label READ label CONSTANT)
public:
	explicit Device(QObject *parent = nullptr)
		: QObject(parent)
	{
	}
	~Device() override = default;

	QString id() const { return m_id; }
	QString name() const { return m_name; }
	QString label() const { return m_label; }

	void setId(const QString &id) { m_id = id; }
	void setName(const QString &name)
	{
		m_name = name;
		setObjectName(name);
	}
	void setLabel(const QString &label) { m_label = label; }

protected:
	QString m_id;
	QString m_name;
	QString m_label;
};

} // namespace scopy::component
