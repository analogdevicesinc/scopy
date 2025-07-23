#ifndef OUTPUTINFO_H
#define OUTPUTINFO_H

#include <QVariantMap>

namespace scopy::qiqplugin {

class OutputInfo
{
public:
	OutputInfo();

	void fromVariantMap(const QVariantMap &params);
	QVariantMap toVariantMap() const;

	int channelCount() const;
	void setChannelCount(int count);

	QStringList channelNames() const;
	void setChannelNames(const QStringList &names);

	QStringList channelFormat() const;
	void setChannelFormat(const QStringList &format);

	bool isValid();

private:
	int m_channelCount;
	QStringList m_channelNames;
	QStringList m_channelFormat;
};

} // namespace scopy::qiqplugin

#endif // OUTPUTINFO_H
