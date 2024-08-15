#include "admtcontroller.h"

#include <iioutil/connectionprovider.h>

#include <string>
#include <cstdint>
#include <algorithm>
#include <cstring>
#include <list>

#include <vector>
#include <cstdlib>
#include <cmath>
#include <math.h>
#include <numeric>
#include <complex>
#include <iterator>

static const size_t maxAttrSize = 512;

using namespace scopy::admt;
using namespace std;

ADMTController::ADMTController(QString uri, QObject *parent)
    :QObject(parent)
    , uri(uri)
{

}

ADMTController::~ADMTController() {}

void ADMTController::connectADMT()
{
	m_conn = ConnectionProvider::open(uri);
    m_iioCtx = m_conn->context();
}

void ADMTController::disconnectADMT()
{
	if(!m_conn || !m_iioCtx){
		return;
	}

	ConnectionProvider::close(uri);
	m_conn = nullptr;
	m_iioCtx = nullptr;

}

const char* ADMTController::getChannelId(Channel channel)
{
	if(channel >= 0 && channel < CHANNEL_COUNT){
		return ChannelIds[channel];
	}
	return "Unknown";
}

const char* ADMTController::getDeviceId(Device device)
{
	if(device >= 0 && device < DEVICE_COUNT){
		return DeviceIds[device];
	}
	return "Unknown";
}

const char* ADMTController::getMotorAttribute(MotorAttribute attribute)
{
	if(attribute >= 0 && attribute < MOTOR_ATTR_COUNT){
		return MotorAttributes[attribute];
	}
	return "Unknown";
}


int ADMTController::getChannelIndex(const char *deviceName, const char *channelName)
{
	iio_device *admtDevice = iio_context_find_device(m_iioCtx, deviceName);
    if(admtDevice == NULL) { return -1; }
	int channelCount = iio_device_get_channels_count(admtDevice);
	iio_channel *channel;
	std::string message = "";
	for(int i = 0; i < channelCount; i++){
		channel = iio_device_get_channel(admtDevice, i);
		const char *deviceChannel = iio_channel_get_id(channel);
		
		std::string strDeviceChannel = std::string(deviceChannel);
		std::string strChannelName = std::string(channelName);

		if(deviceChannel != nullptr && strDeviceChannel == strChannelName){
			message = message + "[" + std::to_string(i) + "]" + std::string(deviceChannel) + ", ";
			return iio_channel_get_index(channel);
		}
		else {
			channel = NULL;
		}
	}
	return -1;
}

double ADMTController::getChannelValue(const char *deviceName, const char *channelName, int bufferSize)
{
	double value;
	char converted[bufferSize] = "";

	int deviceCount = iio_context_get_devices_count(m_iioCtx);
	//if(deviceCount < 1) return QString("No devices found");

	iio_device *admtDevice = iio_context_find_device(m_iioCtx, deviceName);
	//if(admtDevice == NULL) return QString("No ADMT4000 device");

	int channelCount = iio_device_get_channels_count(admtDevice);
	//if(channelCount < 1) return QString("No channels found.");

	iio_channel *channel;
	std::string message = "";
	for(int i = 0; i < channelCount; i++){
		channel = iio_device_get_channel(admtDevice, i);
		const char *deviceChannel = iio_channel_get_id(channel);
		
		if(deviceChannel != nullptr && std::string(deviceChannel) == std::string(channelName)){
			message = message + "[" + std::to_string(i) + "]" + std::string(deviceChannel) + ", ";
			break;
		}
		else {
			channel = NULL;
		}
	}
	if(channel == NULL) {
		//return QString::fromStdString(message);
	}
	iio_channel_enable(channel);

	double scale = 1.0;
	int offsetAttrVal = 0;
	const char *scaleAttrName = "scale";
	const char *offsetAttrName = "offset";
	const char *scaleAttr = iio_channel_find_attr(channel, scaleAttrName);
	//if(scaleAttr == NULL) return QString("No scale attribute");
	const char *offsetAttr = iio_channel_find_attr(channel, offsetAttrName);
	//if(offsetAttr == NULL) return QString("No offset attribute");

	double *scaleVal = new double(1);
	int scaleRet = iio_channel_attr_read_double(channel, scaleAttr, scaleVal);
	//if(scaleRet != 0) return QString("Cannot read scale attribute");
	scale = *scaleVal;

	char *offsetDst = new char[maxAttrSize];
	iio_channel_attr_read(channel, offsetAttr, offsetDst, maxAttrSize);
	offsetAttrVal = std::atoi(offsetDst);

	iio_buffer *iioBuffer = iio_device_create_buffer(admtDevice, bufferSize, false);
	//if(!iioBuffer) return QString("Cannot create buffer.");

	ssize_t numBytesRead;
	int8_t *pointerData, *pointerEnd;
	void *buffer;
	ptrdiff_t pointerIncrement;

	numBytesRead = iio_buffer_refill(iioBuffer);
	//if(numBytesRead < 1) return QString("Cannot refill buffer.");

	pointerIncrement = reinterpret_cast<ptrdiff_t>(iio_buffer_step(iioBuffer));
	pointerEnd = static_cast<int8_t*>(iio_buffer_end(iioBuffer));

	const struct iio_data_format *format = iio_channel_get_data_format(channel);
	const struct iio_data_format channelFormat = *format;
	unsigned int repeat = channelFormat.repeat;
	uint8_t bitLength = static_cast<uint8_t>(channelFormat.bits);
	size_t offset = static_cast<uint8_t>(channelFormat.shift);
	
	QString result;
	std::list<char> rawSamples;
	// std::list<uint16_t> unsignedSamples;
	std::list<int16_t> castSamples;

	size_t sample, bytes;

	size_t sampleSize = channelFormat.length / 8 * repeat;
	//if(sampleSize == 0) return QString("Sample size is zero.");

	buffer = malloc(sampleSize * bufferSize);
	//if(!buffer) return QString("Cannot allocate memory for buffer.");

	bytes = iio_channel_read(channel, iioBuffer, buffer, sampleSize * bufferSize);
	for(sample = 0; sample < bytes / sampleSize; ++sample)
	{
		for(int j = 0; j < repeat; ++j)
		{
			if(channelFormat.length / 8 == sizeof(int16_t))
			{
				rawSamples.push_back(*((int8_t*)buffer));
				int16_t rawValue = ((int16_t*)buffer)[sample+j];
				castSamples.push_back(rawValue);
				value = (rawValue - static_cast<int16_t>(offsetAttrVal)) * scale;
				result = QString::number(value);
			}
		}
	}

	message = message + result.toStdString();
 	iio_buffer_destroy(iioBuffer);
	return value; //QString::fromStdString(message);
}

/** @brief Get the attribute value of a device
 * @param deviceName A pointer to the device name
 * @param attributeName A NULL-terminated string corresponding to the name of the
 * attribute
 * @param returnValue A pointer to a double variable where the value should be stored
 * @return On success, 0 is returned.
 * @return On error, -1 is returned. */
int ADMTController::getDeviceAttributeValue(const char *deviceName, const char *attributeName, double *returnValue)
{
    int result = -1;
    int deviceCount = iio_context_get_devices_count(m_iioCtx);
    if(deviceCount == 0) { return result; }
	iio_device *iioDevice = iio_context_find_device(m_iioCtx, deviceName);
    if(iioDevice == NULL) { return result; }
    const char* hasAttr = iio_device_find_attr(iioDevice, attributeName);
    if(hasAttr == NULL) { return result; }
    result = iio_device_attr_read_double(iioDevice, attributeName, returnValue);

    return result;
}

/** @brief Set the attribute value of a device
 * @param deviceName A pointer to the device name
 * @param attributeName A NULL-terminated string corresponding to the name of the
 * attribute
 * @param writeValue A double variable of the value to be set
 * @return On success, 0 is returned.
 * @return On error, -1 is returned. */
int ADMTController::setDeviceAttributeValue(const char *deviceName, const char *attributeName, double writeValue)
{
    int result = -1;
    int deviceCount = iio_context_get_devices_count(m_iioCtx);
    if(deviceCount == 0) { return result; }
	iio_device *iioDevice = iio_context_find_device(m_iioCtx, deviceName);
    if(iioDevice == NULL) { return result; }
    const char* hasAttr = iio_device_find_attr(iioDevice, attributeName);
    if(hasAttr == NULL) { return result; }
    result = iio_device_attr_write_double(iioDevice, attributeName, writeValue);

    return result;
}

int ADMTController::writeDeviceRegistry(const char *deviceName, uint32_t address, double value)
{
    int result = -1;
    int deviceCount = iio_context_get_devices_count(m_iioCtx);
    if(deviceCount == 0) { return result; }
    iio_device *iioDevice = iio_context_find_device(m_iioCtx, deviceName);
    if(iioDevice == NULL) { return result; }
    result = iio_device_reg_write(iioDevice, address, static_cast<uint32_t>(value));

    return result;
}

int ADMTController::readDeviceRegistry(const char *deviceName, uint32_t address, double& readValue)
{
    int result = -1;
    int deviceCount = iio_context_get_devices_count(m_iioCtx);
    if(deviceCount == 0) { return result; }
    iio_device *iioDevice = iio_context_find_device(m_iioCtx, deviceName);
    if(iioDevice == NULL) { return result; }
    result = iio_device_reg_read(iioDevice, address, reinterpret_cast<uint32_t*>(&readValue));

    return result;
}

/* bit reversal from online example */
unsigned int ADMTController::bitReverse(unsigned int x, int log2n) {
    int n = 0;
    int mask = 0x1;
    for (int i = 0; i < log2n; i++) {
        n <<= 1;
        n |= (x & 1);
        x >>= 1;
    }
    return n;
}

template<class Iter_T>
/* fft from example codes online */
void ADMTController::fft(Iter_T a, Iter_T b, int log2n)
{
    typedef typename iterator_traits<Iter_T>::value_type complex;
    const complex J(0, 1);
    int n = 1 << log2n;
    for (unsigned int i = 0; i < n; ++i) {
        b[bitReverse(i, log2n)] = a[i];
    }
    for (int s = 1; s <= log2n; ++s) {
        int m = 1 << s;
        int m2 = m >> 1;
        complex w(1, 0);
        complex wm = exp(-J * (M_PI / m2));
        for (int j = 0; j < m2; ++j) {
            for (int k = j; k < n; k += m) {
                complex t = w * b[k + m2];
                complex u = b[k];
                b[k] = u + t;
                b[k + m2] = u - t;
            }
            w *= wm;
        }
    }
}

/* For linear fitting (hard-coded based on examples and formula for polynomial fitting) */
int ADMTController::linear_fit(vector<double> x, vector<double> y, double* slope, double* intercept)
{
    /* x, y, x^2, y^2, xy, xy^2 */
    double sum_x = 0, sum_y = 0, sum_x2 = 0, sum_y2 = 0, sum_xy = 0;
    int i;

    if (x.size() != y.size())
        return -22;

    for (i = 0; i < x.size(); i++) {
        sum_x += x[i];
        sum_y += y[i];
        sum_x2 += (x[i] * x[i]);
        sum_y2 += (y[i] * y[i]);
        sum_xy += (x[i] * y[i]);
    }

    *slope = (x.size() * sum_xy - sum_x * sum_y) / (x.size() * sum_x2 - sum_x * sum_x);

    *intercept = (sum_y * sum_x2 - sum_x * sum_xy) / (x.size() * sum_x2 - sum_x * sum_x);

    return 0;
}

/* Calculate angle error based on MATLAB and C# implementation */
int ADMTController::calculate_angle_error(vector<double> angle_meas, vector<double>& angle_error_ret, double* max_angle_err)
{
    vector<double> angle_meas_rad(angle_meas.size()); // radian converted input
    vector<double> angle_meas_rad_unwrap(angle_meas.size()); // unwrapped radian input
    vector<double> angle_fit(angle_meas.size()); // array for polynomial fitted data 
    vector<double> x_data(angle_meas.size());
    double coeff_a, coeff_b; // coefficients generated by polynomial fitting

    // convert to radian
    for (int i = 0; i < angle_meas_rad.size(); i++)
        angle_meas_rad[i] = angle_meas[i] * M_PI / 180.0;

    // unwrap angle (extracted from decompiled Angle GSF Unit
    double num = 0.0;
    angle_meas_rad_unwrap[0] = angle_meas_rad[0];
    for (int i = 1; i < angle_meas_rad.size(); i++)
    {
        double num2 = abs(angle_meas_rad[i] + num - angle_meas_rad_unwrap[i - 1]);
        double num3 = abs(angle_meas_rad[i] + num - angle_meas_rad_unwrap[i - 1] + M_PI * 2.0);
        double num4 = abs(angle_meas_rad[i] + num - angle_meas_rad_unwrap[i - 1] - M_PI * 2.0);
        if (num3 < num2 && num3 < num4)
            num += M_PI * 2.0;

        else if (num4 < num2 && num4 < num3)
            num -= M_PI * 2.0;

        angle_meas_rad_unwrap[i] = angle_meas_rad[i] + num;
    }

    // set initial point to zero
    double offset = angle_meas_rad_unwrap[0];
    for (int i = 0; i < angle_meas_rad_unwrap.size(); ++i)
        angle_meas_rad_unwrap[i] -= offset;

    /* Generate xdata for polynomial fitting */
    iota(x_data.begin(), x_data.end(), 1);

    // linear angle fitting (generated coefficients not same with matlab and python)
    // expecting 0.26 -0.26
    // getting ~0.27 ~-0.27 as of 4/2/2024
    /* input args: x, y, *slope, *intercept */
    linear_fit(x_data, angle_meas_rad_unwrap, &coeff_a, &coeff_b);

    //cout << coeff_a << " " << coeff_b << "\n";

    // generate data using coefficients from polynomial fitting
    for (int i = 0; i < angle_fit.size(); i++) {
        angle_fit[i] = coeff_a * x_data[i];
        //cout << "angle_fit " << angle_fit[i] << "\n";
    }

    // get angle error using pass by ref angle_error_ret
    for (int i = 0; i < angle_error_ret.size(); i++) {
        angle_error_ret[i] = angle_meas_rad_unwrap[i] - angle_fit[i];
        //cout << "angle_err_ret " << angle_error_ret[i] << "\n";
    }

    // Find the offset for error and subtract (using angle_error_ret)
    auto minmax = minmax_element(angle_error_ret.begin(), angle_error_ret.end());
    double angle_err_offset = (*minmax.first + *minmax.second) / 2;

    for (int i = 0; i < angle_error_ret.size(); i++)
        angle_error_ret[i] -= angle_err_offset;

    // Convert back to degrees (angle_error_ret)
    for (int i = 0; i < angle_meas.size(); i++)
        angle_error_ret[i] *= (180 / M_PI);

    // Find maximum absolute angle error
    *max_angle_err = *minmax.second;

    return 0;
}

QString ADMTController::calibrate(vector<double> PANG, int cycles, int samplesPerCycle){
	int CCW = 0, circshiftData = 0;
	QString result = "";

    // original script data (measured data: from data capture using GUI or other medium i.e., csv)
    //PANG = { 0.175781,11.5137,20.0391,39.0234,52.998,65.0391,76.4648,84.8145,98.7012,112.061,130.693,145.195,166.816,182.021,193.799,210.938,0.175781,11.5137,20.0391,39.0234,52.998,65.0391,76.4648,84.8145,98.7012,112.061,130.693,145.195,166.816,182.021,193.799,210.938 };

    /* Check CCW flag to know if array is to be reversed */
    if (CCW)
        reverse(PANG.begin(), PANG.end());


    /* Randomize starting point of array */
    if (circshiftData) {
        int shift = rand() % PANG.size();

        rotate(PANG.begin(), PANG.begin() + shift, PANG.end());
    }

    // Calculate the angular errors for ideal and measured
    double max_err = 0;
    vector<double> angle_errors(PANG.size());

    /* Calculate angle errors */
    calculate_angle_error(PANG, angle_errors, &max_err);

    //for (int i = 0; i < angle_errors.size(); i++)
    //    cout << "angle_err " << angle_errors[i] << "\n";

    /* Caluclate FFT of angle errors */
    /* FFT based on implementation from https://www.oreilly.com/library/view/c-cookbook/0596007612/ch11s18.html */
    vector<double> angle_errors_fft_temp(PANG.size());
    vector<double> angle_errors_fft_phase_temp(PANG.size());
    angle_errors_fft = vector<double>(PANG.size() / 2);
    angle_errors_fft_phase = vector<double>(PANG.size() / 2);
    typedef complex<double> cx;

    /* array declaration must be constant so hardcoded as of now */
    cx fft_in[samplesPerCycle*cycles];
    cx fft_out[samplesPerCycle*cycles];

    /* Format angle errros to match data type used in fft function */
    for (int i = 0; i < PANG.size(); i++)
        fft_in[i] = cx(angle_errors[i], 0);

    /* Invoke FFT function */
    fft(fft_in, fft_out, 8);

    /* Extract magnitude and phase from complex fft_out array */
    for (int i = 0; i < PANG.size(); i++) {
        //cout << "fft_out[" << i << "] = " << fft_out[i].real() / 256 << "  " << fft_out[i].imag() / 256 << "\n";

        angle_errors_fft_temp[i] = pow((pow(fft_out[i].real() / PANG.size(), 2) + pow(-fft_out[i].imag() / PANG.size(), 2)), 0.5);
        angle_errors_fft_temp[i] *= 2;

        angle_errors_fft_phase_temp[i] = atan2(fft_out[i].imag(), fft_out[i].real());
    }

    /* get upper half only */
    for (int i = 0; i < PANG.size() / 2; i++) {
        angle_errors_fft[i] = angle_errors_fft_temp[i];
        angle_errors_fft_phase[i] = angle_errors_fft_phase_temp[i];
    }

    /* end code for FFT of angle errors */
    // Extract HMag parameters
    double H1Mag = angle_errors_fft[cycles];
    double H2Mag = angle_errors_fft[2 * cycles];
    double H3Mag = angle_errors_fft[3 * cycles];
    double H8Mag = angle_errors_fft[8 * cycles];

    /* Display HMAG values */
	result.append("H1Mag = " + QString::number(H1Mag) + "\n");
	result.append("H2Mag = " + QString::number(H2Mag) + "\n");
	result.append("H3Mag = " + QString::number(H3Mag) + "\n");
	result.append("H8Mag = " + QString::number(H8Mag) + "\n");
    // cout << "H1Mag = " << H1Mag << "\n";
    // cout << "H2Mag = " << H2Mag << "\n";
    // cout << "H3Mag = " << H3Mag << "\n";
    // cout << "H8Mag = " << H8Mag << "\n";

    // Extract HPhase parameters
    double H1Phase = (180 / M_PI) * (angle_errors_fft_phase[cycles]);
    double H2Phase = (180 / M_PI) * (angle_errors_fft_phase[2 * cycles]);
    double H3Phase = (180 / M_PI) * (angle_errors_fft_phase[3 * cycles]);
    double H8Phase = (180 / M_PI) * (angle_errors_fft_phase[8 * cycles]);

    /* Display HPHASE values */
	result.append("H1Phase = " + QString::number(H1Phase) + "\n");
	result.append("H2Phase = " + QString::number(H2Phase) + "\n");
	result.append("H3Phase = " + QString::number(H3Phase) + "\n");
	result.append("H8Phase = " + QString::number(H8Phase) + "\n");
    // cout << "H1Phase = " << H1Phase << "\n";
    // cout << "H2Phase = " << H2Phase << "\n";
    // cout << "H3Phase = " << H3Phase << "\n";
    // cout << "H8Phase = " << H8Phase << "\n";

    // cout << "\n\n";

    double H1 = H1Mag * cos(M_PI / 180 * (H1Phase));
    double H2 = H2Mag * cos(M_PI / 180 * (H2Phase));
    double H3 = H3Mag * cos(M_PI / 180 * (H3Phase));
    double H8 = H8Mag * cos(M_PI / 180 * (H8Phase));

    double init_err = H1 + H2 + H3 + H8;
    double init_angle = PANG[0] - init_err;
    double H1PHcor, H2PHcor, H3PHcor, H8PHcor;

    /* Counterclock wise, slope of error FIT is negative */
    if (CCW) {
        H1Phase *= -1;
        H2Phase *= -1;
        H3Phase *= -1;
        H8Phase *= -1;
    }

    /* Clockwise */
    H1PHcor = H1Phase - (1 * init_angle - 90);
    H2PHcor = H2Phase - (2 * init_angle - 90);
    H3PHcor = H3Phase - (3 * init_angle - 90);
    H8PHcor = H8Phase - (8 * init_angle - 90);

    /* Get modulo from 360 */
    H1PHcor = (int)H1PHcor % 360;
    H2PHcor = (int)H2PHcor % 360;
    H3PHcor = (int)H3PHcor % 360;
    H8PHcor = (int)H8PHcor % 360;

    /* Variables declaration for data correction */
    vector<double> H1c(PANG.size());
    vector<double> H2c(PANG.size());
    vector<double> H3c(PANG.size());
    vector<double> H8c(PANG.size());
    vector<double> HXcorrection(PANG.size());

    ///* Apply correction and check if working on original data */
    for (int i = 0; i < PANG.size(); i++)
    {
        H1c[i] = H1Mag * sin(M_PI / 180 * (PANG[i]) + M_PI / 180 * (H1PHcor));
        H2c[i] = H2Mag * sin(2 * M_PI / 180 * (PANG[i]) + M_PI / 180 * (H2PHcor));
        H3c[i] = H3Mag * sin(3 * M_PI / 180 * (PANG[i]) + M_PI / 180 * (H3PHcor));
        H8c[i] = H8Mag * sin(8 * M_PI / 180 * (PANG[i]) + M_PI / 180 * (H8PHcor));

        HXcorrection[i] = H1c[i] + H2c[i] + H3c[i] + H8c[i];
    }

    // These are the results to be programmed into the device
    // Magnitude scaling factor of 0.6072 is needed due to internal ADMT4000
    // CORDIC calculation scaling

    // Hardcoded value for comparison / reference
    //H1Mag = 0.3259;
    //H2Mag = 0.1275;
    //H3Mag = 3.4849e-03;
    //H8Mag = 0.088172;
    //H1PHcor = 202.58;
    //H2PHcor = 342.78;
    //H3PHcor = 303.40;
    //H8PHcor = 179.97;

    // HMag Scaling
    H1Mag = H1Mag * 0.6072;
    H2Mag = H2Mag * 0.6072;
    H3Mag = H3Mag * 0.6072;
    H8Mag = H8Mag * 0.6072;

    // Derive register compatible HMAG values
    double mag_scale_factor_11bit = 11.2455 / (1 << 11);
    double mag_scale_factor_8bit = 1.40076 / (1 << 8);
    HAR_MAG_1 = (int)(H1Mag / mag_scale_factor_11bit) & (0x7FF); // 11 bit 
    HAR_MAG_2 = (int)(H2Mag / mag_scale_factor_11bit) & (0x7FF); // 11 bit
    HAR_MAG_3 = (int)(H3Mag / mag_scale_factor_8bit) & (0xFF); // 8 bit
    HAR_MAG_8 = (int)(H8Mag / mag_scale_factor_8bit) & (0xFF);  // 8 bit

    // Derive register compatible HPHASE values
    double pha_scale_factor_12bit = 360.0 / (1 << 12); // in Deg
    HAR_PHASE_1 = (int)(H1PHcor / pha_scale_factor_12bit) & (0xFFF); // 12bit number
    HAR_PHASE_2 = (int)(H2PHcor / pha_scale_factor_12bit) & (0xFFF); // 12bit number
    HAR_PHASE_3 = (int)(H3PHcor / pha_scale_factor_12bit) & (0xFFF);// 12bit number
    HAR_PHASE_8 = (int)(H8PHcor / pha_scale_factor_12bit) & (0xFFF); // 12bit number

	result.append("HMAG1: " + QString::number(HAR_MAG_1) + "\n");
	result.append("HMAG2: " + QString::number(HAR_MAG_2) + "\n");
	result.append("HMAG3: " + QString::number(HAR_MAG_3) + "\n");
	result.append("HMAG8: " + QString::number(HAR_MAG_8) + "\n");

	result.append("HPHASE1: " + QString::number(HAR_PHASE_1) + "\n");
	result.append("HPHASE2: " + QString::number(HAR_PHASE_2) + "\n");
	result.append("HPHASE3: " + QString::number(HAR_PHASE_3) + "\n");
	result.append("HPHASE8: " + QString::number(HAR_PHASE_8) + "\n");

	// cout << "HMAG1: " << HAR_MAG_1 << "\n";
    // cout << "HMAG2: " << HAR_MAG_2 << "\n";
    // cout << "HMAG3: " << HAR_MAG_3 << "\n";
    // cout << "HMAG8: " << HAR_MAG_8 << "\n";

    // cout << "HPHASE1: " << HAR_PHASE_1 << "\n";
    // cout << "HPHASE2: " << HAR_PHASE_2 << "\n";
    // cout << "HPHASE3: " << HAR_PHASE_3 << "\n";
    // cout << "HPHASE8: " << HAR_PHASE_8 << "\n";

    // cout << "\n\n";

	// /* Sanity check */
    // cout << "Hello World" << "\n";

	return result;
}