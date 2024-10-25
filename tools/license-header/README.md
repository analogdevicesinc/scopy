## License Header Management Scripts

These scripts help scan for missing headers, add headers to individual files, and apply headers in bulk. Below is an overview of the intended workflow and each script’s purpose.

### Workflow Overview

1. **Scan for Missing License Headers**: Identify files in the project directory that lack a license header using `scan_missing_headers.sh`.
2. **Add License Header Individually**: For individual files, use `add_license_header.sh`.
3. **Batch Add License Headers**: For bulk processing, use `batch_add_license_headers.sh` with a list of files that need headers.

### Script Descriptions and Usage

#### 1. `scan_missing_headers.sh`

- **Purpose**: This script scans a directory for files missing a license header.
- **Usage**: Run this script to generate a list of files without a license header. By default, it examines the first 80 lines of each file for license keywords. Filters can be applied to eliminate directories by their name or specific files. Results include a breakdown of licenses and a list of files missing headers.
- **Verbose mode** : 
    - When **enabled**: it highlights files with duplicate licenses or unhandled cases, making it easier for manual inspection and ensuring that all files are correctly processed. 
    - When **disabled**: the output can be fed other scrips for further processing.
- **Process**:
    - Execute the script by providing a path to the directory to scan.
    - Select files and directories to be filtered from the search.
    - The output displays files without a license header in a structured format.
- **Example**:
    ```bash
    ./scan_missing_headers.sh \
        --dirs .git*,build,.vscode,android,apple,ci,docs,tmp,tools,windows \
        --files .clang-format,.clangformatignore,.cmake-format,.gitignore,.gitmodules,*.md,LICENSE,*.png,*.rst,azure-pipelines.yml,requirements.txt,*.html,*.svg,*.icns,*.ico,*.qmodel,*.ui,*.json,*.qrc,*.ts,*.gif,*.theme,*.ttf,*.zip,*.csv,*.bin,*.xml,*.cmakein \
        --verbose > ./results/missing.txt
    ```

#### 2. `add_license_header.sh`
- **Purpose**: This script adds a license header to a single specified file.
- **Usage**: Use this script on a file-by-file basis when you need to customize the license template or parameters for each file.
- **Process**:
    - Specify the file to modify and optionally include paths to a license template and parameters file.
    - The script adapts the header format based on the file’s extension and inserts the header at the beginning.
- **Example**:
    ```bash
    ./add_license_header.sh /path/to/file \
        --template /path/to/template_file \
        --params /path/to/params.conf
    ```

#### 3. `batch_add_license_headers.sh` 
- **Purpose**: This script processes multiple files, adding a license header to each one based on a list.
- **Usage**: When many files require headers, use this script to add headers in bulk based on a specified file list.
- **Process**:
    - Provide a file containing a list of file paths, one per line.
    - The script applies the header to each listed file.
Example:
    ```bash
    ./batch_add_license_headers.sh /path/to/file \
        --template /path/to/template_file \
        --params /path/to/params.conf
    ```

**NOTE**: *For additional information about options, **refer to the usage output** of each script **(-h or --help)**.*
**NOTE**: After adding the license headers, make sure the run the `format.sh` utility to pass CI style checks.




