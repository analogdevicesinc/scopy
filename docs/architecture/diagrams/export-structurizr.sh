#!/bin/bash

# Structurizr Static Site Generator for GitHub Actions
# Generates interactive HTML diagrams from workspace.dsl files

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="${SCRIPT_DIR}/_build"
STRUCTURIZR_CLI_VERSION="2025.11.09"
STRUCTURIZR_CLI="${SCRIPT_DIR}/structurizr.sh"

echo "Building Structurizr static sites..."

# Create output directory
mkdir -p "${OUTPUT_DIR}"

# Download Structurizr CLI if not present
if [ ! -f "${STRUCTURIZR_CLI}" ]; then
    echo "Downloading Structurizr CLI v${STRUCTURIZR_CLI_VERSION}..."
    curl -fsSL "https://github.com/structurizr/cli/releases/download/v${STRUCTURIZR_CLI_VERSION}/structurizr-cli.zip" \
        -o "${SCRIPT_DIR}/structurizr-cli.zip"

    echo "Extracting Structurizr CLI..."
    cd "${SCRIPT_DIR}"
    unzip -q structurizr-cli.zip
    chmod +x structurizr.sh
    rm -f structurizr-cli.zip
    cd - > /dev/null
fi

# Function to export workspace
export_workspace() {
    local workspace_dir="$1"
    local workspace_name="$2"
    local workspace_title="$3"

    if [ ! -d "${workspace_dir}" ] || [ ! -f "${workspace_dir}/workspace.dsl" ]; then
        echo "Skipping ${workspace_title} - workspace not found"
        return 0
    fi

    echo "Exporting ${workspace_title}..."

    local output_path="${OUTPUT_DIR}/${workspace_name}"
    mkdir -p "${output_path}"

    (
        cd "${workspace_dir}"
        "${STRUCTURIZR_CLI}" export \
            -workspace workspace.json \
            -format static \
            -output "${output_path}"
    )

    echo "${workspace_title} exported to ${workspace_name}/"
}

# Export both workspaces
export_workspace "${SCRIPT_DIR}/app_diagrams" "app-architecture" "Application Architecture"
export_workspace "${SCRIPT_DIR}/deployment_diagrams" "deployment-architecture" "Deployment Architecture"

echo "Structurizr static sites generated in ${OUTPUT_DIR}"

# Copy diagrams to Sphinx build directory if it exists
SPHINX_HTML_DIR="${SCRIPT_DIR}/../../_build/html"
if [ -d "${SPHINX_HTML_DIR}" ]; then
    echo "Copying diagrams to Sphinx build directory..."
    mkdir -p "${SPHINX_HTML_DIR}/architecture/diagrams"
    cp -r "${OUTPUT_DIR}"/* "${SPHINX_HTML_DIR}/architecture/diagrams/"
    echo "Diagrams copied to ${SPHINX_HTML_DIR}/architecture/diagrams/"
fi

# GitHub Actions outputs
if [ "${GITHUB_ACTIONS:-false}" = "true" ]; then
    echo "structurizr_output_path=${OUTPUT_DIR}" >> "$GITHUB_OUTPUT"
fi