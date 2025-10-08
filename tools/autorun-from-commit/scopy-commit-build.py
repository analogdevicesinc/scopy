#!/usr/bin/env python3
import argparse
import sys
import os
import requests
import platform

def autodetect_platform():
    sys_platform = platform.system().lower()
    if 'windows' in sys_platform:
        return 'windows'
    elif 'linux' in sys_platform:
        return 'x86'
    else:
        return None

def validate_platform_compatibility(target_platform, current_platform):
    """
    Check if the target platform executable can run on the current platform.
    Returns True if compatible, False otherwise.
    """
    if not current_platform:
        print("Warning: Could not detect current platform, skipping compatibility check")
        return True

    # Same platform is always compatible
    if target_platform == current_platform:
        return True

    # Cross-platform compatibility rules
    compatibility_matrix = {
        'windows': ['windows'],
        'x86': ['x86'],  # Linux x86 only runs x86 executables
        'arm64': ['arm64'],
        'arm32': ['arm32']
    }

    compatible_platforms = compatibility_matrix.get(current_platform, [])
    return target_platform in compatible_platforms

def read_token_from_file(token_file="github_token.txt"):
    if os.path.isfile(token_file):
        with open(token_file, "r") as f:
            return f.readline().strip()
    return None

def read_token_from_git_credentials():
    """Read GitHub token from ~/.git-credentials file."""
    credentials_path = os.path.expanduser("~/.git-credentials")
    if not os.path.isfile(credentials_path):
        return None

    try:
        with open(credentials_path, "r") as f:
            for line in f:
                line = line.strip()
                # Look for GitHub credentials: https://username:token@github.com
                if "github.com" in line and "://" in line:
                    # Parse URL to extract token
                    if "@github.com" in line:
                        # Extract the part before @github.com
                        auth_part = line.split("@github.com")[0]
                        if "://" in auth_part:
                            # Extract credentials part after protocol
                            credentials = auth_part.split("://", 1)[1]
                            if ":" in credentials:
                                # Split username:token
                                username, token = credentials.split(":", 1)
                                # GitHub tokens typically start with ghp_, gho_, ghu_, etc.
                                if token and (token.startswith("ghp_") or token.startswith("gho_") or token.startswith("ghu_") or token.startswith("github_pat_")):
                                    return token
                            else:
                                # Handle case where only token is present (no username)
                                token = credentials
                                if token and (token.startswith("ghp_") or token.startswith("gho_") or token.startswith("ghu_") or token.startswith("github_pat_")):
                                    return token
    except Exception as e:
        print(f"Warning: Error reading ~/.git-credentials: {e}")

    return None

def parse_arguments():
    parser = argparse.ArgumentParser(
        description="Scopy Build Downloader: Downloads and launches Scopy build from GitHub Actions artifacts for a given commit SHA."
    )
    parser.add_argument("commit_sha", type=str, help="Commit SHA to search for (required)")
    parser.add_argument("--token", type=str, required=False, help="GitHub personal access token (optional, fallback to github_token.txt or ~/.git-credentials)")
    parser.add_argument("--platform", type=str, choices=["windows", "x86", "arm64", "arm32"], required=False, help="Target platform (windows, x86, arm64, arm32)")
    parser.add_argument("--script", type=str, required=False, help="Optional path to a script to use or run after extraction")
    return parser.parse_args()

def validate_commit_sha(commit_sha):
    # Accept full SHA (40 chars) or short SHA (>=7 chars)
    return commit_sha and (7 <= len(commit_sha) <= 40) and all(c in "0123456789abcdefABCDEF" for c in commit_sha)

def get_token(token_arg):
    if token_arg:
        print("Using token from command line argument")
        return token_arg

    file_token = read_token_from_file()
    if file_token:
        print("Using token from github_token.txt")
        return file_token

    git_creds_token = read_token_from_git_credentials()
    if git_creds_token:
        print("Using token from ~/.git-credentials")
        return git_creds_token

    return None

def get_workflow_name(platform):
    if platform == "windows":
        return "windows-mingw build"
    elif platform == "x86":
        return "Scopy x86_64 AppImage Build"
    elif platform == "arm64":
        return "Scopy arm64 AppImage Build"
    elif platform == "arm32":
        return "Scopy armhf AppImage Build"
    else:
        return None

def get_github_api_headers(token):
    return {
        "Authorization": f"token {token}",
        "Accept": "application/vnd.github.v3+json",
        "User-Agent": "Python-Scopy-Downloader"
    }

def get_workflow_run(owner, repo, commit_sha, workflow_name, headers):
    api_url = f"https://api.github.com/repos/{owner}/{repo}/actions/runs?head_sha={commit_sha}"
    print(f"Connecting to GitHub API: {api_url}")
    try:
        response = requests.get(api_url, headers=headers)
        response.raise_for_status()
        data = response.json()
    except Exception as e:
        print(f"Error connecting to GitHub API: {e}")
        sys.exit(1)
    total_count = data.get("total_count", 0)
    runs = [run for run in data.get("workflow_runs", []) if run.get("name") == workflow_name and run.get("head_sha", "").startswith(commit_sha)]
    # If no runs and short SHA, retry without head_sha filter and search for partial matches
    if not runs and len(commit_sha) < 40:
        print(f"No workflow runs found for commit {commit_sha}, searching through historical runs...")

        # Try multiple pages to find older commits
        all_runs = []
        page = 1
        max_pages = 5  # Limit to 5 pages (150 runs) to avoid excessive API calls

        while page <= max_pages:
            api_url_all = f"https://api.github.com/repos/{owner}/{repo}/actions/runs?page={page}&per_page=30"
            try:
                response_all = requests.get(api_url_all, headers=headers)
                response_all.raise_for_status()
                data_all = response_all.json()
                runs_batch = data_all.get("workflow_runs", [])

                if not runs_batch:  # No more runs available
                    break

                all_runs.extend(runs_batch)
                print(f"Fetched page {page}: {len(runs_batch)} runs")
                page += 1

            except Exception as e:
                print(f"Error connecting to GitHub API (page {page}): {e}")
                break

        # Create data_all structure for compatibility with existing code
        data_all = {"workflow_runs": all_runs}
        # Debug: show SHAs being checked for partial matching
        target_workflows = [run for run in data_all.get("workflow_runs", []) if run.get("name") == workflow_name]
        print(f"Found {len(target_workflows)} '{workflow_name}' workflows in {len(all_runs)} total runs:")
        for i, run in enumerate(target_workflows[:10]):  # Show first 10
            head_sha = run.get("head_sha", "N/A")
            print(f"  {i+1}. {head_sha} | Status: {run.get('status')}")

        partial_matches = [run for run in target_workflows if commit_sha in run.get("head_sha", "")]
        if partial_matches:
            print(f"Found {len(partial_matches)} workflow runs partially matching SHA '{commit_sha}':")
            for idx, run in enumerate(partial_matches):
                print(f"  [{idx+1}] SHA: {run.get('head_sha')} | Status: {run.get('status')} | ID: {run.get('id')}")
            # Always use the first match
            return partial_matches[0]
        else:
            print(f"")
            print(f"WARNING: No '{workflow_name}' workflow found for commit {commit_sha} in {len(all_runs)} workflow runs checked.")
            print(f" Try using a full commit SHA or check if the commit has a '{workflow_name}' workflow.")
            sys.exit(1)
    elif not runs:
        print(f"No '{workflow_name}' workflow found for exact commit {commit_sha}")
        print(f"Try using a shorter SHA for partial matching or check if the commit has a '{workflow_name}' workflow.")
        sys.exit(1)
    return runs[0]

def get_artifacts(owner, repo, run_id, headers):
    artifacts_url = f"https://api.github.com/repos/{owner}/{repo}/actions/runs/{run_id}/artifacts"
    print(f"Fetching artifacts: {artifacts_url}")
    try:
        artifacts_response = requests.get(artifacts_url, headers=headers)
        artifacts_response.raise_for_status()
        artifacts_data = artifacts_response.json()
    except Exception as e:
        print(f"Error fetching artifacts: {e}")
        sys.exit(1)
    artifacts = artifacts_data.get("artifacts", [])
    if not artifacts:
        print("No artifacts found for this workflow run.")
        sys.exit(1)
    return artifacts

def filter_artifacts(artifacts, workflow_name, platform_value=None):
    # Windows: artifact name starts with 'scopy-windows-x86_64' and does not contain 'setup'
    if workflow_name == "windows-mingw build":
        return [a for a in artifacts if a["name"].startswith("scopy-windows-x86_64") and "setup" not in a["name"]]
    # Linux: artifact name starts with 'scopy-linux-x86_64-'
    elif workflow_name == "Scopy x86_64 AppImage Build":
        return [a for a in artifacts if a["name"].startswith("scopy-linux-x86_64-")]
    # arm64: artifact name starts with 'scopy-arm64'
    elif workflow_name == "Scopy arm64 AppImage Build":
        return [a for a in artifacts if a["name"].startswith("scopy-arm64")]
    # arm32/armhf: artifact name starts with 'scopy-linux-armhf'
    elif workflow_name == "Scopy armhf AppImage Build":
        return [a for a in artifacts if a["name"].startswith("scopy-linux-armhf")]
    else:
        return []

def download_artifact(artifact, commit_sha, headers):
    import datetime
    artifact_name = artifact['name']
    dt = datetime.datetime.now()
    date_folder = dt.strftime('%m_%d_%Y_%H-%M')
    folder_name = f"{date_folder}_{commit_sha}"
    if not os.path.exists(folder_name):
        os.makedirs(folder_name)
    zip_path = os.path.join(folder_name, f"{artifact_name}.zip")
    download_url = artifact['archive_download_url']
    print(f"Downloading artifact to: {zip_path}")
    try:
        with requests.get(download_url, headers=headers, stream=True) as r:
            r.raise_for_status()
            with open(zip_path, 'wb') as f:
                for chunk in r.iter_content(chunk_size=8192):
                    if chunk:
                        f.write(chunk)
        print(f"Download complete: {zip_path}")
        return zip_path, folder_name
    except Exception as e:
        print(f"Error downloading artifact: {e}")
        return None, folder_name

def extract_zip(zip_path, extract_to):
    import zipfile
    print(f"Extracting {zip_path} to {extract_to}")
    try:
        with zipfile.ZipFile(zip_path, 'r') as zip_ref:
            zip_ref.extractall(extract_to)
        print(f"Extraction complete: {extract_to}")
        os.remove(zip_path)
    except Exception as e:
        print(f"Error extracting zip: {e}")
        return False
    # Handle nested ZIPs: extract all nested zips into the main output folder
    for root, dirs, files in os.walk(extract_to):
        for file in files:
            if file.lower().endswith('.zip'):
                nested_zip_path = os.path.join(root, file)
                print(f"Found nested zip: {nested_zip_path}, extracting to {extract_to}")
                try:
                    with zipfile.ZipFile(nested_zip_path, 'r') as zip_ref:
                        zip_ref.extractall(extract_to)
                    os.remove(nested_zip_path)
                    print(f"Nested extraction complete: {extract_to}")
                except Exception as e:
                    print(f"Error extracting nested zip {nested_zip_path}: {e}")
    return True

def find_executable(folder, exe_name):
    if not exe_name:
        return None
    for root, dirs, files in os.walk(folder):
        for file in files:
            if file == exe_name:
                return os.path.join(root, file)
    return None

def main():
    args = parse_arguments()

    if not validate_commit_sha(args.commit_sha):
        print("Error: commit_sha must be a hex string (at least 7 characters, up to 40).")
        print("Example: ad5758d37c9e0021591013d3ca4a6e6529be839f or ad6bd6b")
        sys.exit(1)

    token = get_token(args.token)
    if not token:
        print("Error: GitHub token not provided and no token found in fallback sources.")
        print("Please provide a token with --token, create github_token.txt, or configure ~/.git-credentials.")
        sys.exit(1)

    print(f"Commit SHA: {args.commit_sha}")

    # Auto-detect platform if not provided
    current_platform = autodetect_platform()
    platform_value = args.platform if args.platform else current_platform
    print(f"Platform: {platform_value}")

    # Validate platform compatibility for execution
    if args.platform and not validate_platform_compatibility(args.platform, current_platform):
        print(f"Error: Cannot run {args.platform} executable on {current_platform} platform.")
        print("The executable will be downloaded but not launched.")
        # Set a flag to skip execution later
        skip_execution = True
    else:
        skip_execution = False

    owner = "analogdevicesinc"
    repo = "scopy"
    workflow_name = get_workflow_name(platform_value)
    if not workflow_name:
        print("Error: Only Windows, x86, ARM64, and ARM32 platforms are supported in this script version.")
        sys.exit(1)

    headers = get_github_api_headers(token)
    run = get_workflow_run(owner, repo, args.commit_sha, workflow_name, headers)
    run_id = run["id"]
    print(f"Found matching workflow run: {workflow_name} (ID: {run_id})")

    artifacts = get_artifacts(owner, repo, run_id, headers)
    filtered_artifacts = filter_artifacts(artifacts, workflow_name, platform_value)
    if not filtered_artifacts:
        print("No matching artifacts found.")
        print("Available artifacts:")
        for a in artifacts:
            print(f"  - {a['name']}")
        sys.exit(1)

    print(f"Found {len(filtered_artifacts)} matching artifact(s):")
    for a in filtered_artifacts:
        print(f"  - {a['name']} | Size: {round(a['size_in_bytes']/1_048_576,2)} MB | Created: {a['created_at']}\n    Download URL: {a['archive_download_url']}")

    # Download and extract the first matching artifact
    zip_path, folder_name = download_artifact(filtered_artifacts[0], args.commit_sha, headers)
    if not zip_path:
        print("Artifact download failed.")
        sys.exit(1)
    if not extract_zip(zip_path, folder_name):
        print("Artifact extraction failed.")
        sys.exit(1)

    # Determine exe_name based on platform
    if platform_value == "windows":
        exe_name = "Scopy-console.exe"
    elif platform_value == "x86":
        exe_name = "Scopy-x86_64.AppImage"
    elif platform_value == "arm64":
        exe_name = "Scopy-arm64.AppImage"
    elif platform_value == "arm32":
        exe_name = "Scopy-armhf.AppImage"
    else:
        exe_name = None

    exe_path = find_executable(folder_name, exe_name)
    if not exe_path:
        print(f"Executable not found for platform '{platform_value}'. Searched for: {exe_name}")
        sys.exit(1)
    print(f"Found executable: {exe_path}")

    # Check if we should skip execution due to platform incompatibility
    if skip_execution:
        print(f"Skipping execution of {platform_value} executable on {current_platform} platform.")
        print(f"Executable downloaded to: {exe_path}")
        return

    # Launch the executable, optionally with a script
    import subprocess
    import stat
    try:
        launch_args = [exe_path]
        script_path = None
        if args.script:
            script_path = args.script
            print(f"Script path: {script_path}")
        # Launch Scopy process
        if platform_value == "windows":
            proc = subprocess.Popen(launch_args, shell=True, stdin=subprocess.PIPE, stdout=None, stderr=None, text=True)
        elif platform_value == "x86" or platform_value == "arm64" or platform_value == "arm32":
            st = os.stat(exe_path)
            if not (st.st_mode & stat.S_IXUSR):
                os.chmod(exe_path, st.st_mode | stat.S_IXUSR)
            proc = subprocess.Popen(launch_args, stdin=subprocess.PIPE, stdout=None, stderr=None, text=True)
        else:
            print(f"Platform '{platform_value}' is not supported for launching.")
            return
        # If script is provided, send the command to Scopy's console
        if args.script and proc.stdin:
            import time
            time.sleep(200)  # Wait a bit for Scopy to initialize (adjust as needed)
            cmd = f'scopy.runScript("{script_path}")\n'
            print(f"Sending to Scopy console: {cmd.strip()}")
            proc.stdin.write(cmd)
            proc.stdin.flush()
    except Exception as e:
        print(f"Error launching executable: {e}")

if __name__ == "__main__":
    main()

