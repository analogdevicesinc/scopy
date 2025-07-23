#!/bin/bash

# GitHub Actions Scopy AppImage Downloader
# Downloads and launches Scopy x86_64 AppImage from GitHub Actions artifacts

# Show usage if help is requested (early exit)
if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    echo "Usage: $0 COMMIT_SHA [TOKEN]"
    echo ""
    echo "Scopy x86_64 AppImage Downloader"
    echo ""
    echo "This script downloads and launches Scopy x86_64 AppImage from GitHub Actions artifacts"
    echo "for the specified commit SHA."
    echo ""
    echo "Parameters:"
    echo "  COMMIT_SHA Commit SHA to search for (REQUIRED)"
    echo "  TOKEN      GitHub personal access token (default: hardcoded token)"
    echo ""
    echo "Example:"
    echo "  $0 ad5758d37c9e0021591013d3ca4a6e6529be839f"
    echo "  $0 ad5758d37c9e0021591013d3ca4a6e6529be839f ghp_your_token_here"
    echo ""
    echo "Dependencies: curl, jq, unzip"
    exit 0
fi

# Configuration constants (define early to avoid scoping issues)
COMMIT_SHA_LENGTH=40
MAX_RECENT_RUNS=3
MAX_WORKFLOW_RUNS=1
EXPECTED_APPIMAGE_NAME="Scopy-x86_64.AppImage"
GITHUB_TOKEN_LENGTH=40
DISK_SPACE_BUFFER_MB=50  # Extra space buffer for extraction and temporary files

# Repository details
OWNER="analogdevicesinc"
REPO="scopy"

# Workflow filter
WORKFLOW_NAME="Scopy x86_64 AppImage Build"

# API Base URL
API_URL="https://api.github.com"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
GRAY='\033[0;37m'
NC='\033[0m' # No Color

# Parameters
COMMIT_SHA="$1"
TOKEN="$2"

# Function to print colored output
print_color() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}


if [ -z "$TOKEN" ]; then
    TOKEN_FILE="github_token.txt"
    if [ -f "$TOKEN_FILE" ]; then
        TOKEN=$(head -n 1 "$TOKEN_FILE" | tr -d '\r\n[:space:]')
        print_color $CYAN "Token loaded from $TOKEN_FILE"
    else
        print_color $RED "Error: TOKEN not provided and $TOKEN_FILE not found."
        print_color $YELLOW "Please provide a token as a parameter or create $TOKEN_FILE with your token."
        exit 1
    fi
fi
# COMMIT_SHA="${2:-ad5758d37c9e0021591013d3ca4a6e6529be839f}"  # For testing only

# Validate required parameters immediately
if [ -z "$COMMIT_SHA" ]; then
    echo -e "\033[0;31mError: COMMIT_SHA is required!\033[0m"
    echo -e "\033[1;33mUsage: $0 COMMIT_SHA [TOKEN]\033[0m"
    echo -e "\033[1;33mRun '$0 --help' for more information.\033[0m"
    exit 1
fi

# Validate commit SHA format (40 character hex string)
if [[ ! "$COMMIT_SHA" =~ ^[a-fA-F0-9]{${COMMIT_SHA_LENGTH}}$ ]]; then
    echo -e "\033[0;31mError: Invalid commit SHA format. Must be ${COMMIT_SHA_LENGTH} character hex string.\033[0m"
    echo -e "\033[1;33mExample: ad5758d37c9e0021591013d3ca4a6e6529be839f\033[0m"
    exit 1
fi

# Validate GitHub token format (should start with ghp_, gho_, or ghs_ and be 36 or 40 chars after prefix)
if [[ ! "$TOKEN" =~ ^gh[pso]_[A-Za-z0-9]{36}$ && ! "$TOKEN" =~ ^gh[pso]_[A-Za-z0-9]{40}$ ]]; then
    echo -e "\033[0;31mError: Invalid GitHub token format.\033[0m"
    echo -e "\033[1;33mToken should start with 'ghp_', 'gho_', or 'ghs_' followed by 36 or 40 alphanumeric characters.\033[0m"
    echo -e "\033[1;33mExample: ghp_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\033[0m"
    exit 1
fi




# Function to handle common error patterns
handle_api_error() {
    local response=$1
    local context=$2
    
    # Check if response is valid JSON
    if ! echo "$response" | jq empty 2>/dev/null; then
        print_color $RED "Invalid JSON response from $context"
        print_color $YELLOW "Response (first 200 chars):"
        echo "$response" | head -c 200
        return 1
    fi
    
    # Check for GitHub API errors
    local error_message=$(echo "$response" | jq -r '.message // empty' 2>/dev/null)
    if [ -n "$error_message" ]; then
        print_color $RED "GitHub API error in $context: $error_message"
        local documentation_url=$(echo "$response" | jq -r '.documentation_url // empty')
        if [ -n "$documentation_url" ]; then
            print_color $YELLOW "Documentation: $documentation_url"
        fi
        return 1
    fi
    
    return 0
}

# Function to cleanup files on failure
cleanup_on_failure() {
    local file_path=$1
    if [ -f "$file_path" ]; then
        print_color $YELLOW "Cleaning up failed download: $file_path"
        rm -f "$file_path"
    fi
}

# Function to get workflow runs
get_workflow_runs() {
    local owner=$1
    local repo=$2
    local workflow_name=$3
    local commit_sha=$4
    
    print_color $CYAN "Making API call to: ${API_URL}/repos/${owner}/${repo}/actions/runs?head_sha=${commit_sha}"
    # Make API call filtered by commit SHA
    local response=$(curl -s -H "Authorization: token ${TOKEN}" \
                          -H "Accept: application/vnd.github.v3+json" \
                          -H "User-Agent: Scopy-AppImage-Downloader" \
                          "${API_URL}/repos/${owner}/${repo}/actions/runs?head_sha=${commit_sha}")
    
    if [ $? -ne 0 ]; then
        print_color $RED "API call failed!"
        return 1
    fi
    
    # Check if response is valid JSON and contains expected data
    if ! handle_api_error "$response" "workflow runs"; then
        return 1
    fi
    
    print_color $GREEN "API connection successful!"
    
    # Extract total count with null check
    local total_count=$(echo "$response" | jq -r '.total_count // "unknown"')
    if [ "$total_count" = "null" ] || [ "$total_count" = "unknown" ]; then
        print_color $RED "Invalid API response - no total_count field"
        print_color $YELLOW "Response structure:"
        echo "$response" | jq 'keys' 2>/dev/null || echo "Failed to parse response keys"
        return 1
    fi
    print_color $CYAN "Found ${total_count} workflow runs for commit '${commit_sha:0:7}'"

    # Check if workflow_runs array exists
    local workflow_runs_exists=$(echo "$response" | jq -r 'has("workflow_runs")')
    if [ "$workflow_runs_exists" != "true" ]; then
        print_color $RED "No workflow_runs array in response"
        return 1
    fi

    # Filter by workflow name
    local matching_workflows=$(echo "$response" | jq -c --arg name "$workflow_name" '.workflow_runs[] | select(.name == $name)')

    local matching_count=$(echo "$matching_workflows" | jq -s length 2>/dev/null || echo "0")
    if [ "$matching_count" -eq 0 ]; then
        print_color $YELLOW "No '$workflow_name' workflow found for commit ${commit_sha:0:7}"
        print_color $CYAN "Available workflows for this commit:"
        echo "$response" | jq -r '.workflow_runs[].name' | sort | uniq | while read -r name; do
            print_color $GRAY "  - $name"
        done
        return 1
    fi

    local run_id=$(echo "$matching_workflows" | jq -r '.id' | head -n ${MAX_WORKFLOW_RUNS})
    print_color $GREEN "Found matching run: $workflow_name (ID: $run_id)"

    # Get artifacts for the matching workflow run
    local run_ids_array=($(echo "$matching_workflows" | jq -r '.id' 2>/dev/null))

    if [ ${#run_ids_array[@]} -gt 0 ]; then
        echo ""
        for run_id in "${run_ids_array[@]}"; do
            if [ -n "$run_id" ] && [ "$run_id" != "null" ]; then
                get_workflow_artifacts "$run_id"
            fi
        done
    else
        print_color $YELLOW "No valid run IDs found for artifact fetching"
    fi

    return 0
}

# Function to download artifact with progress - separated logic
download_artifact() {
    local artifact_json=$1
    local artifact_name=$(echo "$artifact_json" | jq -r '.name')
    local download_url=$(echo "$artifact_json" | jq -r '.archive_download_url')
    local download_path="./${artifact_name}.zip"
    
    # Download with progress bar (all output to stderr to avoid mixing)
    curl -L -H "Authorization: token ${TOKEN}" \
         -H "Accept: application/vnd.github.v3+json" \
         -H "User-Agent: Scopy-AppImage-Downloader" \
         --progress-bar \
         -o "$download_path" \
         "$download_url" 2>&1
    
    if [ $? -eq 0 ]; then
        # Only echo the file path - no other output
        echo "$download_path"
        return 0
    else
        # Return error without echoing anything
        return 1
    fi
}

# Function to extract artifact - clean and efficient
extract_artifact() {
    local download_path=$1
    
    echo "Extracting: $download_path"

    # Basic validation
    if [ -z "$download_path" ]; then
        echo "Error: No download path provided" >&2
        return 1
    fi
    
    if [[ ! "$download_path" =~ \.zip$ ]]; then
        echo "Error: File must have .zip extension: $download_path" >&2
        return 1
    fi
    
    if [ ! -f "$download_path" ]; then
        echo "Error: File does not exist: $download_path" >&2
        return 1
    fi
    
    if ! command -v unzip >/dev/null 2>&1; then
        echo "Error: unzip command not found" >&2
        return 1
    fi
    
    # Test zip integrity
    if ! unzip -t "$download_path" >/dev/null 2>&1; then
        echo "Error: Invalid or corrupted zip file: $download_path" >&2
        return 1
    fi
    
    # Extract the zip file
    if unzip -o "$download_path" >/dev/null 2>&1; then
        # Make AppImage executable if it exists
        if [ -f "$EXPECTED_APPIMAGE_NAME" ]; then
            chmod +x "$EXPECTED_APPIMAGE_NAME"
        fi
        
        # Remove the zip file after successful extraction
        rm -f "$download_path"
        
        echo "extracted"
        return 0
    else
        echo "Error: Failed to extract $download_path" >&2
        return 1
    fi
}

# Function to launch Scopy AppImage
launch_scopy() {
    local appimage_path="$EXPECTED_APPIMAGE_NAME"
    
    if [ ! -f "$appimage_path" ]; then
        print_color $RED "Error: Scopy AppImage not found: $appimage_path"
        return 1
    fi
    
    # Make sure it's executable
    chmod +x "$appimage_path"
    
    print_color $GREEN "Launching Scopy AppImage..."
    print_color $CYAN "Location: ./$appimage_path"
    
    # Launch Scopy in background
    if command -v nohup >/dev/null 2>&1; then
        nohup "./$appimage_path" >/dev/null 2>&1 &
    else
        "./$appimage_path" &
    fi
    
    local launch_result=$?
    if [ $launch_result -eq 0 ]; then
        print_color $GREEN "Scopy launched successfully!"
        print_color $CYAN "Process ID: $!"
        return 0
    else
        print_color $RED "Failed to launch Scopy (exit code: $launch_result)"
        return 1
    fi
}

# Function to process a single artifact (download, extract, launch)
process_single_artifact() {
    local artifact=$1
    local artifact_name=$(echo "$artifact" | jq -r '.name')
    local expired=$(echo "$artifact" | jq -r '.expired')
    local size_bytes=$(echo "$artifact" | jq -r '.size_in_bytes')
    
    if [ "$expired" = "true" ]; then
        print_color $YELLOW "Skipping expired artifact: $artifact_name"
        return 1
    fi
    

    # Get artifact creation date and build folder name MM_dd_yyyy_SHA
    local created_at=$(echo "$artifact" | jq -r '.created_at')
    local date_folder=$(date -d "${created_at}" +"%m_%d_%Y_%H_%M_%S" 2>/dev/null)
    if [ -z "$date_folder" ]; then
        date_folder=$(date +"%m_%d_%Y_%H_%M_%S")
    fi
    local folder_name="${date_folder}_${COMMIT_SHA}"
    mkdir -p "$folder_name"
    
    local zip_path="$folder_name/${artifact_name}.zip"
    echo "Downloading to: $zip_path"
    local download_url=$(echo "$artifact" | jq -r '.archive_download_url')
    if ! curl -L -H "Authorization: token ${TOKEN}" \
        -H "Accept: application/vnd.github.v3+json" \
        -H "User-Agent: Scopy-AppImage-Downloader" \
        --progress-bar \
        -o "$zip_path" \
        "$download_url"; then
        print_color $RED "Failed to download: $artifact_name"
        return 1
    fi

    # Extract the downloaded artifact into the folder
    echo "Extracting to: $folder_name/${artifact_name}"
    mkdir -p "$folder_name/${artifact_name}"
    if unzip -o "$zip_path" -d "$folder_name/${artifact_name}" >/dev/null 2>&1; then
        # Make AppImage executable if it exists in the extracted folder
        local appimage_path="$folder_name/${artifact_name}/$EXPECTED_APPIMAGE_NAME"
        if [ -f "$appimage_path" ]; then
            chmod +x "$appimage_path"
            local appimage_size=$(stat -c%s "$appimage_path" 2>/dev/null || echo "unknown")
            echo "Found AppImage: $EXPECTED_APPIMAGE_NAME (Size: $appimage_size bytes)"
            print_color $CYAN "Launching Scopy application..."
            nohup "$appimage_path" >/dev/null 2>&1 &
            print_color $GREEN "Scopy application launched successfully!"
        else
            print_color $YELLOW "AppImage not found in extracted files."
        fi
        rm -f "$zip_path"
        echo "extracted"
        extract_status=0
    else
        echo "Error: Failed to extract $zip_path" >&2
        extract_status=1
    fi
    echo "Extracted successfully!"
    local other_files=$(find "$folder_name/${artifact_name}" -maxdepth 1 -type f ! -name "$EXPECTED_APPIMAGE_NAME" 2>/dev/null | wc -l)
    if [ "$other_files" -gt 0 ]; then
        echo "Found $other_files additional file(s) extracted"
    fi
}
get_workflow_artifacts() {
    local run_id=$1
    
    if [ -z "$run_id" ]; then
        print_color $RED "Error: No run ID provided to get_workflow_artifacts"
        return 1
    fi
    
    print_color $CYAN "Processing run: $run_id (Commit: ${COMMIT_SHA:0:7})"
    
    # Make API call to get artifacts
    local artifacts_response=$(curl -s -H "Authorization: token ${TOKEN}" \
        -H "Accept: application/vnd.github.v3+json" \
        -H "User-Agent: Scopy-AppImage-Downloader" \
        "https://api.github.com/repos/${OWNER}/${REPO}/actions/runs/${run_id}/artifacts")
    
    if [ $? -ne 0 ]; then
        print_color $RED "Failed to fetch artifacts for run ID: $run_id"
        return 1
    fi
    
    # Handle API response errors
    if ! handle_api_error "$artifacts_response" "artifacts"; then
        return 1
    fi
    
    # Get total count and artifacts
    local total_count=$(echo "$artifacts_response" | jq -r '.total_count // 0' 2>/dev/null)
    
    if [ "$total_count" = "null" ] || [ "$total_count" = "0" ]; then
        print_color $YELLOW "  No artifacts found for this workflow run"
        return 0
    fi
    
    print_color $GREEN "Found $total_count artifact(s):"
    
    # Display artifacts in the format matching the example
    echo "$artifacts_response" | jq -r '.artifacts[] | 
        "Name: \(.name)\nSize: \(.size_in_bytes | . / 1024 / 1024 | floor * 100 / 100) MB\nCreated: \(.created_at)\nDownload URL: \(.archive_download_url)"' 2>/dev/null
    
    if [ $? -ne 0 ]; then
        print_color $RED "Failed to format artifact list"
        return 1
    fi
    
    # Download and process all artifacts
    local artifacts_array=$(echo "$artifacts_response" | jq -c '.artifacts[]' 2>/dev/null)
    
    if [ -n "$artifacts_array" ]; then
        local download_count=0
        while IFS= read -r artifact; do
            if [ -n "$artifact" ]; then
                if process_single_artifact "$artifact"; then
                    ((download_count++))
                fi
            fi
        done <<< "$artifacts_array"
        
        if [ "$download_count" -gt 0 ]; then
            print_color $GREEN "Successfully processed $download_count artifact(s)"
        else
            print_color $YELLOW "No artifacts were successfully processed"
        fi
    else
        print_color $RED "Failed to parse artifacts for download"
    fi
    
    return 0
}

# Check dependencies - now need curl, jq, and unzip for downloading/extracting
check_dependencies() {
    local missing_deps=()
    for dep in curl jq unzip; do
        if ! command -v "$dep" >/dev/null 2>&1; then
            missing_deps+=("$dep")
        fi
    done
    if [ ${#missing_deps[@]} -gt 0 ]; then
        print_color $RED "Missing dependencies: ${missing_deps[*]}"
        print_color $YELLOW "Please install the missing dependencies and try again."
        exit 1
    fi
    print_color $GREEN "All dependencies are installed."
}

# Main execution block
main() {
    check_dependencies
    get_workflow_runs "$OWNER" "$REPO" "$WORKFLOW_NAME" "$COMMIT_SHA"
}

# Only run main if not sourced (i.e., if executed directly)
if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
    main
fi



