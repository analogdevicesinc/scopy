#ifndef POWER_CONTROLLER_API_HPP
#define POWER_CONTROLLER_API_HPP

#include "power_controller.hpp"

namespace adiscope {

class PowerController_API : public ApiObject {
	Q_OBJECT

	Q_PROPERTY(bool sync READ syncEnabled WRITE enableSync);
	Q_PROPERTY(int tracking_percent READ getTrackingPercent WRITE
			   setTrackingPercent);
	Q_PROPERTY(double dac1_value READ valueDac1 WRITE setValueDac1);
	Q_PROPERTY(double dac2_value READ valueDac2 WRITE setValueDac2);
	Q_PROPERTY(bool dac1_enabled READ Dac1Enabled WRITE setDac1Enabled);
	Q_PROPERTY(bool dac2_enabled READ Dac2Enabled WRITE setDac2Enabled);

public:
	explicit PowerController_API(PowerController *pw)
		: ApiObject(), pw(pw) {}
	~PowerController_API() {}

	bool syncEnabled() const;
	void enableSync(bool en);

	int getTrackingPercent() const;
	void setTrackingPercent(int percent);

	double valueDac1() const;
	void setValueDac1(double value);

	double valueDac2() const;
	void setValueDac2(double value);

	bool Dac1Enabled() const;
	void setDac1Enabled(bool enable);

	bool Dac2Enabled() const;
	void setDac2Enabled(bool enable);

	Q_INVOKABLE void show();

private:
	PowerController *pw;
};
} // namespace adiscope

#endif // POWER_CONTROLLER_API_HPP
