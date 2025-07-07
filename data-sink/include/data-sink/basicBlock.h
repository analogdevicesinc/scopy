#ifndef BASICBLOCK_H
#define BASICBLOCK_H

#include "scopy-data-sink_export.h"
#include <QMap>
#include <QObject>

namespace scopy::datasink {
class SCOPY_DATA_SINK_EXPORT ChannelDataVector
{
public:
	ChannelDataVector(std::vector<float> data)
		: data(std::move(data))
	{}
	ChannelDataVector(int size = 0) { /*data.reserve(size);*/ }

	~ChannelDataVector() { clear(); };

	void clear()
	{
		data.clear();
		data.shrink_to_fit();
	}

	void setCopiedData(const std::vector<float> &new_data) { data = new_data; }

	std::vector<float> data;
};

class SCOPY_DATA_SINK_EXPORT BlockData : public QMap<uint, ChannelDataVector >
{
public:
	BlockData()
		: QMap()
	{}

	~BlockData()
	{
		clearVectors();
		clear();
	}

	void clearVectors()
	{
		for(auto it = begin(); it != end(); ++it) {
			it.value().clear();
		}
	}
};

class SCOPY_DATA_SINK_EXPORT BasicBlock : public QObject
{
	Q_OBJECT
public:
	BasicBlock(QString name = QString())
		: QObject()
		, m_name(std::move(name))
	{}
	~BasicBlock() {}

	QString getName() { return m_name; }

	void setName(QString name) { m_name = name; }

Q_SIGNALS:
	void newData(ChannelDataVector data, uint ch);

protected:
	QString m_name;
};
} // namespace scopy::datasink

#endif
