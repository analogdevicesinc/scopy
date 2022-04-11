#include "values.h"

/* Default background color */
QMainWindow > .QWidget, adiscope--Sismograph > QwtPlotCanvas, QMessageBox, QToolTip {
	background-color: #272730;
}

/* Default settings for QWidget and its sub-classes */
QWidget {
	background-color: transparent;
	color: #bebebe;
	font-style: normal;
	font-weight: normal;
	font-size: 13px;
}

QDockWidget {
	titlebar-normal-icon: url(:/icons/scopy-default/icons/sba_cmb_box_arrow_right.svg);
}

QDockWidget::title {
	background-color: transparent;
}

QDockWidget::float-button {
	background-color: transparent;
	border: none;
}

QDockWidget::float-button:hover {
	background-color: rgba(255, 255, 255, 30);
	padding: 2px;
}

QDockWidget::float-button:hover {
	background-color: rgba(0, 0, 0, 60);
}

QToolTip {
	padding: 6px;
	border: 1px solid rgba(149, 152, 154, 150);
	color: white;
}

QLabel {
	color: rgba(255, 255, 255, 150);
	font-size: 13px;
	font-style: normal;
	font-weight: normal;
	text-align: left;
}

QLabel:disabled, QRadioButton:disabled {color: rgba(255, 255, 255, 80); }

QLabel[invalid=true] { color: red; }

QLabel[valid=true] { color: rgba(255, 255, 255, 150) }

QTextBrowser { background-color: rgba(0, 0, 0, 150); }

/* Apply a gradient to the MenuAnim widgets */
adiscope--MenuAnim, adiscope--DMM #widget_2, .adiscope--PowerController #rightMenu {
	background-color: qlineargradient(spread:pad, x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgba(0, 0, 0, 40), stop: 0.15 transparent, stop: 0.85 transparent, stop: 1.0 rgba(0, 0, 0, 40));
}

/* Except for the tool launcher's MenuAnim */
QMainWindow > .QWidget > .adiscope--MenuAnim { background-color: none; }

QwtPlot, QwtPolarPlot, pv--view--Viewport, QWidget[plot_container=true]{
	background-color: black;
}

/* QwtPlot should have a black background, except the sismographs */
adiscope--Sismograph { background-color: transparent; }

QwtPlotCanvas { background-color: #141416; }

QwtPolarCanvas { background-color: none; }

QwtScaleWidget { color: rgba(255, 255, 255, 180); }

QwtThermo {
	color: #999999;
	font-size: 26px;
}

adiscope--BufferPreviewer {
	border: 1px solid #7092be;
	alternate-background-color: #4a64ff;
	selection-background-color: #141416;
	selection-color: #ff7200;
	color: white;
}

QDial {
	background-color: black;
	color: #4963ff;
}

QTabWidget::tab-bar { left: 0; }

QTabWidget::pane { border-top: 0px; }

QTabBar { qproperty-drawBase: 0; }

QTabBar::tab {
	min-width: 100px;
	min-height: 32px;
	padding-bottom: 5px;
	font: normal;
}

QTabBar::tab:selected {
	color: white;
	border-bottom: 2px solid #f36d0a;
	margin-top: 0px;
}

QTabBar::tab:!selected {
	border-bottom: 2px solid #373740;
}

QTabBar::scroller {
	width: 25px;
}

QRadioButton {
	color: white;
	spacing: 12px;
}

QRadioButton::indicator {
	width: INDICATOR_SIZE;
	height: INDICATOR_SIZE;
	border: 2px solid;
	border-radius: 9px;
	border-color: #4963FF;
}

QRadioButton::indicator:checked { background-color: #4963FF; }

QRadioButton::indicator:disabled { border-color: #555555; }

QRadioButton::indicator:checked:disabled { background-color: #555555; }

QLineEdit {
	color: white;
	font-size: 16px;
	border: 0px solid gray;
	border-bottom: 1px solid rgba(255, 255, 255, 102);
	padding: 2px;
}

QLineEdit[valid=true]{ color: white; }
QLineEdit[invalid=true]{ color: red; }

QComboBox {
	height: 24px;
	border: none;
	font-size: 14px;

	border-bottom: 1px solid rgba(255, 255, 255, 102);
	padding-bottom: 4px;
}

QComboBox:disabled, QLineEdit:disabled { color: #555555; }

QComboBox QAbstractItemView {
	border: none;
	background-color: #1b1b21;
	text-align: left;
	color: #bebebe;
	outline: none;

	border-bottom: 1px solid #bebebe;
	border-top: 1px solid #bebebe;
}

/* This only works on Windows */
QComboBox QAbstractItemView {
	selection-background-color: #272730;
}

/* This does not work on Windows */
QComboBox::item:selected {
	font-weight: bold;
	font-size: 18px;
	border-bottom: 0px solid none;
	background-color: #272730;
}

QComboBox::drop-down {
	subcontrol-position: center right;
	border-image: url(:/icons/scopy-default/icons/sba_cmb_box_arrow.svg);
	width: 10px;
	height: 6px;
	font-size: 16px;
	text-align: left;
}

QComboBox::indicator {
	background-color: transparent;
	selection-background-color: transparent;
	color: transparent;
	selection-color: transparent;
}

QMessageBox QLabel {
	height: 420px;
	width: 680px;
	color: rgba(255,255,255,240);
	font-size: 14px;
	font: bold;
}

QMessageBox QPushButton {
	height: 20px;
	width: 120px;
	background-color: #4a64ff;
	color: white;
	font-size: 16px;
	font: bold;
	border-radius: 5px;
	border-color: #4a64ff;
	border-style: solid;
	border-width: 1px;
}

QAbstractSpinBox {
	height: 45px;
	font-size: 16px;
	font-weight: bold;
	border-bottom: 1px solid rgba(255, 255, 255, 102);
}

QAbstractSpinBox::up-button, QAbstractSpinBox::down-button {
	height: 20px;
	width: 20px;
}

QAbstractSpinBox::up-button { border-image: url(:/icons/sba_up_btn.svg); }

QAbstractSpinBox::up-button:hover { border-image: url(:/icons/sba_up_btn_hover.svg); }

QAbstractSpinBox::up-button:pressed { border-image: url(:/icons/sba_up_btn_pressed.svg); }

QAbstractSpinBox::down-button { border-image: url(:/icons/sba_dn_btn.svg); }

QAbstractSpinBox::down-button:hover { border-image: url(:/icons/sba_dn_btn_hover.svg); }

QAbstractSpinBox::down-button:pressed { border-image: url(:/icons/sba_dn_btn_pressed.svg); }

QGroupBox {
	border: 2px solid rgba(255, 255, 255, 40);
	border-radius: 8px;
	color: rgba(255, 255, 255, 40);
	font-size: 16px;
	font-weight: bold;
	margin-top: 9px;
	padding-top: 5px;
}

QGroupBox::title {
	subcontrol-origin: margin;
	subcontrol-position: top left; /* position at the top center */
}

QScrollArea { border: 0px;}

QScrollBar:vertical {
	background: #262628;
	max-width: 8px;
}

QScrollBar::handle:vertical {
	background: #404040;
	border: 0;
	border-radius: 3px;
}
QScrollBar::handle:vertical:hover {
	background: #4a4a4b;
	border: 0;
	border-radius: 3px;
}

QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
	border: none;
	background: none;
}

QScrollBar:horizontal {
	background: #262628;
	max-height: 6px;
	border-radius: 3px;
}

QScrollBar::handle:horizontal {
	background: #404040;
	border: 0;
	border-radius: 3px;
}

QScrollBar::handle:horizontal:hover {
	background: #4a4a4b;
	border: 0;
	border-radius: 3px;
}

QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: none; }

QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
	border: none;
	background: none;
}

QDialog { background-color: #1b1b21; }

ScopyAboutDialog QPushButton{
	background-color: #4a64ff;
	color: #ffffff;
}

QTextBrowser#aboutTextBrowser{
	background-color: transparent;
	link-color: yellow;
	selection-color: yellow;
	color: white;
}


/* Label associated with settings menu, right above a blue line divider */

/*********************************** SubsectionSeparator ***************************************/
QLabel[subsection_label=true], QLabel#lblSubsectionSeparator{
        font-size: 12px;
	color: rgba(255, 255, 255, 70);
}
QFrame[subsection_line=true], QFrame#lineSubsectionSeparator{ border: 1px solid rgba(255, 255, 255, 70); }
QPushButton[subsection_arrow_button=true]{
	max-height: 12px;
	max-width: 12px;
	border: none;
	image: url(:/icons/scopy-default/icons/sba_cmb_box_arrow.svg);
}
QPushButton#btnSubsectionSeparator:checked, QPushButton#toggleChannels:checked {
	max-height: 12px;
	max-width: 12px;
	border: none;
	image: url(:/icons/scopy-default/icons/sba_cmb_box_arrow.svg);
}

QPushButton[subsection_arrow_button=true]:checked {
	max-height: 12px;
	max-width: 12px;
	border: none;
	image: url(:/icons/scopy-default/icons/sba_cmb_box_arrow_right.svg);
}

QPushButton#btnSubsectionSeparator, QPushButton#toggleChannels {
	max-height: 12px;
	max-width: 12px;
	border: none;
	image: url(:/icons/scopy-default/icons/sba_cmb_box_arrow_right.svg);
}

QLabel[general_settings_label=true]{
	color: white;
	font-size: 14px;
	font-weight: normal;
}

/* Blue line divider */

QFrame[blue_line=true]{ border: 2px solid #4A64FF; }

/* Drag widget 3 vertical lines */

QFrame[drag_widget=true] { border-image:url(:/icons/scopy-default/icons/menu.svg); }

/* General style for blue buttons */

QPushButton[blue_button=true]{
	background-color: #4a64ff;
	color: #ffffff;
	border-radius: 4px;
	font-size: 14px;
}

QPushButton[blue_button=true]:pressed{ background-color: #2a44df; }

#ifndef __ANDROID__
QPushButton[blue_button=true]:hover{ background-color: #4a34ff; }
#endif

QPushButton[blue_button=true]:disabled { background-color: grey; }

/* Info button for each tool */

QPushButton[info_button=true]{
	min-height: 40px;
	max-height: 40px;
	min-width: 40px;
	max-width: 40px;
	background-color: transparent;
	border-radius: 4px;
}

#ifndef __ANDROID__
QPushButton[info_button=true]:hover { background-color: rgba(0, 0, 0, 60); }
#endif

/* Menu title label */

QLabel[menu_title_label=true]{
	color: white;
	font-weight: normal;
	font-size: 14px;
}


/* Style for all SpinBoxes */

QSpinBox{
	color: white;
	border: 0px;
	border-bottom: 1px solid rgba(255, 255, 255, 100);
}


QCheckBox {
	spacing: 8px;
	background-color: transparent;
	font-size: 14px;
	font-weight: bold;
	color: rgba(255, 255, 255, 153);
}

QCheckBox::indicator {
	width: INDICATOR_SIZE;
	height: INDICATOR_SIZE;
	border: 2px solid rgb(74,100,255);
	border-radius: 4px;
}

QCheckBox::indicator:unchecked { background-color: transparent; }
QCheckBox::indicator:checked { background-color: rgb(74,100,255); }


QSlider::groove {
	border: 1px solid #444444;
	height: 2px; /* the groove expands to the size of the slider by default. by giving it a height, it has a fixed size */
	background: #999999;
	margin: 2px 0;
	border-radius: 2px;
}

QSlider::handle {
	background: rgb(73, 99, 255);
	border: 0px solid;
	width: 18px;
	margin: -8px 0; /* handle is placed by default on the contents rect of the groove. Expand outside the groove */
	border-radius: 8px;
}

QSlider::handle:disabled { background: #444444; }

QSlider::sub-page:horizontal { background: #4A64FF; }


/* Style for settings and general settings buttons from all menus */

QPushButton[menu_icon_button=true] {
	min-height: 40px;
	max-height: 40px;
	min-width: 60px;
	max-width: 60px;
	padding-top:5px
}

QPushButton[menu_icon_button=true]:checked { border-top: 1px solid rgba(255,255,255,150); }

QPushButton[menu_icon_button=true]:!checked { border-top: 1px solid rgba(255,255,255,0); }

QPushButton[menu_icon_button=true]:!checked:hover { border-top: 1px solid rgba(255,255,255,0); }

QPushButton[menu_icon_button=true]:checked:hover { border-top: 1px solid rgba(255,255,255,200); }


/* Style for all Tool launcher buttons, such as Home, Menu, Preferences, Load, Save, Notes, etc. */

QPushButton[tool_launcher_custom_widget=true] {
	text-align:left;
	border: none;
	background-color: none;
}

#ifndef __ANDROID__
.QWidget[tool_launcher_custom_widget=true]:hover {
	background-color: rgba(0, 0, 0, 60);
	border: 1px solid rgba(0, 0, 0, 30);
	border-radius:5px;
}
#endif

#ifdef __ANDROID__
.QWidget[tool_launcher_custom_widget=true]:pressed {
	background-color: rgba(0, 0, 0, 60);
	border: 1px solid rgba(0, 0, 0, 30);
	border-radius:5px;
}
#endif

QWidget[tool_launcher_custom_widget=true][selected=true] {
	background-color: rgba(0, 0, 0, 60);
	border: 1px solid rgba(0, 0, 0, 30);
	border-radius:5px;
}


/* Style for icon-buttons such as Device, Home, Plus, etc. */

QWidget[menu_button=true][selected=true] {
	opacity: 0.6;
	border-radius: 4px;
	background-color: #141416;
}

QWidget[menu_button=true][selected=false] { background-color: transparent; }


/* Styles for DigitalIO widgets */

QWidget#page {
	border-radius: 12px;
	background-color: rgba(0, 0, 0, 80);
}

QWidget#stackedWidgetPage1 {
	border-radius: 12px;
	background-color: #141416;
}


/* Background color for info page */

.adiscope--StackedHomepage#stackedWidget{ background-color: black; }


/* Style for all line separators */

QFrame[line_separator=true]{ background-color: rgba(255, 255, 255, 16); }


/* Background color for Voltmeter and Power Supply */

.adiscope--PowerController #leftPanel, .adiscope--DMM #leftPanel{
	background-color: rgba(0, 0, 0, 40);
}


/* Gradient background for all instruments */

.adiscope--Oscilloscope #mainWidget, .adiscope--SpectrumAnalyzer #mainWidget,
.adiscope--NetworkAnalyzer #mainWidget, .adiscope--SignalGenerator #mainWidget,
.adiscope--logic--LogicAnalyzer #mainWidget, .adiscope--logic--PatternGenerator #mainWidget,
.adiscope--DigitalIO #widget, .adiscope--DMM #hLayout_top_btn_area_voltmeter, .adiscope--PowerController #hWidget_top_area {
	background-color: qlineargradient(spread:pad, x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 rgba(0, 0, 0, 40), stop: 0.1 transparent, stop: 1.0 transparent);
}

/* Remove inherited gradient from the export setting of the Oscilloscope */

.adiscope--ExportSettings #mainWidget, .adiscope--ImportSettings #mainWidget {
	background-color: none;
}

adiscope--CursorReadouts {
	background-color: black;
}

QWidget[scopy_logo=true]{
	background-image: url(:/icons/scopy-default/icons/logo.svg);
	background-repeat: no-repeat;
	background-position: left center;
}

QPushButton[menu_icon=true]{
	background-image: url(:/icons/scopy-default/icons/menu.svg);
	background-repeat: no-repeat;
	background-position: left center;
}

QWidget[adi_logo=true]{
	background-image: url(:/icons/scopy-default/icons/logo_analog.svg);
	background-repeat: no-repeat;
	background-position: left center;
	min-width: 104px;
	min-height: 30px;
}

QPushButton[save_logo=true]{
	icon: url(:/icons/scopy-default/icons/save.svg);
	text-align: right;
}

QPushButton[load_logo=true]{
	icon: url(:/icons/scopy-default/icons/load.svg);
	text-align: left;
}

QPushButton[preferences_icon=true]{
	icon: url(:/icons/scopy-default/icons/preferences.svg);
	text-align: center;
	qproperty-text: "Preferences";
}

QPushButton[general_settings_icon=true]:checked {
	image: url(:/icons/scopy-default/icons/gear_wheel_pressed.svg);
}

QPushButton[general_settings_icon=true]:!checked {
	image: url(:/icons/scopy-default/icons/gear_wheel.svg);
}


QPushButton[general_settings_icon=true]:!checked:hover {
	image: url(:/icons/scopy-default/icons/gear_wheel_hover.svg);
}

QPushButton[general_settings_icon=true]:checked:hover {
	image: url(:/icons/scopy-default/icons/gear_wheel_pressed.svg);
}

QPushButton[settings_icon=true]:checked {
	image: url(:/icons/scopy-default/icons/setup3_checked_hover.svg);
}

QPushButton[settings_icon=true]:!checked {
	image: url(:/icons/scopy-default/icons/setup3_unchecked.svg);
}

QPushButton[settings_icon=true]:!checked:hover {
	image: url(:/icons/scopy-default/icons/setup3_unchecked_hover.svg);
}

QPushButton[settings_icon=true]:checked:hover {
	image: url(:/icons/scopy-default/icons/setup3_checked_hover.svg);
}

QHeaderView::section {
	background-color: transparent;
}

QHeaderView::section:checked {
	background-color: transparent;
}

/*************************************************************/



/************************** Channel widget **************************/

adiscope--ChannelWidget QWidget {
	color: rgba(255, 255, 255, 153);

	border-width: 0px;
	border-radius: 6px;
}

/* Widget containing box, name, btn */

adiscope--ChannelWidget QWidget#widget {
	background-color: transparent;
	border-radius: 4px;
}
adiscope--ChannelWidget QWidget#widget[selected=true] {
	background-color: rbga(20, 20, 22, 153);
}


.adiscope--ChannelWidget QWidget#widget:hover {
	background-color: rbga(20, 20, 22, 153);
	font-weight: bold;
	border-bottom: 0px solid none;
}

/* Round check box */
adiscope--ChannelWidget QCheckBox#box{
	spacing: 0px;
	background-color: none;
	font-size: 14px;
	font-weight: bold;
}

adiscope--ChannelWidget QCheckBox#box::indicator {
	width: INDICATOR_SIZE;
	height: INDICATOR_SIZE;
  border: 2px solid #000000; /* Will be overwritted in the ChannelWidget constructor */
	border-radius: 9px;
}

adiscope--ChannelWidget QCheckBox#box::indicator:unchecked {
	background-color: transparent;
}
adiscope--ChannelWidget QCheckBox#box::indicator:checked {
  background-color: #000000; /* Will be overwritted in the ChannelWidget constructor */
}

/* Name */
adiscope--ChannelWidget QPushButton#name {
	font-size: 14px;
	font-weight: bold;
	background-color: none;
}

/* Delete Button */
adiscope--ChannelWidget QPushButton#delBtn {
	width: 24px;
	height: 24px;
	background-color: transparent;
	background-position: center center;
	background-repeat: no-repeat;
	background-image: url(:/icons/red_x.svg);
}
adiscope--ChannelWidget QPushButton#delBtn::hover {
	background-image: url(:/icons/red_x.svg);
}

/* Menu button */
adiscope--ChannelWidget QPushButton#btn {
	width: 40px;
	height: 20px;
	background-color: transparent;
}
adiscope--ChannelWidget QPushButton#btn:pressed {
	border-image: url(:/icons/setup_btn_checked.svg)
}
adiscope--ChannelWidget QPushButton#btn:!pressed {
	border-image: url(:/icons/setup_btn_unchecked.svg)
}
adiscope--ChannelWidget QPushButton#btn:hover:!pressed:!checked {
	border-image: url(:/icons/setup_btn_hover.svg)
}
adiscope--ChannelWidget QPushButton#btn:checked {
	border-image: url(:/icons/setup_btn_checked.svg)
}

/* Underline */
adiscope--ChannelWidget QFrame#line {
	border: 2px solid transparent;
}
adiscope--ChannelWidget QFrame#line[selected=true] {
  border: 2px solid #000000; /* Will be overwritted in the ChannelWidget constructor */
}

/*************************************************************/



/******************** Dropdown switch list ************************/

adiscope--DropdownSwitchList {
	height: 30px;
	border: 0px;
	font-size: 18px;
	border-radius: 4px;
	padding-left: 20px;
}

adiscope--DropdownSwitchList:editable{
	background-color: #141416;
	color: white;
}

adiscope--DropdownSwitchList::drop-down {
	subcontrol-position: center right;
	width: 10px;
	height: 6px;
	border-image: url(:/icons/scopy-default/icons/sba_cmb_box_arrow.svg);
	margin-right: 20px;
}

adiscope--DropdownSwitchList QAbstractItemView {
	border: 0px;
	background-color: #141416;
	font-size: 18px;
	outline: 0px;

  /* Add left space. Color should match background-color*/
  border-left: 0px solid #141416; /* setting to 0 for now */
}

adiscope--DropdownSwitchList QAbstractItemView::item {
	color: #ffffff;
	height: 60px;
}

adiscope--DropdownSwitchList QAbstractItemView::item:hover {
	background-color: #141416;
	font-weight: bold;
	border-bottom: 0px solid none;
}

adiscope--DropdownSwitchList QHeaderView {
  /* Cancel the effect of the QAbstractItemView border-left property. It's
  necessary because the border (or padding) of the QAbstractItemView applies to
  this element as well. */
	padding-left: -0px;
}

adiscope--DropdownSwitchList QHeaderView:section {
	color: rgba(255, 255, 255, 153);
	background-color: #141416;
	border: 0px;
	font: 14px;
}

adiscope--DropdownSwitchList QCheckBox {
	background-color: #141416;
}

adiscope--DropdownSwitchList QCheckBox::indicator {
	width: 16px;
	height: 16px;
	subcontrol-position: center;
}

/*************************************************************/



/************************* SpinBoxes *************************/

adiscope--SpinBoxA QPushButton#SBA_UpButton {
	width: 30px;
	height: 30px;
	border-image: url(:/icons/sba_up_btn.svg);
	border: 0px;
}

adiscope--SpinBoxA QPushButton#SBA_UpButton:pressed {
	border-image: url(:/icons/sba_up_btn_pressed.svg);
}
adiscope--SpinBoxA QPushButton#SBA_UpButton:hover:!pressed {
	border-image: url(:/icons/sba_up_btn_hover.svg);
}

adiscope--SpinBoxA QPushButton#SBA_DownButton {
	width: 30px;
	height: 30px;
	border-image: url(:/icons/sba_dn_btn.svg);
	border: 0px;
}
adiscope--SpinBoxA QPushButton#SBA_DownButton:pressed {
	border-image: url(:/icons/sba_dn_btn_pressed.svg);
}
adiscope--SpinBoxA QPushButton#SBA_DownButton:hover:!pressed {
	border-image: url(:/icons/sba_dn_btn_hover.svg);
}

adiscope--SpinBoxA QLabel#SBA_Label {
	color: rgba(255, 255, 255, 102);
	font-size: 14px;
}

adiscope--SpinBoxA QLineEdit#SBA_LineEdit {
	height: 20px;
	width: 75px;
	font-size: 18px;
	border: 0px;
	bottom: 10px;
}

adiscope--SpinBoxA QFrame#SBA_Line {
	color: #4a64ff;
}

adiscope--SpinBoxA QFrame#SBA_Line:disabled {
	color: #555555;
}

adiscope--SpinBoxA QComboBox#SBA_Combobox {
	height: 20px;
	font-size: 12px;
	font-weight: normal;
	border-bottom: 0px;
	padding-bottom: 0px;
}

adiscope--SpinBoxA QComboBox#SBA_Combobox::drop-down {
	subcontrol-position: center right;
	width: 10px;
	height: 6px;
	border-image: url(:/icons/scopy-default/icons/sba_cmb_box_arrow.svg);
}

adiscope--SpinBoxA QComboBox#SBA_Combobox::drop-down:disabled {
	subcontrol-position: center right;
	width: 0px;
	height: 0px;
	border-image: url(:/icons/scopy-default/icons/sba_cmb_box_arrow.svg);
}

adiscope--SpinBoxA QDial#SBA_CompletionCircle {
	background-color: black;
	color: #4963ff;
}
/*************************************************************/


/******************** Stop buttons *************************/

QPushButton[stopButton=true] {
	background-repeat: no-repeat;
	background-position: center center;
}

QPushButton[stopButton=true]:enabled {
	background-image: url(:/icons/scopy-default/icons/play_stop.svg);
}

QPushButton[stopButton=true][disabled=true][enabled=false] {
	background-image: url(:/icons/scopy-default/icons/play_stop.svg);
}

QPushButton[stopButton=true]:checked {
	background-image: url(:/icons/play_green.svg);
}

/*************************************************************/

/******************** User Notes *************************/

QPushButton[userNote=true] {
	background-image: url(:/icons/scopy-default/icons/note_unchecked.svg);
}

QPushButton[userNote=true]:checked {
	background-image: url(:/icons/scopy-default/icons/note_checked.svg);
}

/*************************************************************/
