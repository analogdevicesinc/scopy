/*
 * Copyright (c) 2025 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef ADMTCONTROLLER_H
#define ADMTCONTROLLER_H

#include "scopy-admt_export.h"

#include <iio.h>

#include <QElapsedTimer>
#include <QObject>
#include <QString>

#include <iioutil/connectionprovider.h>
#include <pluginbase/statusbarmanager.h>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <math.h>
#include <vector>

using namespace std;

namespace scopy::admt {
class SCOPY_ADMT_EXPORT ADMTController : public QObject
{
	Q_OBJECT
public:
	ADMTController(QString uri, QObject *parent = nullptr);
	~ADMTController();

	int HAR_MAG_1, HAR_MAG_2, HAR_MAG_3, HAR_MAG_8, HAR_PHASE_1, HAR_PHASE_2, HAR_PHASE_3, HAR_PHASE_8,
		sampleCount = 0;

	QAtomicInt stopStream = false;

	double streamedValue = 0.0;
	QVector<double> streamBufferedValues;
	QMap<QString, double> streamedChannelDataMap;

	QElapsedTimer elapsedStreamTimer;

	vector<double> angle_errors_fft_pre, angle_errors_fft_phase_pre, angle_errors_fft_post,
		angle_errors_fft_phase_post, calibration_samples_sine, calibration_samples_cosine,
		calibration_samples_sine_scaled, calibration_samples_cosine_scaled, angleError, FFTAngleErrorMagnitude,
		FFTAngleErrorPhase, correctedError, FFTCorrectedErrorMagnitude, FFTCorrectedErrorPhase;

	enum Channel
	{
		ROTATION,
		ANGL,
		COUNT,
		TEMPERATURE,
		CHANNEL_COUNT
	};

	enum Device
	{
		ADMT4000,
		TMC5240,
		DEVICE_COUNT
	};

	enum DeviceAttribute
	{
		PAGE,
		SEQUENCER_MODE,
		ANGLE_FILT,
		CONVERSION_MODE,
		H8_CTRL,
		SDP_GPIO_CTRL,
		SDP_GPIO0_BUSY,
		SDP_COIL_RS,
		REGMAP_DUMP,
		DEVICE_ATTR_COUNT
	};

	enum MotorAttribute
	{
		AMAX,
		ROTATE_VMAX,
		DMAX,
		DISABLE,
		TARGET_POS,
		CURRENT_POS,
		RAMP_MODE,
		MOTOR_ATTR_COUNT
	};

	enum MotorRampMode
	{
		POSITION,
		RAMP_MODE_1
	};

	enum HarmonicRegister
	{
		H1MAG,
		H1PH,
		H2MAG,
		H2PH,
		H3MAG,
		H3PH,
		H8MAG,
		H8PH,
		HARMONIC_REGISTER_COUNT
	};

	enum ConfigurationRegister
	{
		CNVPAGE,
		DIGIO,
		FAULT,
		GENERAL,
		DIGIOEN,
		ANGLECK,
		ECCCDE,
		ECCDIS,
		CONFIGURATION_REGISTER_COUNT
	};

	enum SensorRegister
	{
		ABSANGLE,
		ANGLE,
		ANGLESEC,
		SINE,
		COSINE,
		SECANGLI,
		SECANGLQ,
		RADIUS,
		DIAG1,
		DIAG2,
		TMP0,
		TMP1,
		CNVCNT,
		SENSOR_REGISTER_COUNT
	};

	enum UniqueIDRegister
	{
		UNIQID0,
		UNIQID1,
		UNIQID2,
		UNIQID3,
		UNIQID_REGISTER_COUNT
	};

	enum RampGeneratorDriverFeatureControlRegister
	{
		VDCMIN,
		SW_MODE,
		RAMP_STAT,
		XLATCH,
		RAMP_GENERATOR_DRIVER_FEATURE_CONTROL_REGISTER_COUNT
	};

	const char *ChannelIds[CHANNEL_COUNT] = {"rot", "angl", "count", "temp"};
	const char *DeviceIds[DEVICE_COUNT] = {"admt4000", "tmc5240"};
	const char *DeviceAttributes[DEVICE_ATTR_COUNT] = {
		"page",		 "sequencer_mode", "angle_filt",  "conversion_mode", "h8_ctrl",
		"sdp_gpio_ctrl", "sdp_gpio0_busy", "sdp_coil_rs", "regmap_dump"};
	const char *MotorAttributes[MOTOR_ATTR_COUNT] = {"amax",       "rotate_vmax", "dmax",	  "disable",
							 "target_pos", "current_pos", "ramp_mode"};
	const uint32_t ConfigurationRegisters[CONFIGURATION_REGISTER_COUNT] = {0x01, 0x04, 0x06, 0x10,
									       0x12, 0x13, 0x1D, 0x23};
	const uint8_t ConfigurationPages[CONFIGURATION_REGISTER_COUNT] = {UINT8_MAX, UINT8_MAX, UINT8_MAX, 0x02,
									  0x02,	     0x02,	0x02,	   0x02};
	const uint32_t UniqueIdRegisters[UNIQID_REGISTER_COUNT] = {0x1E, 0x1F, 0x20, 0x21};
	const uint8_t UniqueIdPages[UNIQID_REGISTER_COUNT] = {0x02, 0x02, 0x02, 0x02};
	const uint32_t HarmonicRegisters[HARMONIC_REGISTER_COUNT] = {0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C};
	const uint8_t HarmonicPages[HARMONIC_REGISTER_COUNT] = {0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};
	const uint32_t SensorRegisters[SENSOR_REGISTER_COUNT] = {0x03, 0x05, 0x08, 0x10, 0x11, 0x12, 0x13,
								 0x18, 0x1D, 0x1E, 0x20, 0x23, 0x14};
	const uint8_t SensorPages[SENSOR_REGISTER_COUNT] = {0x00, UINT8_MAX, UINT8_MAX, 0x00, 0x00, 0x00, 0x00,
							    0x00, 0x00,	     0x00,	0x00, 0x00, 0x02};

	const uint32_t
		RampGeneratorDriverFeatureControlRegisters[RAMP_GENERATOR_DRIVER_FEATURE_CONTROL_REGISTER_COUNT] = {
			0x33, 0x34, 0x35, 0x36};

	const char *getChannelId(Channel channel);
	const char *getDeviceId(Device device);
	const char *getDeviceAttribute(DeviceAttribute attribute);
	const char *getMotorAttribute(MotorAttribute attribute);
	const uint32_t getConfigurationRegister(ConfigurationRegister registerID);
	const uint8_t getConfigurationPage(ConfigurationRegister registerID);
	const uint32_t getUniqueIdRegister(UniqueIDRegister registerID);
	const uint32_t getHarmonicRegister(HarmonicRegister registerID);
	const uint8_t getHarmonicPage(HarmonicRegister registerID);
	const uint8_t getUniqueIdPage(UniqueIDRegister registerID);
	const uint32_t getSensorRegister(SensorRegister registerID);
	const uint8_t getSensorPage(SensorRegister registerID);

	const uint32_t
	getRampGeneratorDriverFeatureControlRegister(RampGeneratorDriverFeatureControlRegister registerID);

	void connectADMT();
	void disconnectADMT();
	int getChannelIndex(const char *deviceName, const char *channelName);
	double getChannelValue(const char *deviceName, const char *channelName, int bufferSize = 1);
	int getDeviceAttributeValue(const char *deviceName, const char *attributeName, double *returnValue);
	int getDeviceAttributeValueString(const char *deviceName, const char *attributeName, char *returnValue,
					  size_t byteLength = 512);
	int setDeviceAttributeValue(const char *deviceName, const char *attributeName, double writeValue);
	QString calibrate(vector<double> PANG, int cycles, int samplesPerCycle, bool CCW);
	int writeDeviceRegistry(const char *deviceName, uint32_t address, uint32_t value);
	int readDeviceRegistry(const char *deviceName, uint32_t address, uint32_t *returnValue);
	int readDeviceRegistry(const char *deviceName, uint32_t address, uint8_t page, uint32_t *returnValue);
	void computeSineCosineOfAngles(const vector<double> &angles);
	uint16_t calculateHarmonicCoefficientMagnitude(uint16_t harmonicCoefficient, uint16_t originalValue,
						       const string &key);
	uint16_t calculateHarmonicCoefficientPhase(uint16_t harmonicPhase, uint16_t originalValue);
	double getActualHarmonicRegisterValue(uint16_t registerValue, const string key);
	map<string, bool> getFaultRegisterBitMapping(uint16_t registerValue);
	map<string, int> getGeneralRegisterBitMapping(uint16_t registerValue);
	map<string, bool> getDIGIOENRegisterBitMapping(uint16_t registerValue);
	map<string, bool> getDIGIORegisterBitMapping(uint16_t registerValue);
	map<string, bool> getDiag1RegisterBitMapping_Register(uint16_t registerValue);
	map<string, double> getDiag1RegisterBitMapping_Afe(uint16_t registerValue, bool is5V);
	map<string, double> getDiag2RegisterBitMapping(uint16_t registerValue);
	uint16_t setGeneralRegisterBitMapping(uint16_t currentRegisterValue, map<string, int> settings);
	void postcalibrate(vector<double> PANG, int cycleCount, int samplesPerCycle, bool CCW);
	int getAbsAngleTurnCount(uint16_t registerValue);
	double getAbsAngle(uint16_t registerValue);
	double getAngle(uint16_t registerValue);
	double getTemperature(uint16_t registerValue);
	uint16_t setDIGIOENRegisterBitMapping(uint16_t currentRegisterValue, map<string, bool> settings);
	uint16_t setDIGIORegisterBitMapping(uint16_t currentRegisterValue, map<string, bool> settings);
	void unwrapAngles(vector<double> &angles_rad);
	map<string, string> getUNIQID3RegisterMapping(uint16_t registerValue);
	map<string, double> getSineRegisterBitMapping(uint16_t registerValue);
	map<string, double> getCosineRegisterBitMapping(uint16_t registerValue);
	map<string, double> getRadiusRegisterBitMapping(uint16_t registerValue);
	map<string, double> getAngleSecRegisterBitMapping(uint16_t registerValue);
	map<string, double> getSecAnglQRegisterBitMapping(uint16_t registerValue);
	map<string, double> getSecAnglIRegisterBitMapping(uint16_t registerValue);
	map<string, double> getTmp1RegisterBitMapping(uint16_t registerValue, bool is5V);
	bool checkRegisterFault(uint16_t registerValue, bool isMode1);
	int streamIO();
	void bufferedStreamIO(int totalSamples, int targetSampleRate, int bufferSize);
	void registryStream(int totalSamples, int targetSampleRate);
	bool checkVelocityReachedFlag(uint16_t registerValue);
	uint16_t changeCNVPage(uint16_t registerValue, uint8_t page);
	uint16_t convertStart(bool start, uint16_t registerValue);
	int streamChannel(const char *deviceName, const QVector<QString> channelNames, int bufferSize, int sampleRate);
public Q_SLOTS:
	void handleStreamData(double value);
	void handleStreamChannelData(QMap<QString, double> dataMap);
	void handleStreamBufferedData(const QVector<double> &value);
Q_SIGNALS:
	void streamData(double value);
	void streamChannelData(QMap<QString, double> dataMap);
	void streamBufferedData(const QVector<double> &value);
	void requestDisconnect();

private:
	QWidget *m_page;
	iio_context *m_iioCtx;
	iio_buffer *m_iioBuffer;
	Connection *m_conn;
	QString uri;

	unsigned int bitReverse(unsigned int x, int log2n);
	template <typename Iter_T>
	void fft(Iter_T a, Iter_T b, int log2n);
	void performFFT(const vector<double> &angle_errors, vector<double> &angle_errors_fft,
			vector<double> &angle_errors_fft_phase, int cycleCount);
	int linear_fit(vector<double> x, vector<double> y, double *slope, double *intercept);
	int calculate_angle_error(vector<double> angle_meas, vector<double> &angle_error_ret, double *max_angle_err);
	void getPreCalibrationFFT(const vector<double> &PANG, vector<double> &angle_errors_fft_pre,
				  vector<double> &angle_errors_fft_phase_pre, int cycleCount, int samplesPerCycle);
	void getPostCalibrationFFT(const vector<double> &updated_PANG, vector<double> &angle_errors_fft_post,
				   vector<double> &angle_errors_fft_phase_post, int cycleCount, int samplesPerCycle);
};
} // namespace scopy::admt

#endif // ADMTCONTROLLER_H
