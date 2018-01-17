#ifndef HW_DAC_H
#define HW_DAC_H

#include <qglobal.h>
#include <QList>
#include <map>
#include <memory>

extern "C" {
	struct iio_context;
	struct iio_device;
	struct iio_channel;
}

namespace adiscope {

class GenericDac
{
public:
	GenericDac(struct iio_context *ctx, struct iio_device *dac_dev);
	virtual ~GenericDac();

	struct iio_context * iio_context() const;
	struct iio_device * iio_dac_dev() const;

	uint numDacChannels() const;
	QList<struct iio_channel *> dacChannelList() const;
	uint numDacBits() const;

	double sampleRate() const;
	virtual double readSampleRate();
	virtual void setSampleRate(double);
	virtual size_t maxNumberOfSamples();
	virtual void setMaxNumberOfSamples(size_t val);

	virtual double vlsb() const;
	virtual void setVlsb(double vlsb);
	virtual double convVoltsToSample(double);
	virtual double vOutL() const;
	virtual void setVOutL(double value);
	virtual double vOutH() const;
	virtual void setVOutH(double value);

private:
	struct iio_context *m_ctx;
	struct iio_device *m_dac;
	QList<struct iio_channel *> m_dac_channels;
	uint m_dac_bits;
	size_t m_maxNumberOfSamples;
	double m_sample_rate;
	double m_vOutL;
	double m_vOutH;
};

class M2kDac: public GenericDac
{
public:
	M2kDac(struct iio_context *, struct iio_device *dac_dev);
	~M2kDac();

	double compTable(double samplRate) const;

	virtual double vlsb() const;
	virtual void setVlsb(double vlsb);
	virtual double convVoltsToSample(double);

private:
	std::map<double, double> m_filt_comp_table;
	double m_vlsb;
};

class DacBuilder
{
public:
	enum DacType {
		GENERIC = 0,
		M2K = 1,
	};

	static std::shared_ptr<GenericDac> newDac(DacType dac_type,
		struct iio_context *ctx, struct iio_device *dac)
	{
		switch (dac_type) {
		case GENERIC: return std::make_shared<GenericDac>(ctx, dac);
		case M2K: return std::make_shared<M2kDac>(ctx, dac);
		}
	}
};

} /* namespace adiscope */

#endif // HW_DAC_H
