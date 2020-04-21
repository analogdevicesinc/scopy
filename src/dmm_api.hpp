#ifndef DMM_API_HPP
#define DMM_API_HPP

#include "dmm.hpp"

namespace adiscope {
class DMM_API : public ApiObject {
	friend class ToolLauncher_API;

	Q_OBJECT

	Q_PROPERTY(bool mode_ac_high_ch1 READ get_mode_ac_high_ch1 WRITE
			   set_mode_ac_high_ch1);
	Q_PROPERTY(bool mode_ac_low_ch1 READ get_mode_ac_low_ch1 WRITE
			   set_mode_ac_low_ch1);
	Q_PROPERTY(bool mode_ac_high_ch2 READ get_mode_ac_high_ch2 WRITE
			   set_mode_ac_high_ch2);
	Q_PROPERTY(bool mode_ac_low_ch2 READ get_mode_ac_low_ch2 WRITE
			   set_mode_ac_low_ch2);
	Q_PROPERTY(bool running READ running WRITE run STORED false);

	Q_PROPERTY(bool histogram_ch1 READ get_histogram_ch1 WRITE
			   set_histogram_ch1);
	Q_PROPERTY(bool histogram_ch2 READ get_histogram_ch2 WRITE
			   set_histogram_ch2);

	Q_PROPERTY(int history_ch1_size_idx READ get_history_ch1_size_idx WRITE
			   set_history_ch1_size_idx);
	Q_PROPERTY(int history_ch2_size_idx READ get_history_ch2_size_idx WRITE
			   set_history_ch2_size_idx);

	Q_PROPERTY(double value_ch1 READ read_ch1);
	Q_PROPERTY(double value_ch2 READ read_ch2);
	Q_PROPERTY(bool data_logging_en READ getDataLoggingEn WRITE
			   setDataLoggingEn)
	Q_PROPERTY(double data_logging_timer READ getDataLoggingTimer WRITE
			   setDataLoggingTimer)
	Q_PROPERTY(bool data_logging_append READ getDataLoggingAppend WRITE
			   setDataLoggingAppend)
	Q_PROPERTY(bool peak_hold_en READ getPeakHoldEn WRITE setPeakHoldEn)

public:
	bool get_mode_ac_high_ch1() const;
	bool get_mode_ac_low_ch1() const;
	bool get_mode_ac_high_ch2() const;
	bool get_mode_ac_low_ch2() const;

	void set_mode_ac_high_ch1(bool en);
	void set_mode_ac_low_ch1(bool en);
	void set_mode_ac_high_ch2(bool en);
	void set_mode_ac_low_ch2(bool en);

	bool get_histogram_ch1() const;
	bool get_histogram_ch2() const;
	void set_histogram_ch1(bool en);
	void set_histogram_ch2(bool en);

	int get_history_ch1_size_idx() const;
	int get_history_ch2_size_idx() const;
	void set_history_ch1_size_idx(int idx);
	void set_history_ch2_size_idx(int idx);

	double read_ch1() const;
	double read_ch2() const;

	bool running() const;
	void run(bool en);

	bool getDataLoggingEn() const;
	void setDataLoggingEn(bool);

	double getDataLoggingTimer() const;
	void setDataLoggingTimer(double);

	bool getDataLoggingAppend() const;
	void setDataLoggingAppend(bool);

	bool getPeakHoldEn() const;
	void setPeakHoldEn(bool);

	Q_INVOKABLE void show();

	explicit DMM_API(DMM *dmm) : ApiObject(), dmm(dmm) {}
	~DMM_API() {}

private:
	DMM *dmm;
};
} // namespace adiscope
#endif // DMM_API_HPP
