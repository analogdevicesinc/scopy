#ifndef REGMAPSTYLEHELPER_HPP
#define REGMAPSTYLEHELPER_HPP

#include "titlespinbox.hpp"
#include "searchbarwidget.hpp"

#include <QCheckBox>
#include <QLabel>
#include <QMap>
#include <QObject>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>

#include <register/bitfield/bitfielddetailedwidget.hpp>
#include <register/registersimplewidget.hpp>

#include <register/bitfield/bitfielddetailedwidget.hpp>
#include <register/bitfield/bitfieldsimplewidget.hpp>

#include <register/registersimplewidget.hpp>

namespace scopy::regmap {

class RegisterMapSettingsMenu;

class RegmapStyleHelper
{
protected:
	RegmapStyleHelper(QObject *parent = nullptr);
	~RegmapStyleHelper();

public:
	// singleton
	RegmapStyleHelper(RegmapStyleHelper &other) = delete;
	void operator=(const RegmapStyleHelper &) = delete;
	static RegmapStyleHelper *GetInstance();

	static void initColorMap();
	static QString getColor(QString id);

	static void DeviceRegisterMap(QWidget *widget);
	static QString PartialFrameWidgetStyle();
	static QString FrameWidgetStyle();
	static QString bigTextLabelStyle();
	static void bigTextLabel(QLabel *label, QString objectName = "");
	static void labelStyle(QLabel *label, QString objectName);

	static void regmapSettingsMenu(RegisterMapSettingsMenu *settings, QString objectName = "");
	static void grayBackgroundHoverWidget(QWidget *widget, QString objectName = "");
	static QString BlueButtonStyle();
	static void BlueButton(QPushButton *btn, QString objectName = "");
	static QString checkBoxStyle();
	static void checkBox(QCheckBox *checkbox, QString objectName = "");
	static void BitFieldDetailedWidgetStyle(BitFieldDetailedWidget *widget, QString objectName = "");
	static void BitFieldSimpleWidgetStyle(BitFieldSimpleWidget *widget, QString objectName = "");
	static void RegisterSimpleWidgetStyle(RegisterSimpleWidget *widget, QString objectName = "");
	static QString grayLabelStyle();
	static QString whiteSmallTextLableStyle();
	static QString simpleWidgetStyle();
	static void comboboxStyle(QComboBox *combobox, QString objectName = "");
	static void titleSpinBoxStyle(TitleSpinBox *spinbox, QString objectName = "");
	static void searchBarStyle(SearchBarWidget *searchBar, QString objectName = "");
	static void smallBlueButton(QPushButton *button, QString objectName = "");
	static QString regmapControllerStyle(QWidget *widget, QString objectName = "");
	static QString widgetidthRoundCornersStyle(QWidget *widget, QString objectName = "");
	static QString sliderStyle();
	static QString getColorBasedOnValue(QString value);

private:
	QMap<QString, QString> colorMap;
	static RegmapStyleHelper *pinstance_;
};
} // namespace scopy::regmap
#endif // REGMAPSTYLEHELPER_HPP
