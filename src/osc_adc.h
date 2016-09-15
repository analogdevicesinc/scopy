#ifndef OSC_ADC_H
#define OSC_ADC_H

#include <QList>
#include <QStringList>

extern "C" {
	struct iio_context;
	struct iio_device;
	struct iio_channel;
}

namespace adiscope {

	class Filter;

	class OscADC
	{
	public:
		OscADC(struct iio_context *, const Filter *);
		~OscADC();

		double sampleRate();
		void setSampleRate(double);
		QList<double>& availSamplRates();
		unsigned int numChannels();

		struct iio_device *iio_adc();

	private:
		static unsigned int get_nb_channels(struct iio_device *dev);
		static QStringList get_avail_options_list(struct iio_device *dev,
					const char *attr_name);

	private:
		struct iio_context *m_ctx;
		struct iio_device *m_adc;
		double m_sampleRate;
		QList<double > m_availSampRates;
		unsigned int m_numChannels;
	};
}

#endif // OSC_ADC_H
