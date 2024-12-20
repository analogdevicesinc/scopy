function main() {
    var prefs = scopy.getPreferences();
    var keys = Object.keys(prefs);
    var rst_data = "";
    var output_rst_file = "preferences_table.rst";
    rst_data += "Preferences\n";
    rst_data += "===========\n\n";
    rst_data += ".. list-table::\n";
    rst_data += "   :header-rows: 1\n";
    rst_data += "   :widths: 20 30 50\n\n";
    rst_data += "   * - PreferenceId\n";
    rst_data += "     - Title\n";
    rst_data += "     - Description\n";

    keys.forEach(element => {
        var prefsDetails = scopy.getPreferenceDescription(element);
        if (prefsDetails === undefined) {
            printToConsole("Error: Preference details not found for " + element);
        }
        var title = (prefsDetails[0] !== undefined )? prefsDetails[0] : "---";
        var description = (prefsDetails[1] !== undefined )? prefsDetails[1] : "---";
        rst_data += "   * - " + element + "\n";
        rst_data += "     - " + title + "\n";
        rst_data += "     - " + description + "\n";
    });
    fileIO.writeToFile(rst_data, output_rst_file);
    printToConsole("Documentation has been generated in " + output_rst_file);
}

main()