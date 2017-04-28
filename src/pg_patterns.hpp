/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef PG_PATTERNS_HPP
#define PG_PATTERNS_HPP

#include <QWidget>
#include <QPushButton>
#include <QJsonArray>
#include <QJsonObject>
#include <QIntValidator>
#include <QtQml/QJSEngine>
#include <QtUiTools/QUiLoader>
#include <vector>
#include <deque>
#include <string>
#include <memory>
#include "spinbox_a.hpp"

// Generated UI
#include "ui_binarycounterpatternui.h"
#include "ui_uartpatternui.h"
#include "ui_lfsrpatternui.h"
#include "ui_emptypatternui.h"
#include "ui_genericjspatternui.h"
#include "ui_constantpattern.h"
#include "ui_numberpattern.h"
#include "ui_frequencypattern.h"
#include "ui_pulsepattern.h"
#include "ui_walkingpattern.h"
#include "ui_spipatternui.h"
#include "ui_i2cpatternui.h"


namespace Ui {
class BinaryCounterPatternUI;
class UARTPatternUI;
class LFSRPatternUI;
class EmptyPatternUI;
class GenericJSPatternUI;
class ConstantPatternUI;
class NumberPatternUI;
class FrequencyPatternUI;
class PulsePatternUI;
class WalkingPatternUI;
class SPIPatternUI;
class I2CPatternUI;
}

namespace adiscope {


#define ConstantPatternName  "Constant Pattern"
#define ConstantPatternDescription "Samples a constant 0 or 1 on the active channels"
#define NumberPatternName "Number Pattern"
#define NumberPatternDescription "Samples a constant number over the active channels"
#define ClockPatternName "Clock"
#define ClockPatternDescription "Clock pattern generator"
#define PulsePatternName "Pulse Pattern"
#define PulsePatternDescription "Pulse pattern generator"
#define RandomPatternName "Random"
#define RandomPatternDescription "Random pattern generator"
#define SPIPatternName "SPI"
#define SPIPatternDescription "SPI pattern generator"
#define I2CPatternName "I2C"
#define I2CPatternDescription "I2C pattern generator"
#define UARTPatternName "UART"
#define UARTPatternDescription "UART pattern generator"
#define BinaryCounterPatternName "Binary Counter"
#define BinaryCounterPatternDescription "Binary counter pattern generator"
#define GrayCounterPatternName "Gray Counter"
#define GrayCounterPatternDescription "Gray counter pattern generator"
#define JohnsonCounterPatternName "Johnson Counter Pattern"
#define JohnsonCounterPatternDescription "Johnson counter pattern generator"
#define WalkingCounterPatternName "Walking Counter Pattern"
#define WalkingCounterPatternDescription "Walking counter pattern generator"

enum {
	ClockPatternId = 0,
	NumberPatternId,
	RandomPatternId,
	BinaryCounterId,
	UARTPatternId,
	SPIPatternId,
	I2CPatternId,
	GrayCounterId,
};




// http://stackoverflow.com/questions/32040101/qjsengine-print-to-console
class JSConsole : public QObject // for logging purposes in QJSEngine
{
	Q_OBJECT
public:
	explicit JSConsole(QObject *parent = 0);
	Q_INVOKABLE void log(QString msg);
};

class Pattern
{
private:
	std::string name;
	std::string description;
	bool periodic;
protected: // temp
	short *buffer;
public:

	Pattern(/*string name_, string description_*/);
	virtual ~Pattern();
	std::string get_name();
	void set_name(std::string name_);
	std::string get_description();
	void set_description(std::string description_);
	void set_periodic(bool periodic_);
	short *get_buffer();
	void delete_buffer();
	virtual void init();
	virtual uint8_t pre_generate();
	virtual bool is_periodic();
	virtual uint32_t get_min_sampling_freq();
	virtual uint32_t get_required_nr_of_samples(uint32_t sample_rate,
	                uint32_t number_of_channels);
	virtual uint8_t generate_pattern(uint32_t sample_rate,
	                                 uint32_t number_of_samples, uint16_t number_of_channels) = 0;
	virtual void deinit();

	virtual std::string toString();
	virtual bool fromString(std::string from);

};

uint32_t changeBit(uint32_t number,uint8_t n, bool x);
class Pattern_API : public QObject
{
	Q_OBJECT
public:
	static QJsonValue toJson(Pattern *p);
	static Pattern *fromJson(QJsonValue j);
private:
	Pattern_API() {}
};

class PatternUI : public QWidget
{
	Q_OBJECT
public:
	PatternUI(QWidget *parent = 0);
	virtual ~PatternUI();
	//static PatternUI *create_ui(int index, QWidget *parent = 0);
	virtual void build_ui(QWidget *parent = 0,uint16_t number_of_channels=0);
	virtual Pattern *get_pattern() = 0;
	virtual void post_load_ui();
	virtual void parse_ui();
	virtual void destroy_ui();
Q_SIGNALS:
	void patternChanged();
};



class ClockPattern : virtual public Pattern
{
	int duty_cycle_granularity = 20;
	float frequency;
	float duty_cycle;
	int phase;
public:
	ClockPattern();
	virtual ~ClockPattern();
	uint8_t generate_pattern(uint32_t sample_rate, uint32_t number_of_samples,
	                         uint16_t number_of_channels);
	float get_frequency() const;
	void set_frequency(float value);
	float get_duty_cycle() const;
	void set_duty_cycle(float value);
	int get_phase() const;
	void set_phase(int value);
	uint32_t get_min_sampling_freq();
	uint32_t get_required_nr_of_samples(uint32_t  sample_rate,
	                                    uint32_t number_of_channels);

};

class ClockPatternUI : public PatternUI
{
	Q_OBJECT
	Ui::EmptyPatternUI *ui;
	QWidget *parent_;
	ClockPattern *pattern;
	ScaleSpinButton *frequencySpinButton;
	PhaseSpinButton *phaseSpinButton;
	PositionSpinButton *dutySpinButton;
public:
	ClockPatternUI(ClockPattern *pattern, QWidget *parent = 0);
	virtual ~ClockPatternUI();
	Pattern *get_pattern();
	void build_ui(QWidget *parent = 0,uint16_t number_of_channels=0);
	void destroy_ui();

private Q_SLOTS:
	void parse_ui();
};

class RandomPattern : virtual public Pattern
{
protected:
	uint32_t frequency;
public:
	RandomPattern();
	virtual ~RandomPattern();
	uint32_t get_min_sampling_freq();
	uint32_t get_required_nr_of_samples(uint32_t sample_rate,
	                                    uint32_t number_of_channels);
	uint8_t generate_pattern(uint32_t sample_rate, uint32_t number_of_samples,
	                         uint16_t number_of_channels);

	uint32_t get_frequency() const;
	void set_frequency(const uint32_t& value);

};

class RandomPatternUI : public PatternUI
{
	Q_OBJECT
	Ui::EmptyPatternUI *ui;
	QWidget *parent_;
	RandomPattern *pattern;
	ScaleSpinButton *frequencySpinButton;
public:
	RandomPatternUI(RandomPattern *pattern, QWidget *parent = 0);
	~RandomPatternUI();
	Pattern *get_pattern();
	void build_ui(QWidget *parent = 0,uint16_t number_of_channels=0);
	void destroy_ui();

private Q_SLOTS:
	void parse_ui();
};


class BinaryCounterPattern : virtual public Pattern
{
protected:
	uint32_t frequency;
	uint16_t start_value;
	uint16_t end_value;
	uint16_t increment;
	uint16_t init_value;
public:
	BinaryCounterPattern();
	virtual ~BinaryCounterPattern();
	virtual uint8_t generate_pattern(uint32_t sample_rate,
	                                 uint32_t number_of_samples, uint16_t number_of_channels);
	uint32_t get_min_sampling_freq();
	uint32_t get_required_nr_of_samples(uint32_t sample_rate,
	                                    uint32_t number_of_channels);

	uint32_t get_frequency() const;
	void set_frequency(const uint32_t& value);
	uint16_t get_start_value() const;
	void set_start_value(const uint16_t& value);
	uint16_t get_end_value() const;
	void set_end_value(const uint16_t& value);
	uint16_t get_increment() const;
	void set_increment(const uint16_t& value);
	uint16_t get_init_value() const;
	void set_init_value(const uint16_t& value);
};

class BinaryCounterPatternUI : public PatternUI
{
	Q_OBJECT
	Ui::EmptyPatternUI *ui;
	QWidget *parent_;
	BinaryCounterPattern *pattern;
	ScaleSpinButton *frequencySpinButton;
public:
	BinaryCounterPatternUI(BinaryCounterPattern *pattern, QWidget *parent = 0);
	~BinaryCounterPatternUI();
	Pattern *get_pattern();
	void build_ui(QWidget *parent = 0,uint16_t number_of_channels=0);
	void destroy_ui();

private Q_SLOTS:
	void parse_ui();
};


class GrayCounterPattern : virtual public BinaryCounterPattern
{
public:
	GrayCounterPattern();
	virtual ~GrayCounterPattern() {}
	uint8_t generate_pattern(uint32_t sample_rate,
	                         uint32_t number_of_samples, uint16_t number_of_channels);
};

class GrayCounterPatternUI : public PatternUI
{
	Q_OBJECT
	Ui::EmptyPatternUI *ui;
	QWidget *parent_;
	GrayCounterPattern *pattern;
	ScaleSpinButton *frequencySpinButton;
public:
	GrayCounterPatternUI(GrayCounterPattern *pattern, QWidget *parent = 0);
	~GrayCounterPatternUI();
	Pattern *get_pattern();
	void build_ui(QWidget *parent = 0,uint16_t number_of_channels=0);
	void destroy_ui();

private Q_SLOTS:
	void parse_ui();
};


class UARTPattern : virtual public Pattern
{

	/** Parity settings. */
	enum sp_parity {
		/** Special value to indicate setting should be left alone. */
		SP_PARITY_INVALID = -1,
		/** No parity. */
		SP_PARITY_NONE = 0,
		/** Odd parity. */
		SP_PARITY_ODD = 1,
		/** Even parity. */
		SP_PARITY_EVEN = 2,
		/** Mark parity. */
		SP_PARITY_MARK = 3,
		/** Space parity. */
		SP_PARITY_SPACE = 4
	};
protected:
	std::string str;
	std::string params;
	bool msb_first;
	unsigned int baud_rate;
	unsigned int data_bits;
	unsigned int stop_bits;
	enum sp_parity parity;


public:
	UARTPattern();
	virtual ~UARTPattern() {}

	virtual uint8_t generate_pattern(uint32_t sample_rate,
	                                 uint32_t number_of_samples, uint16_t number_of_channels);
	uint32_t get_min_sampling_freq();
	uint32_t get_required_nr_of_samples(uint32_t sample_rate,
	                                    uint32_t number_of_channels);

	unsigned int get_baud_rate();
	unsigned int get_data_bits();
	unsigned int get_stop_bits();
	enum sp_parity get_parity();

	void set_string(std::string str_);
	std::string get_string();
	std::string get_params();
	int set_params(std::string params_);
	void set_msb_first(bool msb_first_);
	uint16_t encapsulateUartFrame(char chr, uint16_t *bits_per_frame);

};

class UARTPatternUI : public PatternUI
{
	Q_OBJECT
	Ui::UARTPatternUI *ui;
	QWidget *parent_;
	UARTPattern *pattern;
public:
	UARTPatternUI(UARTPattern *pattern, QWidget *parent = 0);
	~UARTPatternUI();
	Pattern *get_pattern();
	void build_ui(QWidget *parent = 0,uint16_t number_of_channels=0);
	void destroy_ui();
private Q_SLOTS:
	void parse_ui();
};

class I2CPattern: virtual public Pattern
{
	bool tenbit;
	uint8_t address;
	bool read;
	bool msbFirst;

	uint32_t clkFrequency;
	uint32_t samples_per_bit;
	uint8_t interFrameSpace;
	uint8_t bytesPerFrame;

	short *buf_ptr;

	const int SDA = 1;
	const int SCL = 0;

	void sample_bit(bool bit);
	void sample_start_bit();
	void sample_address();
	void sample_ack();
	void sample_payload();
	void sample_stop();
public:
	std::deque<uint8_t> v;
	I2CPattern();
	virtual ~I2CPattern() {}
	virtual uint8_t generate_pattern(uint32_t sample_rate,
	                                 uint32_t number_of_samples, uint16_t number_of_channels);
	uint32_t get_min_sampling_freq();
	uint32_t get_required_nr_of_samples(uint32_t sample_rate,
	                                    uint32_t number_of_channels);
	bool getTenbit() const;
	void setTenbit(bool value);
	uint8_t getAddress() const;
	void setAddress(const uint8_t& value);
	bool getWrite() const;
	void setWrite(bool value);
	bool getMsbFirst() const;
	void setMsbFirst(bool value);
	uint8_t getInterFrameSpace() const;
	void setInterFrameSpace(const uint8_t& value);
	uint32_t getClkFrequency() const;
	void setClkFrequency(const uint32_t& value);
	uint8_t getBytesPerFrame() const;
	void setBytesPerFrame(const uint8_t& value);
};

class I2CPatternUI : public PatternUI
{
	Q_OBJECT
	Ui::I2CPatternUI *ui;
	QWidget *parent_;
	I2CPattern *pattern;
	ScaleSpinButton *frequencySpinButton;
public:
	I2CPatternUI(I2CPattern *pattern, QWidget *parent = 0);
	~I2CPatternUI();
	Pattern *get_pattern();
	void build_ui(QWidget *parent = 0,uint16_t number_of_channels=0);
	void destroy_ui();
private Q_SLOTS:
	void parse_ui();
};


class SPIPattern : virtual public Pattern
{
private:
	bool CSEnabled;
	uint8_t bytesPerFrame;
	uint32_t clkFrequency;
	uint8_t waitClocks;
	bool msbFirst;
	bool CPOL;
	bool CPHA;

public:
	std::deque<uint8_t> v;
	SPIPattern();
	virtual ~SPIPattern() {}
	virtual uint8_t generate_pattern(uint32_t sample_rate,
	                                 uint32_t number_of_samples, uint16_t number_of_channels);
	uint32_t get_min_sampling_freq();
	uint32_t get_required_nr_of_samples(uint32_t sample_rate,
	                                    uint32_t number_of_channels);

	bool getCSEnabled() const;
	void setCSEnabled(bool value);
	bool getCPOL() const;
	void setCPOL(bool value);
	bool getCPHA() const;
	void setCPHA(bool value);
	uint32_t getClkFrequency() const;
	void setClkFrequency(const uint32_t& value);
	uint8_t getWaitClocks() const;
	void setWaitClocks(const uint8_t& value);
	uint8_t getBytesPerFrame() const;
	void setBytesPerFrame(const uint8_t& value);
	bool getMsbFirst() const;
	void setMsbFirst(bool value);
};



class SPIPatternUI : public PatternUI
{
	Q_OBJECT
	Ui::SPIPatternUI *ui;
	QWidget *parent_;
	SPIPattern *pattern;
	ScaleSpinButton *frequencySpinButton;
public:
	SPIPatternUI(SPIPattern *pattern, QWidget *parent = 0);
	~SPIPatternUI();
	Pattern *get_pattern();
	void build_ui(QWidget *parent = 0,uint16_t number_of_channels=0);
	void destroy_ui();
private Q_SLOTS:
	void parse_ui();
};

class NumberPattern : virtual public Pattern
{
private:
	uint16_t nr;
public:
	NumberPattern();
	virtual ~NumberPattern() {}
	virtual uint8_t generate_pattern(uint32_t sample_rate,
	                                 uint32_t number_of_samples, uint16_t number_of_channels);
	uint16_t get_nr() const;
	void set_nr(const uint16_t& value);
};

class NumberPatternUI : public PatternUI
{
	Q_OBJECT
	Ui::NumberPatternUI *ui;
	QWidget *parent_;
	NumberPattern *pattern;
	uint16_t max;
public:
	NumberPatternUI(NumberPattern *pattern, QWidget *parent = 0);
	~NumberPatternUI();
	Pattern *get_pattern();
	void build_ui(QWidget *parent = 0,uint16_t number_of_channels=0);
	void destroy_ui();
private Q_SLOTS:
	void parse_ui();
};


#if 0

class JSPattern : public QObject, virtual public Pattern
{
	Q_OBJECT
private:
protected:
	QJSEngine *qEngine;
	JSConsole *console;
	QWidget *ui_form;
	QJsonObject obj;
public:

	JSPattern(QJsonObject obj_);
	Q_INVOKABLE quint32 get_nr_of_samples();
	Q_INVOKABLE quint32 get_nr_of_channels();
	Q_INVOKABLE quint32 get_sample_rate();
	/*Q_INVOKABLE*/ void JSErrorDialog(QString errorMessage);
	/*Q_INVOKABLE*/ void commitBuffer(QJSValue jsBufferValue,
	                                  QJSValue jsBufferSize);
	bool is_periodic();
	uint32_t get_min_sampling_freq();
	uint32_t get_required_nr_of_samples();
	void init();
	uint8_t pre_generate();
	uint8_t generate_pattern();
	void deinit();
	virtual bool handle_result(QJSValue result,QString str = "");
};

class JSPatternUIStatusWindow : public QObject
{
	Q_OBJECT
	QTextEdit *con;
public:
	JSPatternUIStatusWindow(QTextEdit *textedit);
	Q_INVOKABLE void clear();
	Q_INVOKABLE void print(QString str);
};


class JSPatternUI : public PatternUI, public JSPattern
{
	QUiLoader *loader;
	Ui::GenericJSPatternUI *ui;
	//QWidget *ui_form;
	QString form_name;
	QWidget *parent_;
	JSPatternUIStatusWindow *textedit;
public:
	JSPatternUI(QJsonObject obj_, QWidget *parent = 0);
	~JSPatternUI();

	bool handle_result(QJSValue result,QString str = "");
	void find_all_children(QObject *parent, QJSValue property);
	void build_ui(QWidget *parent = 0);
	void post_load_ui();
	void parse_ui();
	void destroy_ui();
};

class LFSRPattern : virtual public Pattern
{
private:

	uint32_t lfsr_poly;
	uint16_t start_state;
	uint32_t lfsr_period;
public:
	LFSRPattern();
	uint8_t generate_pattern();

	uint32_t compute_period();
	uint32_t get_lfsr_poly() const;
	void set_lfsr_poly(const uint32_t& value);
	uint16_t get_start_state() const;
	void set_start_state(const uint16_t& value);
	uint32_t get_lfsr_period() const;
};

class LFSRPatternUI : public PatternUI, public LFSRPattern
{
	Q_OBJECT
	Ui::LFSRPatternUI *ui;
	QWidget *parent_;
public:
	LFSRPatternUI(QWidget *parent = 0);
	~LFSRPatternUI();
	void build_ui(QWidget *parent = 0);
	void parse_ui();
	void destroy_ui();
	/*private Q_SLOTS:
	void on_setLFSRParameters_clicked();*/
};

class ConstantPattern : virtual public Pattern
{
private:
	bool constant;
public:
	ConstantPattern();
	uint8_t generate_pattern();
	bool get_constant() const;
	void set_constant(bool value);
};

class ConstantPatternUI : public PatternUI, public ConstantPattern
{
	Q_OBJECT
	Ui::ConstantPatternUI *ui;
	QWidget *parent_;
public:
	ConstantPatternUI(QWidget *parent = 0);
	~ConstantPatternUI();
	void parse_ui();
	void build_ui(QWidget *parent = 0);
	void destroy_ui();
};

class PulsePattern : virtual public Pattern
{
protected:
	bool start;
	uint32_t low_number_of_samples;
	uint32_t high_number_of_samples;
	uint32_t counter_init;
	uint16_t divider;
	uint16_t divider_init;

public:
	PulsePattern();
	uint8_t generate_pattern();
	uint32_t get_min_sampling_freq();
	uint32_t get_required_nr_of_samples();
	bool get_start();
	uint32_t get_low_number_of_samples();
	uint32_t get_high_number_of_samples();
	uint32_t get_counter_init();
	uint16_t get_divider();
	uint16_t get_divider_init();
	void set_start(bool val);
	void set_low_number_of_samples(uint32_t val);
	void set_high_number_of_samples(uint32_t val);
	void set_counter_init(uint32_t val);
	void set_divider(uint16_t val);
	void set_divider_init(uint16_t val);

};

class PulsePatternUI : public PatternUI, public PulsePattern
{
	Q_OBJECT
	Ui::PulsePatternUI *ui;
	QWidget *parent_;
public:
	PulsePatternUI(QWidget *parent = 0);
	~PulsePatternUI();
	void parse_ui();
	void build_ui(QWidget *parent = 0);
	void destroy_ui();
};

class JohnsonCounterPattern : virtual public Pattern
{
	uint32_t frequency;
public:
	JohnsonCounterPattern();
	uint8_t generate_pattern();
	uint32_t get_min_sampling_freq();
	uint32_t get_required_nr_of_samples();
	uint32_t get_frequency() const;
	void set_frequency(const uint32_t& value);
};

class JohnsonCounterPatternUI: public PatternUI, public JohnsonCounterPattern
{
	Q_OBJECT
	Ui::FrequencyPatternUI *ui;
	QWidget *parent_;
public:
	JohnsonCounterPatternUI(QWidget *parent = 0);
	~JohnsonCounterPatternUI();
	void parse_ui();
	void build_ui(QWidget *parent = 0);
	void destroy_ui();
};

class WalkingPattern : virtual public Pattern
{
	uint32_t frequency;
	uint16_t length;
	bool right;
	bool level;
public:
	WalkingPattern();
	uint8_t generate_pattern();
	uint32_t get_min_sampling_freq();
	uint32_t get_required_nr_of_samples();

	uint32_t get_frequency() const;
	void set_frequency(const uint32_t& value);
	uint16_t get_length() const;
	void set_length(const uint16_t& value);
	bool get_right() const;
	void set_right(bool value);
	bool get_level() const;
	void set_level(bool value);
};

class WalkingPatternUI: public PatternUI, public WalkingPattern
{
	Q_OBJECT
	Ui::WalkingPatternUI *ui;
	QWidget *parent_;
public:
	WalkingPatternUI(QWidget *parent = 0);
	~WalkingPatternUI();
	void parse_ui();
	void build_ui(QWidget *parent = 0);
	void destroy_ui();
};
#endif

class PatternFactory
{
	static QStringList ui_list;
	static QStringList description_list;
	static int static_ui_limit;
	static QJsonObject patterns;
public:
	static void init();
	static Pattern *create(QString name);
	static Pattern *create(int index);
	static PatternUI *create_ui(Pattern *pattern, int index, QWidget *parent = 0);
	static PatternUI *create_ui(Pattern *pattern, QWidget *parent=0);
	static QStringList get_ui_list();
	static QStringList get_description_list();
private:
	PatternFactory() {}
};

}
#endif
