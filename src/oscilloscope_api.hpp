#ifndef OSCILLOSCOPE_API_HPP
#define OSCILLOSCOPE_API_HPP

#include <oscilloscope.hpp>

namespace adiscope {

class ApiObject;
class Oscilloscope;
/**
  * @brief osc object
  */
class Oscilloscope_API : public ApiObject
{
	Q_OBJECT


	Q_PROPERTY(QVariantList channels READ getChannels);

	Q_PROPERTY(bool running READ running WRITE run STORED false);
	Q_PROPERTY(bool single READ isSingle WRITE single STORED false);

	/**
	  * @brief Enables the cursors
	  */
	Q_PROPERTY(bool cursors READ hasCursors WRITE setCursors);
	Q_PROPERTY(bool measure READ hasMeasure WRITE setMeasure);
	Q_PROPERTY(bool measure_all
			READ measureAll WRITE setMeasureAll);
	Q_PROPERTY(bool counter READ hasCounter WRITE setCounter);
	Q_PROPERTY(bool statistics
			READ hasStatistics WRITE setStatistics);

	Q_PROPERTY(bool horizontal_cursors READ horizontalCursors
			WRITE setHorizontalCursors)
	Q_PROPERTY(bool vertical_cursors READ verticalCursors
			WRITE setVerticalCursors)

	Q_PROPERTY(double cursor_v1 READ cursorV1 WRITE setCursorV1);
	Q_PROPERTY(double cursor_v2 READ cursorV2 WRITE setCursorV2);
	Q_PROPERTY(double cursor_h1 READ cursorH1 WRITE setCursorH1);
	Q_PROPERTY(double cursor_h2 READ cursorH2 WRITE setCursorH2);
	Q_PROPERTY(int cursors_position READ getCursorsPosition
		  WRITE setCursorsPosition)
	Q_PROPERTY(int cursors_transparency READ getCursorsTransparency
		  WRITE setCursorsTransparency)

	Q_PROPERTY(bool gating_enabled READ gatingEnabled WRITE setGatingEnabled);
	Q_PROPERTY(double cursor_gateLeft READ cursorGateLeft WRITE setCursorGateLeft);
	Q_PROPERTY(double cursor_gateRight READ cursorGateRight WRITE setCursorGateRight);

	Q_PROPERTY(bool auto_trigger
			READ autoTrigger WRITE setAutoTrigger)
	Q_PROPERTY(bool internal_trigger
			READ internalTrigger WRITE setInternalTrigger)
	Q_PROPERTY(bool external_trigger
			READ externalTrigger WRITE setExternalTrigger)
	Q_PROPERTY(int trigger_source
			READ triggerSource WRITE setTriggerSource)
	Q_PROPERTY(double trigger_level
			READ getTriggerLevel WRITE setTriggerLevel)
	Q_PROPERTY(double trigger_hysteresis READ getTriggerHysteresis
			WRITE setTriggerHysteresis)
	Q_PROPERTY(bool internal_condition READ internalCondition
			WRITE setInternalCondition)
	Q_PROPERTY(bool external_condition READ externalCondition
			WRITE setExternalCondition)
	Q_PROPERTY(bool trigger_input READ getTriggerInput
			WRITE setTriggerInput STORED false)

	Q_PROPERTY(QList<QString> math_channels
			READ getMathChannels WRITE setMathChannels
			SCRIPTABLE false /* too complex for now */);

	Q_PROPERTY(double time_position
			READ getTimePos WRITE setTimePos);
	Q_PROPERTY(double time_base READ getTimeBase WRITE setTimeBase);

	Q_PROPERTY(QList<int> measure_en
			READ measureEn WRITE setMeasureEn);

	Q_PROPERTY(QList<int> statistic_en
			READ statisticEn WRITE setStatisticEn)

	Q_PROPERTY(int current_channel READ getCurrentChannel
			WRITE setCurrentChannel)

	Q_PROPERTY(bool fft_en READ getFftEn WRITE setFftEn)
	Q_PROPERTY(bool xy_en READ getXyEn WRITE setXyEn)
	Q_PROPERTY(bool hist_en READ getHistEn WRITE setHistEn)
	Q_PROPERTY(bool export_all READ getExportAll
		   WRITE setExportAll)
	Q_PROPERTY(bool autoset_en READ autosetEnabled WRITE enableAutoset)

	Q_PROPERTY(int memory_depth READ getMemoryDepth
		   WRITE setMemoryDepth)

public:
	explicit Oscilloscope_API(Oscilloscope *osc) :
		ApiObject(), osc(osc) {}
	~Oscilloscope_API() {}

	QVariantList getChannels();

	bool running() const;
	void run(bool en);
	bool isSingle() const;
	void single(bool en);

	bool hasCursors() const;
	void setCursors(bool en);

	bool autosetEnabled() const;
	void enableAutoset(bool en);

	bool hasMeasure() const;
	void setMeasure(bool en);

	bool measureAll() const;
	void setMeasureAll(bool en);

	bool hasCounter() const;
	void setCounter(bool en);

	bool hasStatistics() const;
	void setStatistics(bool en);

	bool horizontalCursors() const;
	void setHorizontalCursors(bool en);

	bool verticalCursors() const;
	void setVerticalCursors(bool en);

	double cursorV1() const;
	double cursorV2() const;
	double cursorH1() const;
	double cursorH2() const;
	void setCursorV1(double val);
	void setCursorV2(double val);
	void setCursorH1(double val);
	void setCursorH2(double val);

	bool gatingEnabled() const;
	void setGatingEnabled(bool en);

	double cursorGateLeft() const;
	double cursorGateRight() const;
	void setCursorGateLeft(double val);
	void setCursorGateRight(double val);

	bool autoTrigger() const;
	void setAutoTrigger(bool en);

	bool internalTrigger() const;
	void setInternalTrigger(bool en);

	bool externalTrigger() const;
	void setExternalTrigger(bool en);

	int triggerSource() const;
	void setTriggerSource(int idx);

	double getTriggerLevel() const;
	void setTriggerLevel(double level);

	double getTriggerHysteresis() const;
	void setTriggerHysteresis(double hyst);

	bool getTriggerInput() const;
	void setTriggerInput(bool en);

	int internalCondition() const;
	void setInternalCondition(int cond);

	int externalCondition() const;
	void setExternalCondition(int cond);

	int internExtern() const;
	void setInternExtern(int option);

	QList<QString> getMathChannels() const;
	void setMathChannels(const QList<QString>& list);

	double getTimePos() const;
	void setTimePos(double pos);

	double getTimeBase() const;
	void setTimeBase(double base);

	QList<int> measureEn() const;
	void setMeasureEn(const QList<int>& list);

	QList<int> statisticEn() const;
	void setStatisticEn(const QList<int>& list);

	int getCurrentChannel() const;
	void setCurrentChannel(int chn_id);

	bool getFftEn() const;
	void setFftEn(bool en);

	bool getXyEn() const;
	void setXyEn(bool en);

	bool getHistEn() const;
	void setHistEn(bool en);

	bool getExportAll() const;
	void setExportAll(bool en);

	int getCursorsPosition() const;
	void setCursorsPosition(int val);

	int getCursorsTransparency() const;
	void setCursorsTransparency(int val);

	int getMemoryDepth();
	void setMemoryDepth(int val);

	Q_INVOKABLE void show();

	private:
		Oscilloscope *osc;
	};

	class Channel_API : public ApiObject
	{
		Q_OBJECT

	Q_PROPERTY(bool enabled READ channelEn WRITE setChannelEn)

	Q_PROPERTY(double volts_per_div
			READ getVoltsPerDiv WRITE setVoltsPerDiv)

	Q_PROPERTY(double v_offset READ getVOffset WRITE setVOffset)

	Q_PROPERTY(double line_thickness
			READ getLineThickness WRITE setLineThickness)

	Q_PROPERTY(double probe_attenuation READ getProbeAttenuation
			WRITE setProbeAttenuation)

	Q_PROPERTY(bool ac_coupling READ getAcCoupling
			WRITE setAcCoupling)

	Q_PROPERTY(double period READ measured_period)
	Q_PROPERTY(double frequency READ measured_frequency)
	Q_PROPERTY(double min READ measured_min)
	Q_PROPERTY(double max READ measured_max)
	Q_PROPERTY(double peak_to_peak READ measured_peak_to_peak)
	Q_PROPERTY(double mean READ measured_mean)
	Q_PROPERTY(double cycle_mean READ measured_cycle_mean)
	Q_PROPERTY(double rms READ measured_rms)
	Q_PROPERTY(double cycle_rms READ measured_cycle_rms)
	Q_PROPERTY(double ac_rms READ measured_ac_rms)
	Q_PROPERTY(double area READ measured_area)
	Q_PROPERTY(double cycle_area READ measured_cycle_area)
	Q_PROPERTY(double low READ measured_low)
	Q_PROPERTY(double high READ measured_high)
	Q_PROPERTY(double amplitude READ measured_amplitude)
	Q_PROPERTY(double middle READ measured_middle)
	Q_PROPERTY(double pos_overshoot READ measured_pos_overshoot)
	Q_PROPERTY(double neg_overshoot READ measured_neg_overshoot)
	Q_PROPERTY(double rise READ measured_rise)
	Q_PROPERTY(double fall READ measured_fall)
	Q_PROPERTY(double pos_width READ measured_pos_width)
	Q_PROPERTY(double neg_width READ measured_neg_width)
	Q_PROPERTY(double pos_duty READ measured_pos_duty)
	Q_PROPERTY(double neg_duty READ measured_neg_duty)
	Q_PROPERTY(QList<double> data READ data STORED false)
public:
	explicit Channel_API(Oscilloscope *osc) :
		ApiObject(), osc(osc) {}
	~Channel_API() {}

	bool channelEn() const;
	void setChannelEn(bool en);

	double getVoltsPerDiv() const;
	void setVoltsPerDiv(double val);

	double getVOffset() const;
	void setVOffset(double val);

	double getLineThickness() const;
	void setLineThickness(double val);

	double getProbeAttenuation() const;
	void setProbeAttenuation(double val);

	bool getAcCoupling() const;
	void setAcCoupling(bool val);

	double measured_period() const;
	double measured_frequency() const;
	double measured_min() const;
	double measured_max() const;
	double measured_peak_to_peak() const;
	double measured_mean() const;
	double measured_cycle_mean() const;
	double measured_rms() const;
	double measured_cycle_rms() const;
	double measured_ac_rms() const;
	double measured_area() const;
	double measured_cycle_area() const;
	double measured_low() const;
	double measured_high() const;
	double measured_amplitude() const;
	double measured_middle() const;
	double measured_pos_overshoot() const;
	double measured_neg_overshoot() const;
	double measured_rise() const;
	double measured_fall() const;
	double measured_pos_width() const;
	double measured_neg_width() const;
	double measured_pos_duty() const;
	double measured_neg_duty() const;
	QList<double> data() const;

	Q_INVOKABLE void setColor(int, int, int, int a = 255);

private:
	Oscilloscope *osc;
};
}

#endif // OSCILLOSCOPE_API_HPP
