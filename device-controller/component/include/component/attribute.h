#pragma once

#include "component/attributereader.h"
#include "component/attributewriter.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

namespace scopy::component {

// Identity + metadata node for one parameter. Performs no I/O itself: reads and
// writes are delegated to AttributeReader / AttributeWriter capability children.
// Writability is "has an AttributeWriter child".
class Attribute : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString cachedValue MEMBER m_cachedValue NOTIFY valueChanged)
	Q_PROPERTY(QString name READ name CONSTANT)
	Q_PROPERTY(QString unit READ unit CONSTANT)
	Q_PROPERTY(QList<double> range READ range CONSTANT)
	Q_PROPERTY(QStringList options READ options CONSTANT)
	Q_PROPERTY(bool hasRange READ hasRange CONSTANT)
	Q_PROPERTY(bool hasOptions READ hasOptions CONSTANT)
	Q_PROPERTY(bool isWritable READ isWritable CONSTANT)
public:
	explicit Attribute(QObject *parent = nullptr)
		: QObject(parent)
	{
	}
	~Attribute() override = default;

	QString name() const { return m_name; }
	QString unit() const { return m_unit; }
	QString cachedValue() const { return m_cachedValue; }
	QList<double> range() const { return m_range; }
	QStringList options() const { return m_options; }
	bool hasRange() const { return !m_range.isEmpty(); }
	bool hasOptions() const { return !m_options.isEmpty(); }

	void setName(const QString &name)
	{
		m_name = name;
		setObjectName(name);
	}
	void setUnit(const QString &unit) { m_unit = unit; }
	void setRange(const QList<double> &range) { m_range = range; }
	void setOptions(const QStringList &options) { m_options = options; }
	void setCachedValue(const QString &value)
	{
		if(m_cachedValue == value) {
			return;
		}
		m_cachedValue = value;
		Q_EMIT valueChanged(value);
	}

	AttributeReader *readCapability() const { return m_read; }
	AttributeWriter *writeCapability() const { return m_write; }
	bool isWritable() const { return m_write != nullptr; }

	// Reparent + wire the capabilities. The Attribute owns the wiring.
	void addReadCapability(AttributeReader *cap)
	{
		m_read = cap;
		cap->setParent(this);
		connect(cap, &AttributeReader::readSucceeded, this, [this](Result<QByteArray> &r) {
			if(r) {
				setCachedValue(QString::fromUtf8(r.value()));
			}
		});
	}
	void addWriteCapability(AttributeWriter *cap)
	{
		m_write = cap;
		cap->setParent(this);
		connect(cap, &AttributeWriter::writeSucceeded, this, [this]() {
			if(m_read) {
				m_read->readAsync();
			}
		});
	}

Q_SIGNALS:
	void valueChanged(const QString &value);

private:
	AttributeReader *m_read = nullptr;
	AttributeWriter *m_write = nullptr;
	QString m_name;
	QString m_unit;
	QString m_cachedValue;
	QList<double> m_range;
	QStringList m_options;
};

} // namespace scopy::component
