#include "values.h"

/* Default background color */
QMainWindow > .QWidget, scopy--m2k--Sismograph > QwtPlotCanvas, QMessageBox, QToolTip {
	background-color: #F7F7F7;
}

/* Default settings for QWidget and its sub-classes */
QWidget {
	background-color: transparent;
	color: #141416;
	font-style: normal;
	font-weight: normal;
	font-size: 13px;
}

QDockWidget {
	titlebar-normal-icon: url(:/icons/scopy-light/icons/sba_cmb_box_arrow_right.svg);
	background-color: #F7F7F7;
}

QDockWidget::title {
	background-color: transparent;
}

QDockWidget::float-button {
	background-color: transparent;
	border: none;
}

QDockWidget::float-button:hover {
	background-color: rgba(0, 0, 0, 30);
	padding: 2px;
}

QDockWidget::float-button:hover {
	background-color: rgba(0, 0, 0, 60);
}

QToolTip {
	padding: 6px;
	border: 1px solid rgba(149, 152, 154, 150);
	color: rgba(0, 0, 0, 200);
}

QLabel {
	color: rgba(0, 0, 0, 200);
	font-size: 13px;
	font-style: normal;
	font-weight: normal;
	text-align: left;
}

QLabel:disabled, QRadioButton:disabled {color: rgba(0, 0, 0, 70); }

QLabel[invalid=true] { color: red; }

QLabel[valid=true] { color: rgba(0, 0, 0, 200); }

QTextBrowser { background-color: #EDEDED; }

/* Apply a gradient to the MenuAnim widgets */
scopy--MenuAnim, scopy--m2k--DMM #widget_2, .scopy--m2k--PowerController #rightMenu  {
	background-color: qlineargradient(spread:pad, x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #EDEDED, stop: 0.15 #F7F7F7, stop: 0.85 #F7F7F7, stop: 1.0 #EDEDED);
}

/* Except for the tool launcher's MenuAnim */
QMainWindow > .QWidget > .scopy--MenuAnim { background-color: none; }

QwtPlot, QwtPolarPlot, pv--view--Viewport, QWidget[plot_container=true] {
	background-color: white;
}

/* QwtPlot should have a black background, except the sismographs */
scopy--m2k--Sismograph { background-color: transparent; }

QwtPlotCanvas { background-color: white; }

QwtPolarCanvas { background-color: none; }

QwtScaleWidget { color: rgba(0, 0, 0, 180); }

QwtThermo {
	color: #999999;
	font-size: 26px;
}

scopy--BufferPreviewer {
	border: 1px solid #7092be;
	alternate-background-color: #4a64ff;
	selection-background-color: #F7F7F7;
	selection-color: #ff7200;
	color: white;
}

QDial {
	background-color: white;
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
	color: #7D7D83;
	border-bottom: 2px solid #f36d0a;
	margin-top: 0px;
}

QTabBar::tab:!selected {
	border-bottom: 2px solid #7D7D83;
}

QTabBar::scroller {
	width: 25px;
}

QRadioButton {
	color: rgba(0, 0, 0, 200);
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

QRadioButton::indicator:disabled { border-color: #C5C5C5; }

QRadioButton::indicator:checked:disabled { background-color: #C5C5C5; }

QLineEdit {
	color: rgba(0, 0, 0, 200);
	font-size: 16px;
	border: 0px solid gray;
	border-bottom: 1px solid rgba(0, 0, 0, 40);
	padding: 2px;
}

QLineEdit[valid=true]{ color: rgba(0, 0, 0, 200); }
QLineEdit[invalid=true]{ color: red; }

QComboBox {
	height: 24px;
	border: none;
	font-size: 14px;

	border-bottom: 1px solid rgba(0, 0, 0, 40);
	padding-bottom: 4px;
}

QComboBox:disabled, QLineEdit:disabled { color: rgba(0, 0, 0, 70); }

QComboBox QAbstractItemView {
	border: none;
	background-color: white;
	text-align: left;
	color: black;
	outline: none;

	border-bottom: 1px solid #bebebe;
	border-top: 1px solid #bebebe;

	/* This only works on Windows */
	selection-color: black;
	selection-background-color: #EDEDED;
}

/* This does not work on Windows */
QComboBox::item:selected {
	font-weight: bold;
	font-size: 18px;
	border-bottom: 0px solid none;
	background-color: #EDEDED;
}

QComboBox::drop-down {
	subcontrol-position: center right;
	border-image: url(:/icons/scopy-light/icons/sba_cmb_box_arrow.svg);
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
	color: rgba(0, 0, 0, 200);
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
	border-bottom: 1px solid rgba(0, 0, 0, 40);
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
	background: #F7F7F7;
	max-width: 8px;
}

QScrollBar::handle:vertical {
	background: #BEBEC1;
	border: 0;
	border-radius: 3px;
}
QScrollBar::handle:vertical:hover {
	background: #7D7D83;
	border: 0;
	border-radius: 3px;
}

QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
	border: none;
	background: none;
}

QScrollBar:horizontal {
	background: #F7F7F7;
	max-height: 6px;
	border-radius: 3px;
}

QScrollBar::handle:horizontal {
	background: #BEBEC1;
	border: 0;
	border-radius: 3px;
}

QScrollBar::handle:horizontal:hover {
	background: #7D7D83;
	border: 0;
	border-radius: 3px;
}

QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: none; }

QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
	border: none;
	background: none;
}

QDialog { background-color: white; }

ScopyAboutDialog QPushButton{
	background-color: #4a64ff;
	color: #ffffff;
}

QTextBrowser#aboutTextBrowser{
	background-color: transparent;
	link-color: yellow;
	selection-color: yellow;
	color: rgba(0, 0, 0, 200);
}

/* Label, line and arrow button dividers between sections */

QLabel[subsection_label=true], QLabel#lblSubsectionSeparator {
	font-size: 12px;
	color: rgba(0, 0, 0, 70);
}

QFrame[subsection_line=true], QFrame#lineSubsectionSeparator { border: 1px solid rgba(0, 0, 0, 50); }

QPushButton[subsection_arrow_button=true]{
	max-height: 12px;
	max-width: 12px;
	border: none;
	image: url(:/icons/scopy-light/icons/sba_cmb_box_arrow.svg);
}
QPushButton#btnSubsectionSeparator:checked, QPushButton#toggleChannels:checked {
	max-height: 12px;
	max-width: 12px;
	border: none;
	image: url(:/icons/scopy-light/icons/sba_cmb_box_arrow.svg);
}

QPushButton[subsection_arrow_button=true]:checked {
	max-height: 12px;
	max-width: 12px;
	border: none;
	image: url(:/icons/scopy-light/icons/sba_cmb_box_arrow_right.svg);
}

QPushButton#btnSubsectionSeparator, QPushButton#toggleChannels {
	max-height: 12px;
	max-width: 12px;
	border: none;
	image: url(:/icons/scopy-light/icons/sba_cmb_box_arrow_right.svg);
}

/* Label associated with settings menu, right above a blue line divider */

QLabel[general_settings_label=true]{
	color: rgba(0, 0, 0, 200);
	font-size: 14px;
	font-weight: normal;
}

/* Blue line divider */

QFrame[blue_line=true]{ border: 2px solid #4A64FF; }

/* Drag widget 3 vertical lines */

QFrame[drag_widget=true] { border-image:url(:/icons/scopy-light/icons/menu.svg); }

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


QWidget[device_page="true"] {
	background-color: #EDEDED;
}
QWidget[device_page="true"] QPushButton[device_page="true"] {
	min-height: 30px;
	max-height: 30px;
	min-width: 80px;
	max-width: 80px;
}

/* Menu title label */

QLabel[menu_title_label=true]{
	color: rgba(0, 0, 0, 200);
	font-weight: normal;
	font-size: 14px;
}


/* Style for all SpinBoxes */

QSpinBox{
	color: rgba(0, 0, 0, 200);
	border: 0px;
	border-bottom: 1px solid rgba(0, 0, 0, 40);
}


QCheckBox {
	spacing: 8px;
	background-color: transparent;
	font-size: 14px;
	font-weight: normal;
	color: rgba(0, 0, 0, 200);
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

QPushButton[menu_icon_button=true]:checked { border-top: 1px solid rgba(0, 0, 0, 150); }

QPushButton[menu_icon_button=true]:!checked { border-top: 1px solid rgba(0, 0, 0, 0); }

QPushButton[menu_icon_button=true]:!checked:hover { border-top: 1px solid rgba(0, 0, 0, 0); }

QPushButton[menu_icon_button=true]:checked:hover { border-top: 1px solid rgba(0, 0, 0, 200); }


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
	background-color: #C5C5C5;
}

QWidget[menu_button=true][selected=false] { background-color: transparent; }


/* Styles for DigitalIO widgets */

QWidget#page {
	border-radius: 12px;
	background-color: rgba(0, 0, 0, 40);
}

QWidget#stackedWidgetPage1 {
	border-radius: 12px;
	background-color: rgba(0, 0, 0, 80);
}


/* Background color for info page */

.scopy--m2k--StackedHomepage#stackedWidget{ background-color: #EDEDED; }


/* Style for all line separators */

QFrame[line_separator=true]{ background-color: rgba(0, 0, 0, 20); }


/* Background color for Voltmeter and Power Supply */

.scopy--m2k--PowerController #leftPanel, .scopy--m2k--DMM #leftPanel{
	background-color: #EDEDED;
}


/* Gradient background for all instruments */

.scopy--m2k--Oscilloscope #mainWidget, .scopy--m2k--SpectrumAnalyzer #mainWidget,
.scopy--m2k--NetworkAnalyzer #mainWidget, .scopy--m2k--SignalGenerator #mainWidget,
.scopy--m2k--logic--LogicAnalyzer #mainWidget, .scopy--m2k--logic--PatternGenerator #mainWidget,
.scopy--m2k--DigitalIO #widget, .scopy--m2k--DMM #hLayout_top_btn_area_voltmeter, .scopy--m2k--PowerController #hWidget_top_area  {
	background-color: qlineargradient(spread:pad, x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #EDEDED, stop: 0.1 #F7F7F7, stop: 1.0 #F7F7F7);
}

/* Remove inherited gradient from the export setting of the Oscilloscope */

.scopy--m2k--ExportSettings #mainWidget, .scopy--m2k--ImportSettings #mainWidget {
	background-color: none;
}

scopy--CursorReadouts {
	background-color: white;
}

QWidget[scopy_logo=true]{
	background-image: url(:/icons/scopy-light/icons/logo.svg);
	background-repeat: no-repeat;
	background-position: left center;
}

QPushButton[menu_icon=true]{
	background-image: url(:/icons/scopy-light/icons/menu.svg);
	background-repeat: no-repeat;
	background-position: left center;
}

QWidget[adi_logo=true]{
	background-image: url(:/icons/scopy-light/icons/logo_analog.svg);
	background-repeat: no-repeat;
	background-position: left center;
	min-width: 104px;
	min-height: 30px;
}

QPushButton[save_logo=true]{
	icon: url(:/icons/scopy-light/icons/save.svg);
	text-align: right;
}

QPushButton[load_logo=true]{
	icon: url(:/icons/scopy-light/icons/load.svg);
	text-align: left;
}

QPushButton[preferences_icon=true]{
	icon: url(:/icons/scopy-light/icons/preferences.svg);
	text-align: center;
	qproperty-text: "Preferences";
}

QPushButton[general_settings_icon=true]:checked {
	image: url(:/icons/scopy-light/icons/gear_wheel_pressed.svg);
}

QPushButton[general_settings_icon=true]:!checked {
	image: url(:/icons/scopy-light/icons/gear_wheel.svg);
}


QPushButton[general_settings_icon=true]:!checked:hover {
	image: url(:/icons/scopy-light/icons/gear_wheel_hover.svg);
}

QPushButton[general_settings_icon=true]:checked:hover {
	image: url(:/icons/scopy-light/icons/gear_wheel_pressed.svg);
}

QPushButton[settings_icon=true]:checked {
	image: url(:/icons/scopy-light/icons/setup3_checked_hover.svg);
}

QPushButton[settings_icon=true]:!checked {
	image: url(:/icons/scopy-light/icons/setup3_unchecked.svg);
}

QPushButton[settings_icon=true]:!checked:hover {
	image: url(:/icons/scopy-light/icons/setup3_unchecked_hover.svg);
}

QPushButton[settings_icon=true]:checked:hover {
	image: url(:/icons/scopy-light/icons/setup3_checked_hover.svg);
}

QHeaderView::section {
	background-color: #D3D3D3;
}

QHeaderView::section:checked {
	background-color: #D3D3D3;
}

/*************************************************************/



/*********************** Channel widget *******************/

scopy--ChannelWidget QWidget {
	color: rgba(0, 0, 0, 170);

	border-width: 0px;
	border-radius: 6px;
}

/* Widget containing box, name, btn */
scopy--ChannelWidget QWidget#widget {
	background-color: transparent;
	border-radius: 4px;
}
scopy--ChannelWidget QWidget#widget[selected=true] {
	background-color: rbga(0, 0, 0, 40);
}

/* Round check box */
scopy--ChannelWidget QCheckBox#box{
	spacing: 0px;
	background-color: none;
	font-size: 14px;
	font-weight: bold;
}

scopy--ChannelWidget QCheckBox#box::indicator {
	width: INDICATOR_SIZE;
	height: INDICATOR_SIZE;
	border: 2px solid #000000; /* Will be overwritted in the ChannelWidget constructor */
	border-radius: 9px;
}

scopy--ChannelWidget QCheckBox#box::indicator:unchecked {
	background-color: transparent;
}
scopy--ChannelWidget QCheckBox#box::indicator:checked {
	background-color: #000000; /* Will be overwritted in the ChannelWidget constructor */
}

/* Name */
scopy--ChannelWidget QPushButton#name {
	font-size: 14px;
	font-weight: bold;
	background-color: none;
}

/* Delete Button */
scopy--ChannelWidget QPushButton#delBtn {
	width: 24px;
	height: 24px;
	background-color: transparent;
	background-position: center center;
	background-repeat: no-repeat;
	background-image: url(:/icons/red_x.svg);
}
scopy--ChannelWidget QPushButton#delBtn::hover {
	background-image: url(:/icons/red_x.svg);
}

/* Menu button */
scopy--ChannelWidget QPushButton#btn {
	width: 40px;
	height: 20px;
	background-color: transparent;
}
scopy--ChannelWidget QPushButton#btn:pressed {
	border-image: url(:/icons/setup_btn_checked.svg)
}
scopy--ChannelWidget QPushButton#btn:!pressed {
	border-image: url(:/icons/setup_btn_unchecked.svg)
}
scopy--ChannelWidget QPushButton#btn:hover:!pressed:!checked {
	border-image: url(:/icons/setup_btn_hover.svg)
}
scopy--ChannelWidget QPushButton#btn:checked {
	border-image: url(:/icons/setup_btn_checked.svg)
}

/* Underline */
scopy--ChannelWidget QFrame#line {
	border: 2px solid transparent;
}
scopy--ChannelWidget QFrame#line[selected=true] {
	border: 2px solid #000000; /* Will be overwritted in the ChannelWidget constructor */
}

/*************************************************************/



/******************* Dropdown ***************/

scopy--m2k--DropdownSwitchList {
	height: 30px;
	border: 0px;
	font-size: 18px;
	border-radius: 4px;
	padding-left: 20px;
}

scopy--m2k--DropdownSwitchList:editable{
	background-color: white;
	color: rgba(0, 0, 0, 200);
}

scopy--m2k--DropdownSwitchList::drop-down {
	subcontrol-position: center right;
	width: 10px;
	height: 6px;
	border-image: url(:/icons/scopy-light/icons/sba_cmb_box_arrow.svg);
	margin-right: 20px;
}

scopy--m2k--DropdownSwitchList QAbstractItemView {
	border: 0px;
	background-color: white;
	font-size: 18px;
	outline: 0px;

	/* Add left space. Color should match background-color*/
	border-left: 0px solid white; /* setting to 0 for now */
}

scopy--m2k--DropdownSwitchList QAbstractItemView::item {
	color: rgba(0, 0, 0, 200);
	height: 60px;
}

scopy--m2k--DropdownSwitchList QAbstractItemView::item:hover {
	background-color: #EDEDED;
	font-weight: bold;
	border-bottom: 0px solid none;
}

scopy--m2k--DropdownSwitchList QHeaderView {
  /* Cancel the effect of the QAbstractItemView border-left property. It's
  necessary because the border (or padding) of the QAbstractItemView applies to
  this element as well. */
  padding-left: -0px;
}

scopy--m2k--DropdownSwitchList QHeaderView:section {
	color: rgba(0, 0, 0, 200);
	background-color: white;
	border: 0px;
	font: 14px;
}

scopy--m2k--DropdownSwitchList QCheckBox {
	background-color: white;
}

scopy--m2k--DropdownSwitchList QCheckBox::indicator {
	width: 16px;
	height: 16px;
	subcontrol-position: center;
}

/*************************************************************/



/**************** SpinBoxes *****************/

scopy--SpinBoxA QPushButton#SBA_UpButton {
	width: 30px;
	height: 30px;
	border-image: url(:/icons/sba_up_btn.svg);
	border: 0px;
}

scopy--SpinBoxA QPushButton#SBA_UpButton:pressed {
	border-image: url(:/icons/sba_up_btn_pressed.svg);
}
scopy--SpinBoxA QPushButton#SBA_UpButton:hover:!pressed {
	border-image: url(:/icons/sba_up_btn_hover.svg);
}

scopy--SpinBoxA QPushButton#SBA_DownButton {
	width: 30px;
	height: 30px;
	border-image: url(:/icons/sba_dn_btn.svg);
	border: 0px;
}
scopy--SpinBoxA QPushButton#SBA_DownButton:pressed {
	border-image: url(:/icons/sba_dn_btn_pressed.svg);
}
scopy--SpinBoxA QPushButton#SBA_DownButton:hover:!pressed {
	border-image: url(:/icons/sba_dn_btn_hover.svg);
}

scopy--SpinBoxA QLabel#SBA_Label {
	color: rgba(0, 0, 0, 200);
	font-size: 14px;
}

scopy--SpinBoxA QLineEdit#SBA_LineEdit {
	height: 20px;
	width: 75px;
	font-size: 18px;
	border: 0px;
	bottom: 10px;
}

scopy--SpinBoxA QFrame#SBA_Line {
	color: #4a64ff;
}

scopy--SpinBoxA QFrame#SBA_Line:disabled {
	color: #555555;
}

scopy--SpinBoxA QComboBox#SBA_Combobox {
	height: 20px;
	font-size: 12px;
	font-weight: normal;
	border-bottom: 0px;
	padding-bottom: 0px;
}

scopy--SpinBoxA QComboBox#SBA_Combobox::drop-down {
	subcontrol-position: center right;
	width: 10px;
	height: 6px;
	border-image: url(:/icons/scopy-light/icons/sba_cmb_box_arrow.svg);
}

scopy--SpinBoxA QComboBox#SBA_Combobox::drop-down:disabled {
	subcontrol-position: center right;
	width: 0px;
	height: 0px;
	border-image: url(:/icons/scopy-light/icons/sba_cmb_box_arrow.svg);
}

scopy--SpinBoxA QDial#SBA_CompletionCircle {
	background-color: #C5C5C5;
	color: #4963ff;
}

/*************************************************************/


/******************** Stop buttons *************************/

QPushButton[stopButton=true] {
	background-repeat: no-repeat;
	background-position: center center;
}

QPushButton[stopButton=true]:enabled {
	background-image: url(:/icons/scopy-light/icons/play_stop.svg);
}

QPushButton[stopButton=true][disabled=true][enabled=false] {
	background-image: url(:/icons/scopy-light/icons/play_stop.svg);
}

QPushButton[stopButton=true]:checked {
	background-image: url(:/icons/play_green.svg);
}

/*************************************************************/

/******************** User Notes *************************/

QPushButton[userNote=true] {
	background-image: url(:/icons/scopy-light/icons/note_unchecked.svg);
}

QPushButton[userNote=true]:checked {
	background-image: url(:/icons/scopy-light/icons/note_checked.svg);
}

/*************************************************************/
