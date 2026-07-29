#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace scopy::component {

namespace tags {
inline constexpr auto SampleRate = "sample-rate";
inline constexpr auto TriggerLevel = "trigger-level";
inline constexpr auto Offset = "offset";
inline constexpr auto Enable = "enable";
} // namespace tags

inline QStringList componentTags(const QObject *o) { return o->property("tags").toStringList(); }

inline bool hasTag(const QObject *o, const QString &tag) { return componentTags(o).contains(tag); }

inline void addTag(QObject *o, const QString &tag)
{
	QStringList t = componentTags(o);
	if(!t.contains(tag)) {
		t.append(tag);
		o->setProperty("tags", t);
	}
}

template <typename T>
QList<T *> findByTag(const QObject *root, const QString &tag, bool recursive = true)
{
	const auto opt = recursive ? Qt::FindChildrenRecursively : Qt::FindDirectChildrenOnly;
	QList<T *> out;
	for(T *c : root->findChildren<T *>(QString(), opt)) {
		if(hasTag(c, tag)) {
			out.append(c);
		}
	}
	return out;
}

inline QList<QObject *> findByTag(const QObject *root, const QString &tag, bool recursive = true)
{
	return findByTag<QObject>(root, tag, recursive);
}

} // namespace scopy::component
