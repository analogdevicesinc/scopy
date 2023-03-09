/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SIGNAL_GENERATOR_API_HPP
#define SIGNAL_GENERATOR_API_HPP

#include "signal_generator.hpp"

namespace adiscope {

struct signal_generator_data;
class SignalGenerator_API : public ApiObject
{
	Q_OBJECT

	Q_PROPERTY(bool running READ running WRITE run STORED false);

	Q_PROPERTY(QList<int> mode READ getMode WRITE setMode);
	Q_PROPERTY(QList<bool> enabled
	       READ enabledChannels WRITE enableChannels)
	Q_PROPERTY(QList<double> constant_volts
	           READ getConstantValue WRITE setConstantValue);
	Q_PROPERTY(QList<int> waveform_type
	           READ getWaveformType WRITE setWaveformType);
	Q_PROPERTY(QList<double> waveform_amplitude
	           READ getWaveformAmpl WRITE setWaveformAmpl);
	Q_PROPERTY(QList<double> waveform_frequency
	           READ getWaveformFreq WRITE setWaveformFreq);
	Q_PROPERTY(QList<double> waveform_offset
	           READ getWaveformOfft WRITE setWaveformOfft);
	Q_PROPERTY(QList<double> waveform_phase
	           READ getWaveformPhase WRITE setWaveformPhase);
	Q_PROPERTY(QList<double> math_record_length
			   READ getMathRecordLength WRITE setMathRecordLength);
	Q_PROPERTY(QList<double> math_sample_rate
			   READ getMathSampleRate WRITE setMathSampleRate);
	Q_PROPERTY(QList<QString> math_function
	           READ getMathFunction WRITE setMathFunction);
	Q_PROPERTY(QList<double> waveform_duty
		   READ getWaveformDuty WRITE setWaveformDuty);

	Q_PROPERTY(QList<int> steps_up
		   READ getStairWaveformStepsUp WRITE setStairWaveformStepsUp);
	Q_PROPERTY(QList<int> steps_down
		   READ getStairWaveformStepsDown WRITE setStairWaveformStepsDown);
	Q_PROPERTY(QList<int> stairphase
		   READ getStairWaveformPhase WRITE setStairWaveformPhase);



	Q_PROPERTY(QList<int> noise_type
		   READ getNoiseType WRITE setNoiseType);
	Q_PROPERTY(QList<double> noise_amplitude
		   READ getNoiseAmpl WRITE setNoiseAmpl);
	Q_PROPERTY(QList<double> waveform_rise
		   READ getWaveformRise WRITE setWaveformRise);
	Q_PROPERTY(QList<double> waveform_fall
		   READ getWaveformFall WRITE setWaveformFall);
	Q_PROPERTY(QList<double> waveform_holdhigh
		   READ getWaveformHoldHigh WRITE setWaveformHoldHigh);
	Q_PROPERTY(QList<double> waveform_holdlow
		   READ getWaveformHoldLow WRITE setWaveformHoldLow);

	Q_PROPERTY(QList<QString> buffer_file_path
		   READ getBufferFilePath WRITE setBufferFilePath)
	Q_PROPERTY(QList<double> buffer_amplitude
		   READ getBufferAmplitude WRITE setBufferAmplitude)
	Q_PROPERTY(QList<double> buffer_offset
		   READ getBufferOffset WRITE setBufferOffset)
	Q_PROPERTY(QList<double> buffer_sample_rate
		   READ getBufferSampleRate WRITE setBufferSampleRate)
	Q_PROPERTY(QList<double> buffer_phase
		   READ getBufferPhase WRITE setBufferPhase)

	Q_PROPERTY(QString notes READ getNotes WRITE setNotes)

        Q_PROPERTY(QList<int> line_thickness
                   READ getLineThickness WRITE setLineThickness)

	Q_PROPERTY(bool autoscale READ getAutoscale WRITE setAutoscale);
	Q_PROPERTY(QList<double> load READ getLoad WRITE setLoad);


public:
	bool running() const;
	void run(bool en);

	QList<int> getMode() const;
	void setMode(const QList<int>& list);

	QList<bool> enabledChannels() const;
	void enableChannels(const QList<bool>& list);

	QList<double> getConstantValue() const;
	void setConstantValue(const QList<double>& list);

	QList<int> getWaveformType() const;
	void setWaveformType(const QList<int>& list);

	QList<double> getWaveformAmpl() const;
	void setWaveformAmpl(const QList<double>& list);

	QList<double> getWaveformFreq() const;
	void setWaveformFreq(const QList<double>& list);

	QList<double> getWaveformOfft() const;
	void setWaveformOfft(const QList<double>& list);

	QList<double> getWaveformPhase() const;
	void setWaveformPhase(const QList<double>& list);

	QList<double> getWaveformDuty() const;
	void setWaveformDuty(const QList<double>& list);

	QList<int> getNoiseType() const;
	void setNoiseType(const QList<int>& list);
	QList<double> getNoiseAmpl() const;
	void setNoiseAmpl(const QList<double>& list);

	QList<double> getWaveformHoldLow() const;
	void setWaveformHoldLow(const QList<double>& list);
	QList<double> getWaveformHoldHigh() const;
	void setWaveformHoldHigh(const QList<double>& list);
	QList<double> getWaveformFall() const;
	void setWaveformFall(const QList<double>& list);
	QList<double> getWaveformRise() const;
	void setWaveformRise(const QList<double>& list);
	QList<int> getStairWaveformStepsUp() const;
	void setStairWaveformStepsUp(const QList<int>& list);
	QList<int> getStairWaveformStepsDown() const;
	void setStairWaveformStepsDown(const QList<int>& list);
	QList<int> getStairWaveformPhase() const;
	void setStairWaveformPhase(const QList<int>& list);


	QList<double> getMathRecordLength() const;
	void setMathRecordLength(const QList<double>& list);
	QList<double> getMathSampleRate() const;
	void setMathSampleRate(const QList<double>& list);

	QList<QString> getMathFunction() const;
	void setMathFunction(const QList<QString>& list);

	QList<QString> getBufferFilePath() const;
	void setBufferFilePath(const QList<QString>& list);
	QList<double> getBufferAmplitude() const;
	void setBufferAmplitude(const QList<double>& list);
	QList<double> getBufferOffset() const;
	void setBufferOffset(const QList<double>& list);
	QList<double> getBufferSampleRate() const;
	void setBufferSampleRate(const QList<double>& list);
	QList<double> getBufferPhase() const;
	void setBufferPhase(const QList<double>& list);

	QString getNotes();
	void setNotes(QString str);

	QList<int> getLineThickness() const;
	void setLineThickness(const QList<int>& list);

	QList<double> getLoad() const;
	void setLoad(const QList<double>& list);

	bool getAutoscale() const;
	void setAutoscale(bool checked);



        Q_INVOKABLE void show();

	explicit SignalGenerator_API(SignalGenerator *gen) :
		ApiObject(), gen(gen) {}
	~SignalGenerator_API() {}

private:
	SignalGenerator *gen;
};
}

#endif // SIGNAL_GENERATOR_API_HPP
