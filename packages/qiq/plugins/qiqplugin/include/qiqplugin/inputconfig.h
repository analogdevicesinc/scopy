#ifndef INPUTCONFIG_H
#define INPUTCONFIG_H

#include <QString>
#include <QStringList>
#include <QVariantMap>

namespace scopy::qiqplugin {

class InputConfig
{

public:
	InputConfig();
	~InputConfig();

	bool isValid();
	void fromVariantMap(const QVariantMap &params);
	QVariantMap toVariantMap();

	int sampleCount() const;
	void setSampleCount(int newSampleCount);

	int channelCount() const;
	void setChannelCount(int newChannelCount);

	double samplingFrequency() const;
	void setSamplingFrequency(double newSamplingFrequency);

	double frequencyOffset() const;
	void setFrequencyOffset(double newFrequencyOffset);

	QString inputFileFormat() const;
	void setInputFileFormat(const QString &newInputFileFormat);

	QStringList chnlsFormat() const;
	void setChnlsFormat(const QStringList &newChnlsFormat);

	QString inputFile() const;
	void setInputFile(const QString &newInputFile);

private:
	int m_sampleCount;
	int m_channelCount;
	double m_samplingFrequency;
	double m_frequencyOffset;
	QString m_inputFile;
	QString m_inputFileFormat;
	QStringList m_chnlsFormat;
};

} // namespace scopy::qiqplugin

#endif // INPUTCONFIG_H
