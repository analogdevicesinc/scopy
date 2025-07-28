import argparse
import sys
import os
import requests
import platform
import subprocess
import stat
import datetime
import zipfile

def print_progress_bar(iteration, total, prefix='', suffix='', length=40):
    percent = f"{100 * (iteration / float(total)):.1f}" if total else "0.0"
    filled_length = int(length * iteration // total) if total else 0
    bar = '█' * filled_length + '-' * (length - filled_length)
    print(f'\r{prefix} |{bar}| {percent}% {suffix}', end='\r')
    if iteration == total:
        print()

def autodetect_platform():
    sys_platform = platform.system().lower()
    if 'windows' in sys_platform:
        return 'windows'
    elif 'linux' in sys_platform:
        # Try to detect arm64
        machine = platform.machine().lower()
        if 'aarch64' in machine or 'arm64' in machine:
            return 'arm64'
        return 'linux'
    else:
        return None

def read_token_from_file(token_file="github_token.txt"):
    if os.path.isfile(token_file):
        with open(token_file, "r") as f:
            return f.readline().strip()
    return None

def parse_arguments():
    parser = argparse.ArgumentParser(
        description="Scopy Build Downloader: Downloads and launches Scopy build from GitHub Actions artifacts for a given commit SHA."
    )
    parser.add_argument("commit_sha", type=str, help="Commit SHA to search for (required)")
    parser.add_argument("--token", type=str, required=False, help="GitHub personal access token (optional, fallback to github_token.txt)")
    parser.add_argument("--platform", type=str, choices=["windows", "linux", "arm64"], required=False, help="Target platform (windows, linux, arm64)")
    return parser.parse_args()

def validate_commit_sha(commit_sha):
    # Accept full SHA (40 chars) or short SHA (>=7 chars)
    return commit_sha and (7 <= len(commit_sha) <= 40) and all(c in "0123456789abcdefABCDEF" for c in commit_sha)

def get_token(token_arg):
    return token_arg or read_token_from_file()

def get_workflow_name(platform):
    if platform == "windows":
        return "windows-mingw build"
    elif platform == "linux":
        return "Scopy x86_64 AppImage Build"
    elif platform == "arm64":
        return "Scopy arm64 AppImage Build"
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
    runs = [run for run in data.get("workflow_runs", []) if run.get("name") == workflow_name and run.get("head_sha", "").startswith(commit_sha)]
    # If no runs and short SHA, retry without head_sha filter and search for partial matches
    if not runs and len(commit_sha) < 40:
        print(f"No workflow runs found for commit {commit_sha}, retrying with all recent runs...")
        api_url_all = f"https://api.github.com/repos/{owner}/{repo}/actions/runs"
        try:
            response_all = requests.get(api_url_all, headers=headers)
            response_all.raise_for_status()
            data_all = response_all.json()
        except Exception as e:
            print(f"Error connecting to GitHub API (all runs): {e}")
            sys.exit(1)
        partial_matches = [run for run in data_all.get("workflow_runs", []) if run.get("name") == workflow_name and commit_sha in run.get("head_sha", "")]
        if partial_matches:
            print(f"Found {len(partial_matches)} workflow runs partially matching SHA '{commit_sha}':")
            for idx, run in enumerate(partial_matches):
                print(f"  [{idx+1}] SHA: {run.get('head_sha')} | Status: {run.get('status')} | ID: {run.get('id')}")
            # Always use the first match
            return partial_matches[0]
        else:
            print(f"No '{workflow_name}' workflow found for commit {commit_sha}")
            print("Available workflows for this commit:")
            for run in data_all.get("workflow_runs", []):
                print(f"  - {run.get('name')} (Status: {run.get('status')})")
            sys.exit(1)
    elif not runs:
        print(f"No '{workflow_name}' workflow found for commit {commit_sha}")
        print("Available workflows for this commit:")
        for run in data.get("workflow_runs", []):
            print(f"  - {run.get('name')} (Status: {run.get('status')})")
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
    else:
        return []

def download_artifact(artifact, commit_sha, headers):
    artifact_name = artifact['name']
    created_at = artifact['created_at']
    try:
        dt = datetime.datetime.fromisoformat(created_at.replace('Z', '+00:00'))
    except Exception:
        dt = datetime.datetime.now()
    date_folder = dt.strftime('%d_%m_%Y:%H:%M')
    folder_name = f"{date_folder}_{commit_sha}"
    if not os.path.exists(folder_name):
        os.makedirs(folder_name)
    zip_path = os.path.join(folder_name, f"{artifact_name}.zip")
    download_url = artifact['archive_download_url']
    print(f"Downloading artifact to: {zip_path}")
    try:
        with requests.get(download_url, headers=headers, stream=True) as r:
            r.raise_for_status()
            total = int(r.headers.get('content-length', 0))
            downloaded = 0
            chunk_size = 8192
            with open(zip_path, 'wb') as f:
                for chunk in r.iter_content(chunk_size=chunk_size):
                    if chunk:
                        f.write(chunk)
                        downloaded += len(chunk)
                        print_progress_bar(downloaded, total, prefix='Downloading', suffix='Complete', length=40)
            # Ensure progress bar completes if total is unknown
            if total == 0:
                print_progress_bar(1, 1, prefix='Downloading', suffix='Complete', length=40)
        print(f"Download complete: {zip_path}")
        return zip_path, folder_name
    except Exception as e:
        print(f"Error downloading artifact: {e}")
        return None, folder_name

def extract_zip(zip_path, extract_to):
    print(f"Extracting {zip_path} to {extract_to}")
    try:
        with zipfile.ZipFile(zip_path, 'r') as zip_ref:
            members = zip_ref.namelist()
            total = len(members)
            for i, member in enumerate(members, 1):
                zip_ref.extract(member, extract_to)
                print_progress_bar(i, total, prefix='Extracting', suffix='Complete', length=40)
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
                        members = zip_ref.namelist()
                        total = len(members)
                        for i, member in enumerate(members, 1):
                            zip_ref.extract(member, extract_to)
                            print_progress_bar(i, total, prefix='Extracting', suffix='Complete', length=40)
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
        print("Error: commit_sha must be a hex string (of 7 characters (short sha), or 40 characters (full sha)).")
        print("Example: ad5758d37c9e0021591013d3ca4a6e6529be839f or ad6bd6b")
        sys.exit(1)

    token = get_token(args.token)
    if not token:
        print("Error: GitHub token not provided and github_token.txt not found.")
        print("Please provide a token with --token or create github_token.txt with your token.")
        sys.exit(1)

    print(f"Commit SHA: {args.commit_sha}")

    # Auto-detect platform if not provided
    platform_value = args.platform if args.platform else autodetect_platform()
    print(f"Platform: {platform_value}")

    owner = "analogdevicesinc"
    repo = "scopy"
    workflow_name = get_workflow_name(platform_value)
    if not workflow_name:
        print("Error: Only Windows and Linux platforms are supported in this script version.")
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
        exe_name = "scopy.exe"
    elif platform_value == "linux":
        exe_name = "Scopy-x86_64.AppImage"
    elif platform_value == "arm64":
        exe_name = "Scopy-arm64.AppImage"
    else:
        exe_name = None

    exe_path = find_executable(folder_name, exe_name)
    if not exe_path:
        print(f"Executable not found for platform '{platform_value}'. Searched for: {exe_name}")
        sys.exit(1)
    print(f"Found executable: {exe_path}")

    # Launch the executable
    try:
        if platform_value == "windows":
            os.startfile(exe_path)
        elif platform_value == "linux":
            st = os.stat(exe_path)
            if not (st.st_mode & stat.S_IXUSR):
                os.chmod(exe_path, st.st_mode | stat.S_IXUSR)
            subprocess.Popen([exe_path])
        else:
            print(f"Platform '{platform_value}' is not supported for launching.")
    except Exception as e:
        print(f"Error launching executable: {e}")

if __name__ == "__main__":
    main()
