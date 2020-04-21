#ifndef SIGNAL_GENERATOR_API_HPP
#define SIGNAL_GENERATOR_API_HPP

#include "signal_generator.hpp"

namespace adiscope {

struct signal_generator_data;
class SignalGenerator_API : public ApiObject {
	Q_OBJECT

	Q_PROPERTY(bool running READ running WRITE run STORED false);

	Q_PROPERTY(QList<int> mode READ getMode WRITE setMode);
	Q_PROPERTY(QList<bool> enabled READ enabledChannels WRITE enableChannels)
	Q_PROPERTY(QList<double> constant_volts READ getConstantValue WRITE setConstantValue);
	Q_PROPERTY(QList<int> waveform_type READ getWaveformType WRITE setWaveformType);
	Q_PROPERTY(QList<double> waveform_amplitude READ getWaveformAmpl WRITE setWaveformAmpl);
	Q_PROPERTY(QList<double> waveform_frequency READ getWaveformFreq WRITE setWaveformFreq);
	Q_PROPERTY(QList<double> waveform_offset READ getWaveformOfft WRITE setWaveformOfft);
	Q_PROPERTY(QList<double> waveform_phase READ getWaveformPhase WRITE setWaveformPhase);
	Q_PROPERTY(QList<double> math_frequency READ getMathFreq WRITE setMathFreq);
	Q_PROPERTY(QList<QString> math_function READ getMathFunction WRITE setMathFunction);
	Q_PROPERTY(QList<double> waveform_duty READ getWaveformDuty WRITE setWaveformDuty);

	Q_PROPERTY(QList<int> noise_type READ getNoiseType WRITE setNoiseType);
	Q_PROPERTY(QList<double> noise_amplitude READ getNoiseAmpl WRITE setNoiseAmpl);
	Q_PROPERTY(QList<double> waveform_rise READ getWaveformRise WRITE setWaveformRise);
	Q_PROPERTY(QList<double> waveform_fall READ getWaveformFall WRITE setWaveformFall);
	Q_PROPERTY(QList<double> waveform_holdhigh READ getWaveformHoldHigh WRITE setWaveformHoldHigh);
	Q_PROPERTY(QList<double> waveform_holdlow READ getWaveformHoldLow WRITE setWaveformHoldLow);

	Q_PROPERTY(QList<QString> buffer_file_path READ getBufferFilePath WRITE setBufferFilePath)
	Q_PROPERTY(QList<double> buffer_amplitude READ getBufferAmplitude WRITE setBufferAmplitude)
	Q_PROPERTY(QList<double> buffer_offset READ getBufferOffset WRITE setBufferOffset)
	Q_PROPERTY(QList<double> buffer_sample_rate READ getBufferSampleRate WRITE setBufferSampleRate)
	Q_PROPERTY(QList<double> buffer_phase READ getBufferPhase WRITE setBufferPhase)

public:
	bool running() const;
	void run(bool en);

	QList<int> getMode() const;
	void setMode(const QList<int> &list);

	QList<bool> enabledChannels() const;
	void enableChannels(const QList<bool> &list);

	QList<double> getConstantValue() const;
	void setConstantValue(const QList<double> &list);

	QList<int> getWaveformType() const;
	void setWaveformType(const QList<int> &list);

	QList<double> getWaveformAmpl() const;
	void setWaveformAmpl(const QList<double> &list);

	QList<double> getWaveformFreq() const;
	void setWaveformFreq(const QList<double> &list);

	QList<double> getWaveformOfft() const;
	void setWaveformOfft(const QList<double> &list);

	QList<double> getWaveformPhase() const;
	void setWaveformPhase(const QList<double> &list);

	QList<double> getWaveformDuty() const;
	void setWaveformDuty(const QList<double> &list);

	QList<int> getNoiseType() const;
	void setNoiseType(const QList<int> &list);
	QList<double> getNoiseAmpl() const;
	void setNoiseAmpl(const QList<double> &list);

	QList<double> getWaveformHoldLow() const;
	void setWaveformHoldLow(const QList<double> &list);
	QList<double> getWaveformHoldHigh() const;
	void setWaveformHoldHigh(const QList<double> &list);
	QList<double> getWaveformFall() const;
	void setWaveformFall(const QList<double> &list);
	QList<double> getWaveformRise() const;
	void setWaveformRise(const QList<double> &list);

	QList<double> getMathFreq() const;
	void setMathFreq(const QList<double> &list);

	QList<QString> getMathFunction() const;
	void setMathFunction(const QList<QString> &list);

	QList<QString> getBufferFilePath() const;
	void setBufferFilePath(const QList<QString> &list);
	QList<double> getBufferAmplitude() const;
	void setBufferAmplitude(const QList<double> &list);
	QList<double> getBufferOffset() const;
	void setBufferOffset(const QList<double> &list);
	QList<double> getBufferSampleRate() const;
	void setBufferSampleRate(const QList<double> &list);
	QList<double> getBufferPhase() const;
	void setBufferPhase(const QList<double> &list);

	Q_INVOKABLE void show();

	explicit SignalGenerator_API(SignalGenerator *gen) : ApiObject(), gen(gen) {}
	~SignalGenerator_API() {}

private:
	SignalGenerator *gen;
};
} // namespace adiscope

#endif // SIGNAL_GENERATOR_API_HPP
