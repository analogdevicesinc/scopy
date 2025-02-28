.. list-table::
   :header-rows: 1
   :widths: 20 40 50

   * - ID
     - Title
     - Description
   * - general_save_session
     - Save/Load Scopy session
     - Allow Scopy to automatically save/load the session     using .ini files into a predefined location.
   * - general_save_attached
     - Save/Load tool attached state
     - Allow Scopy to save the state of all instruments, whether they were      detached/attached when the application was closed, and load this      state accordingly when the application restarts.
   * - general_doubleclick_attach
     - Doubleclick to attach/detach tool
     - Indicates whether double-clicking attaches instruments in the application.      Enabling this option allows the user to quickly attach instruments by      double-clicking on them.
   * - general_doubleclick_ctrl_opens_menu
     - Doubleclick control buttons to open menu
     - Enable the double click action for menu opening.
   * - general_use_opengl
     - Enable OpenGL plotting
     - Use OpenGL accelerated plotting in order to speed-up the process and      avoid blocking the application while large numbers of samples are      displayed. This feature may not be supported on all systems.
   * - general_use_animations
     - Enable menu animations
     - Enable a smooth sliding transition effect for menus when they are      opened or closed.
   * - general_check_online_version
     - Enable automatic online check for updates.
     - Indicates whether the application should check for online version updates.      Enabling this option allows the application to automatically check for and      notify the user of available updates at startup.
   * - general_show_status_bar
     - Enable the status bar for displaying important messages.
     - Indicates whether the status bar should be displayed in the application. This setting allows the user to enable or disable the status bar, which provides information about the current state of the application.Important messages such as connection done or alerts are displayed here.
   * - show_grid
     - Show Grid
     - Indicates whether the plot grid is visible across the application.     The user can enable or disable this in order to configure the 
   * - show_graticule
     - Show Graticule
     - This setting allows the user to enable or disable the display of the graticule      on all the plots in the application for better signal visualization.
   * - iiowidgets_use_lazy_loading
     - Use Lazy Loading
     - Indicates whether lazy loading should be used for IIO widgets.      Enabling this option allows the application to load IIO widgets only      when they are needed, improving startup performance.
   * - general_use_native_dialogs
     - Use native dialogs
     - Indicates whether native dialogs should be used in the application.     This setting allows the user to enable or disable the use of native file      dialogs and other system dialogs within the application.
   * - general_scan_for_devices
     - Regularly scan for new devices
     - Select whether Scopy should be able to search for new devices periodically,      allowing the application to automatically populate the device list with connected     devices. Otherwise, all devices need to be added manually from the Add page.
   * - autoconnect_previous
     - Auto-connect to previous session
     - Automatically use the saved URI to connect to previous devices,       if available, when the application starts. This speeds up the       connection process and reduces the number of necessary steps.
   * - general_show_plot_fps
     - Show plot FPS
     - Indicates whether the frames per second should be displayed in plots      across the entire application.
   * - general_theme
     - Theme
     - Theme setting for the application interface. The user can choose between Harmonic style (light or dark
   * - general_language
     - Language
     - Language setting for the application interface. Multiple options available, check out the documentation if a new language is needed
   * - general_plot_target_fps
     - Plot target FPS
     - Select the Frames per second value for rendering plots in the entire application.
   * - regmap_color_by_value
     - Use color to reflect value
     - Select from various configurations available which elements in the Register Map  are color coded for better visual interpretation.
   * - adc_acquisition_timeout
     - ADC Acquisition timeout
     - Select the timeout for the I/O operation. A valid value is a positive integer representing the time in milliseconds after which a timeout should occur. 
   * - adc_plot_xaxis_label_position
     - Plot X-Axis scale position
     - Select whether the X-Axis labels are displayed on the top or bottom of the plot. Only applied after restart.
   * - adc_plot_yaxis_label_position
     - Plot Y-Axis scale position
     - Select whether the Y-Axis labels are displayed on the left or right side of the       plot. Only applied after restart.
   * - adc_plot_yaxis_handle_position
     - Plot channel Y-handle position
     - Select whether the Y-Axis handle is located on the left or right side of the plot.Only applied after restart.
   * - adc_plot_xcursor_position
     - Plot X-Cursor position
     - Select whether the X-Axis cursor handles are located on the top or bottom of the plot.Only applied after restart.
   * - adc_plot_ycursor_position
     - Plot Y-Cursor position
     - Select whether the Y-Axis cursor handles are located on the left or right side of the plot.Only applied after restart.
   * - adc_default_y_mode
     - ADC Default Y-Mode
     - Select the Y-Axis default mode, can be either ADC Counts or %Full Scale. This is also controllable while running, from the instrument settings.Only applied after restart.
   * - m2k_instrument_notes_active
     - Instrument Notes
     - Enable or disable a text section at the bottom of each ADALM2000 instrument, allowing the user to take notes. Disabled by default.
   * - m2k_show_adc_filters
     - Show ADC digital filter config
     - Enable/disable the configuration panel for ADC digital filters.     This allows manual control over the parameters of the filters.
   * - m2k_show_graticule
     - Enable graticule
     - Select whether the plot backbone (graticule
   * - m2k_mini_histogram
     - Enable mini histogram
     - Select whether the Oscilloscope plot displays a histogram on the right side.
   * - m2k_osc_filtering
     - Enable sample rate filters
     - Enable or disable the ADALM2000 ADC filters for all samplerates less than      100 MHz.
   * - m2k_osc_label
     - Enable labels on the plot
     - Enable or disable the plot labels in the Oscilloscope instrument.
   * - m2k_spectrum_visible_peak_search
     - Only search marker peaks in visible domain
     - Enabled by default, this control allows the Spectrum Analyzer to search for marker peaks outside the visible frequency range.
   * - m2k_na_show_zero
     - Always display 0db value on the graph
     - Select whether the 0dB value is displayed as a reference point on the     Network Analyzer plot. This control is disabled by default.
   * - m2k_logic_separate_annotations
     - Separate decoder annotations when exporting
     - Select whether to export decoder annotations and data separately from the      Logic Analyzer instrument. Disabled by default.
   * - m2k_logic_display_sampling_points
     - Display sampling points when zoomed
     - Select whether to display each sampling point when zoomed in on the Logic Analyzer.Disabled by default.
   * - m2k_logic_display_sample_time
     - Show sample and time info in decoder table
     - Select whether the sample and time detailed information is shown in the      decoder table of the Logic Analyzer. Enabled by default.
   * - m2k_siggen_periods
     - Number of displayed periods
     - Select the number of displayed signal periods in the Signal Generator.Default value is 2.
   * - plugins_use_debugger_v2
     - Use Debugger V2 plugin
     - Switch between the old style debugger instrument, having     just basic controls over attributes or the new instrument     type, IIO Explorer, having a tree-like structure.
   * - debugger_v2_include_debugfs
     - Include debug attributes in IIO Explorer
     - Enable/disable the use of advanced debug attributes in the instrument.
   * - dataloggerplugin_date_time_format
     - DateTime format :
     - Select the date time format of the instrument. Default value is: hh:mm:ss
   * - dataloggerplugin_data_storage_size
     - Maximum data stored for each monitor
     - Select the maximum data storage size for each monitor in the datalogger.
