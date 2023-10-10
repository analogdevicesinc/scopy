#ifndef REGMAPSTYLEHELPER_HPP
#define REGMAPSTYLEHELPER_HPP

#include <QCheckBox>
#include <QLabel>
#include <QMap>
#include <QObject>
#include <QPushButton>

#include <register/bitfield/bitfielddetailedwidget.hpp>
#include <register/registersimplewidget.hpp>

namespace scopy::regmap {

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

	static void RegisterMapStyle(QWidget *widget);
	static void PartialFrameWidget(QWidget *widget);
	static void FrameWidget(QWidget *widget);
	static void SettingsMenu(QWidget *widget);
	static void bigTextLabelStyle(QLabel *label, QString objectName);
	static void labelStyle(QLabel *label, QString objectName);

	static QString grayBackgroundHoverWidget(QWidget *widget, QString objectName = "");
	static QString BlueButton(QPushButton *btn, QString objectName = "");
	static QString checkboxStyle(QCheckBox *checkbox, QString objectName = "");
	static QString detailedBitFieldStyle(BitFieldDetailedWidget *widget, QString objectName);
	static QString simpleRegisterStyle(RegisterSimpleWidget *widget, QString objectName);
	static QString valueLabel(QLabel *label, QString objectName = "");
	static QString grayLabel(QLabel *label, QString objectName = "");
	static QString whiteSmallTextLable(QLabel *label, QString objectName = "");
	static QString frameBorderHover(QFrame *frame, QString objectName = "");

private:
	QMap<QString, QString> colorMap;
	static RegmapStyleHelper *pinstance_;
};
} // namespace scopy::regmap
#endif // REGMAPSTYLEHELPER_HPP
