#pragma once

#include <QObject>
#include <QString>

namespace scopy::component {

// Identity component: a channel within a device. Plain QObject — its attributes
// are composed under it via Qt parent-child. Backend-agnostic: holds only shared
// identity (id/name/label + direction). Concrete backends subclass this
// (IIOChannel, ...) to carry backend handles; the base holds no backend types.
// Shared identity is exposed through Q_PROPERTY for uniform client binding.
class Channel : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString id READ id CONSTANT)
	Q_PROPERTY(QString name READ name CONSTANT)
	Q_PROPERTY(QString label READ label CONSTANT)
	Q_PROPERTY(bool isOutput READ isOutput CONSTANT)
public:
	explicit Channel(QObject *parent = nullptr)
		: QObject(parent)
	{
	}
	~Channel() override = default;

	QString id() const { return m_id; }
	QString name() const { return m_name; }
	QString label() const { return m_label; }
	bool isOutput() const { return m_isOutput; }

	void setId(const QString &id)
	{
		m_id = id;
		setObjectName(id);
	}
	void setName(const QString &name) { m_name = name; }
	void setLabel(const QString &label) { m_label = label; }
	void setIsOutput(bool isOutput) { m_isOutput = isOutput; }

protected:
	QString m_id;
	QString m_name;
	QString m_label;
	bool m_isOutput = false;
};

} // namespace scopy::component
