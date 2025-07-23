#!/bin/bash

# Azure Pipelines API Test Script for macOS
# This script downloads and launches Scopy from Azure DevOps builds

# Help function
show_help() {
    echo "Usage: $0 COMMIT_SHA [TOKEN]"
    echo ""
    echo "Parameters:"
    echo "  COMMIT_SHA - Git commit SHA to search for (REQUIRED)"
    echo "  TOKEN   - GitHub personal access token (can be put in a file called github_token.txt)"
    echo ""
    echo "Default values:"
    echo "  Organization: AnalogDevices"
    echo "  Project: M2k"
    echo "  Pipeline: analogdevicesinc.scopy"
    echo ""
    echo "Example:"
    echo "  $0 ad5758d37c9e0021591013d3ca4a6e6529be839f"
    echo "  $0 ad5758d37c9e0021591013d3ca4a6e6529be839f your_token_here"
}

# Check for help flag
if [[ "$1" == "--help" || "$1" == "-h" ]]; then
    show_help
    exit 0
fi

# Color constants (must be declared before any function uses them)
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
GRAY='\033[0;37m'
NC='\033[0m' # No Color

# Configuration constants
COMMIT_SHA_LENGTH=40

# Parameters
COMMIT_SHA="$1"
TOKEN="$2"
if [ -z "$TOKEN" ]; then
    TOKEN_FILE="github_token.txt"
    if [ -f "$TOKEN_FILE" ]; then
        TOKEN=$(head -n 1 "$TOKEN_FILE")
        print_color $CYAN "Token loaded from $TOKEN_FILE"
    else
        print_color $RED "Error: TOKEN not provided and $TOKEN_FILE not found."
        print_color $YELLOW "Please provide a token as a parameter or create $TOKEN_FILE with your token."
        exit 1
    fi
fi

# Validate required parameters
if [ -z "$COMMIT_SHA" ]; then
    echo -e "${RED}Error: COMMIT_SHA is required!${NC}"
    echo -e "${YELLOW}Usage: $0 COMMIT_SHA [TOKEN]${NC}"
    echo -e "${YELLOW}Run '$0 --help' for more information.${NC}"
    exit 1
fi

# Validate commit SHA format (40 character hex string)
if [[ ! "$COMMIT_SHA" =~ ^[a-fA-F0-9]{${COMMIT_SHA_LENGTH}}$ ]]; then
    echo -e "${RED}Error: Invalid commit SHA format. Must be ${COMMIT_SHA_LENGTH} character hex string.${NC}"
    echo -e "${YELLOW}Example: ad5758d37c9e0021591013d3ca4a6e6529be839f${NC}"
    exit 1
fi

# Validate token (basic check - not empty)
if [ -z "$TOKEN" ]; then
    echo -e "${RED}Error: TOKEN cannot be empty.${NC}"
    echo -e "${YELLOW}Provide a valid Azure DevOps Personal Access Token.${NC}"
    exit 1
fi

# Azure DevOps configuration
ORGANIZATION="AnalogDevices"
PROJECT="M2k"
PIPELINE_NAME="analogdevicesinc.scopy"

# API configuration
API_URL="https://dev.azure.com/${ORGANIZATION}/${PROJECT}/_apis"
API_VERSION="7.1"

# Function to print colored output
print_color() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

# Function to check dependencies
check_dependencies() {
    local missing_deps=()
    
    for dep in curl jq unzip base64; do
        if ! command -v "$dep" &> /dev/null; then
            missing_deps+=("$dep")
        fi
    done
    
    if [ ${#missing_deps[@]} -gt 0 ]; then
        print_color $RED "Missing dependencies: ${missing_deps[*]}"
        print_color $YELLOW "Install with: brew install ${missing_deps[*]}"
        return 1
    fi
}

# Function to make API calls with Azure DevOps authentication
make_api_call() {
    local url=$1
    local auth_header=$(echo -n ":$TOKEN" | base64)
    local response=$(curl -s -H "Authorization: Basic $auth_header" \
                          -H "Accept: application/json" \
                          "$url")
    
    if [ $? -ne 0 ]; then
        print_color $RED "API call failed!"
        return 1
    fi
    
    echo "$response"
}

# Function to get current macOS version
get_macos_version() {
    local version=$(sw_vers -productVersion)
    local major_version=$(echo "$version" | cut -d'.' -f1)
    
    if [ "$major_version" -ge 13 ]; then
        echo "$major_version"
    else
        echo "unknown"
    fi
}

# Function to get matching build and download artifact
get_matching_build() {
    local url="${API_URL}/build/builds?api-version=${API_VERSION}&sourceVersion=${COMMIT_SHA}"
    print_color $YELLOW "Getting builds for commit: $COMMIT_SHA"
    
    local response=$(make_api_call "$url")
    local total_count=$(echo "$response" | jq -r '.count')
    
    print_color $CYAN "Found $total_count builds for this commit"
    
    # Filter by pipeline name AND exact commit SHA
    local filtered_builds=$(echo "$response" | jq -r --arg pipeline "$PIPELINE_NAME" --arg sha "$COMMIT_SHA" '.value[] | select(.definition.name == $pipeline and .sourceVersion == $sha)')
    
    if [ -n "$filtered_builds" ]; then
        local build_id=$(echo "$filtered_builds" | jq -r '.id' | head -1)
        local build_status=$(echo "$filtered_builds" | jq -r '.status' | head -1)
        local build_result=$(echo "$filtered_builds" | jq -r '.result' | head -1)
        
        print_color $GREEN "Found matching build: $build_id (Status: $build_status, Result: $build_result)"
        
        download_artifacts "$build_id"
        return 0
    else
        print_color $RED "No matching build found for '$PIPELINE_NAME' with commit $COMMIT_SHA"
        return 1
    fi
}


# Helper function to find matching artifact for current macOS version
find_matching_artifact() {
    local response="$1"
    local current_macos=$(get_macos_version)
    local matching_artifact=""
    if [ "$current_macos" != "unknown" ]; then
        matching_artifact=$(echo "$response" | jq -c --arg version "$current_macos" '.value[] | select(.name | contains("macOS_" + $version))' | head -1)
        if [ -z "$matching_artifact" ]; then
            matching_artifact=$(echo "$response" | jq -c --arg version "$current_macos" '.value[] | select(.name | contains("macOS" + $version))' | head -1)
        fi
        if [ -z "$matching_artifact" ]; then
            matching_artifact=$(echo "$response" | jq -c --arg version "$current_macos" '.value[] | select(.name | contains($version))' | head -1)
        fi
    fi
    # Use first artifact if no match found
    if [ -z "$matching_artifact" ] || [ "$matching_artifact" = "null" ]; then
        print_color $YELLOW "No artifact found for macOS $current_macos, using first available"
        matching_artifact=$(echo "$response" | jq -c '.value[0]')
    fi
    echo "$matching_artifact"
}

# Function to download artifacts
download_artifacts() {
    local build_id=$1
    local url="${API_URL}/build/builds/${build_id}/artifacts?api-version=${API_VERSION}"
    local response=$(make_api_call "$url")
    local artifact_count=$(echo "$response" | jq -r '.count')
    if [ "$artifact_count" -eq 0 ]; then
        print_color $YELLOW "No artifacts found for build $build_id"
        return 1
    fi
    print_color $GREEN "Found $artifact_count artifacts"
    local matching_artifact=$(find_matching_artifact "$response")
    # Validate matching_artifact is not empty/null and is valid JSON
    if [ -z "$matching_artifact" ] || [ "$matching_artifact" = "null" ]; then
        print_color $RED "Error: No valid artifact found to download."
        return 1
    fi
    # Check if matching_artifact is valid JSON (jq will fail if not)
    echo "$matching_artifact" | jq empty 2>/dev/null
    if [ $? -ne 0 ]; then
        print_color $RED "Error: matching_artifact is not valid JSON. Value: $matching_artifact"
        return 1
    fi
    local artifact_name=$(echo "$matching_artifact" | jq -r '.name')
    local download_url=$(echo "$matching_artifact" | jq -r '.resource.downloadUrl')
    if [ -z "$artifact_name" ] || [ -z "$download_url" ] || [ "$artifact_name" = "null" ] || [ "$download_url" = "null" ]; then
        print_color $RED "Error: Artifact name or download URL is missing."
        print_color $RED "artifact_name: $artifact_name"
        print_color $RED "download_url: $download_url"
        return 1
    fi
    # Get build finish date for folder name
    local build_finish_time=$(echo "$matching_artifact" | jq -r '.resource.data.finishTime')
    if [ -z "$build_finish_time" ] || [ "$build_finish_time" = "null" ]; then
        build_finish_time=$(date +"%Y-%m-%dT%H:%M:%S")
    fi
    local date_folder=$(date -j -f "%Y-%m-%dT%H:%M:%S" "${build_finish_time:0:19}" +"%m_%d_%Y_%H_%M_%S" 2>/dev/null)
    if [ -z "$date_folder" ]; then
        date_folder=$(date +"%m_%d_%Y_%H_%M_%S")
    fi
    local folder_name="${date_folder}_${COMMIT_SHA}"
    mkdir -p "$folder_name"
    print_color $GREEN "Downloading artifact: $artifact_name"
    local filename="$folder_name/${artifact_name}.zip"
    local auth_header=$(echo -n ":$TOKEN" | base64)
    curl -L -H "Authorization: Basic $auth_header" \
         -H "Accept: application/zip" \
         --progress-bar \
         -o "$filename" \
         "$download_url"
    if [ $? -eq 0 ]; then
        print_color $GREEN "Downloaded successfully!"
        extract_and_launch "$filename" "$artifact_name" "$folder_name"
    else
        print_color $RED "Download failed!"
    fi
}

# Function to extract and launch Scopy
extract_and_launch() {
    local zip_file=$1
    local artifact_name=$2
    local folder_name=$3
    local extract_dir="$folder_name/${artifact_name}"

    print_color $YELLOW "Extracting to: $extract_dir"
    # Debug: print file size and type
    if command -v stat >/dev/null 2>&1; then
        print_color $CYAN "Zip file size: $(stat -c %s "$zip_file" 2>/dev/null || stat -f %z "$zip_file") bytes"
    fi
    if command -v file >/dev/null 2>&1; then
        print_color $CYAN "Zip file type: $(file "$zip_file")"
    fi
    print_color $CYAN "Listing zip contents:"
    unzip -l "$zip_file"
    # Extract
    mkdir -p "$extract_dir"
    if ! unzip -q "$zip_file" -d "$extract_dir"; then
        print_color $RED "Extraction failed!"
        print_color $YELLOW "First 20 lines of the zip file (may show error if not a real zip):"
        head -20 "$zip_file"
        return 1
    fi
    # Handle double-zipped files
    local inner_zips=$(find "$extract_dir" -name "*.zip" -type f)
    if [ -n "$inner_zips" ]; then
        print_color $YELLOW "Extracting inner zip files..."
        echo "$inner_zips" | while read -r inner_zip; do
            local inner_name=$(basename "$inner_zip" .zip)
            local inner_extract_dir="$extract_dir/$inner_name"
            mkdir -p "$inner_extract_dir"
            unzip -q "$inner_zip" -d "$inner_extract_dir" && rm -f "$inner_zip"
        done
    fi
    # Clean up zip file
    rm -f "$zip_file"
    print_color $GREEN "Extraction completed"
    # Launch Scopy
    launch_scopy "$extract_dir"
}

# Function to launch Scopy
launch_scopy() {
    local extract_path=$1
    
    print_color $YELLOW "Searching for Scopy executable..."
    
    # Look for ScopyApp directory first, then fallback to entire path
    local scopy_app_dir=$(find "$extract_path" -name "ScopyApp" -type d | head -1)
    local search_path="${scopy_app_dir:-$extract_path}"
    local scopy_exe=$(find "$search_path" -name "scopy" -o -name "Scopy" -o -name "scopy.app" -o -name "Scopy.app" -type f | head -1)
    if [ -n "$scopy_exe" ]; then
        print_color $GREEN "Found Scopy executable: $scopy_exe"
        # Make executable and remove quarantine
        chmod +x "$scopy_exe"
        xattr -d com.apple.quarantine "$scopy_exe" 2>/dev/null || true
        if [[ "$scopy_exe" == *.app ]]; then
            xattr -rd com.apple.quarantine "$scopy_exe" 2>/dev/null || true
            open "$scopy_exe"
        else
            nohup "$scopy_exe" > /dev/null 2>&1 &
        fi
        print_color $GREEN "Scopy launched successfully!"
    else
        print_color $YELLOW "Scopy executable not found in extracted files"
    fi
}

# Main execution
main() {
    check_dependencies
    
    print_color $BLUE "Azure DevOps Scopy Launcher"
    print_color $GREEN "Organization: $ORGANIZATION | Project: $PROJECT"
    
    if get_matching_build; then
        print_color $GREEN "Process completed successfully!"
    else
        print_color $RED "Failed to find or process build"
    fi
}

# Run main function
main