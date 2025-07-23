# GitHub Actions Scopy Windows Build Downloader
# Downloads and launches Scopy Windows build from GitHub Actions artifacts

param(
    [Parameter(Position=0)]
    [string]$CommitSha,
    
    [Parameter(Position=1)]
    [string]$Token,
    
    [switch]$Help
)

# Show help if requested
if ($Help) {
    Write-Host "Usage: .\scopy-commit-build-windows.ps1 COMMIT_SHA [TOKEN]" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Scopy Windows Build Downloader" -ForegroundColor Green
    Write-Host ""
    Write-Host "This script downloads and launches Scopy Windows build from GitHub Actions artifacts" -ForegroundColor White
    Write-Host "for the specified commit SHA." -ForegroundColor White
    Write-Host ""
    Write-Host "Parameters:" -ForegroundColor Cyan
    Write-Host "  COMMIT_SHA Commit SHA to search for (REQUIRED)" -ForegroundColor White
    Write-Host "  TOKEN      GitHub personal access token (default: hardcoded token)" -ForegroundColor White
    Write-Host ""
    Write-Host "Example:" -ForegroundColor Cyan
    Write-Host "  .\scopy-commit-build-windows.ps1 ad5758d37c9e0021591013d3ca4a6e6529be839f" -ForegroundColor White
    Write-Host "  .\scopy-commit-build-windows.ps1 ad5758d37c9e0021591013d3ca4a6e6529be839f ghp_your_token_here" -ForegroundColor White
    Write-Host ""
    Write-Host "Dependencies: PowerShell 5.1+, Expand-Archive cmdlet" -ForegroundColor Gray
    exit 0
}

# Configuration constants
$COMMIT_SHA_LENGTH = 40
$MAX_RECENT_RUNS = 3
$MAX_WORKFLOW_RUNS = 1
$EXPECTED_EXECUTABLE = "scopy.exe"
$GITHUB_TOKEN_LENGTH = 36  # Length of token part after 'ghp_' prefix

# Repository details
$Owner = "analogdevicesinc"
$Repo = "scopy"

# Workflow filter
$WorkflowName = "windows-mingw build"

# API Base URL
$apiUrl = "https://api.github.com"

# Validate required parameters
if (-not $CommitSha) {
    Write-Host "Error: COMMIT_SHA is required!" -ForegroundColor Red
    Write-Host "Usage: .\scopy-commit-build-windows.ps1 COMMIT_SHA [TOKEN]" -ForegroundColor Yellow
    Write-Host "Run '.\scopy-commit-build-windows.ps1 -Help' for more information." -ForegroundColor Yellow
    exit 1
}

# Validate commit SHA format (40 character hex string)
if ($CommitSha -notmatch "^[a-fA-F0-9]{$COMMIT_SHA_LENGTH}$") {
    Write-Host "Error: Invalid commit SHA format. Must be $COMMIT_SHA_LENGTH character hex string." -ForegroundColor Red
    Write-Host "Example: ad5758d37c9e0021591013d3ca4a6e6529be839f" -ForegroundColor Yellow
    exit 1
}

# Validate GitHub token format
if (-not $Token) {
    $tokenFile = "github_token.txt"
    if (Test-Path $tokenFile) {
        $Token = Get-Content $tokenFile | Select-Object -First 1
        Write-Host "Token loaded from $tokenFile" -ForegroundColor Cyan
    } else {
        Write-Host "Error: GitHub token not provided and $tokenFile not found." -ForegroundColor Red
        Write-Host "Please provide a token as a parameter or create $tokenFile with your token." -ForegroundColor Yellow
        exit 1
    }
}
if ($Token -notmatch "^gh[pso]_[A-Za-z0-9]{$GITHUB_TOKEN_LENGTH}$") {
    Write-Host "Error: Invalid GitHub token format." -ForegroundColor Red
    Write-Host "Token should start with 'ghp_', 'gho_', or 'ghs_' followed by $GITHUB_TOKEN_LENGTH alphanumeric characters." -ForegroundColor Yellow
    Write-Host "Example: ghp_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" -ForegroundColor Yellow
    exit 1
}

# Check available disk space
function Test-DiskSpace {
    param(
        [int]$RequiredMB = 100,
        [bool]$ShowDetails = $false
    )
    
    try {
        $driveLetter = (Split-Path -Qualifier $PWD).TrimEnd(':')
        $drive = Get-PSDrive -Name $driveLetter
        $availableMB = [math]::Round($drive.Free / 1MB)
        
        if ($availableMB -lt $RequiredMB) {
            Write-Host "Error: Insufficient disk space. Available: ${availableMB}MB, Required: ${RequiredMB}MB" -ForegroundColor Red
            return $false
        }
        
        if ($ShowDetails) {
            Write-Host "Disk space check passed: ${availableMB}MB available, ${RequiredMB}MB required" -ForegroundColor Green
        }
        
        return $true
    } catch {
        Write-Host "Warning: Cannot check disk space" -ForegroundColor Yellow
        return $true
    }
}

# Initial disk space check
if (-not (Test-DiskSpace -RequiredMB 100 -ShowDetails $true)) {
    exit 1
}

# Set up headers with authentication
$headers = @{
    "Authorization" = "token $Token"
    "Accept" = "application/vnd.github.v3+json"
    "User-Agent" = "PowerShell-Scopy-Downloader"
}

# Function to handle API errors
function Test-ApiResponse {
    param(
        [PSObject]$Response,
        [string]$Context
    )
    
    if ($Response.message) {
        Write-Host "GitHub API error in $Context`: $($Response.message)" -ForegroundColor Red
        if ($Response.documentation_url) {
            Write-Host "Documentation: $($Response.documentation_url)" -ForegroundColor Yellow
        }
        return $false
    }
    
    return $true
}

# Function to cleanup files on failure
function Remove-FailedDownload {
    param([string]$FilePath)
    
    if (Test-Path $FilePath) {
        Write-Host "Cleaning up failed download: $FilePath" -ForegroundColor Yellow
        Remove-Item $FilePath -Force -ErrorAction SilentlyContinue
    }
}

# Function to download artifact
function Get-WorkflowArtifact {
    param(
        [PSObject]$Artifact
    )
    
    $artifactName = $Artifact.name
    $downloadUrl = $Artifact.archive_download_url
    $downloadPath = ".\$artifactName.zip"
    
    Write-Host "Downloading: $artifactName" -ForegroundColor Cyan
    
    try {
        Invoke-WebRequest -Uri $downloadUrl -Headers $headers -OutFile $downloadPath -UseBasicParsing
        
        if (Test-Path $downloadPath) {
            Write-Host "Downloaded successfully: $downloadPath" -ForegroundColor Green
            return $downloadPath
        } else {
            Write-Host "Download failed: $artifactName" -ForegroundColor Red
            return $null
        }
    } catch {
        Write-Host "Error downloading $artifactName`: $($_.Exception.Message)" -ForegroundColor Red
        Remove-FailedDownload $downloadPath
        return $null
    }
}

# Function to extract artifact
function Expand-WorkflowArtifact {
    param([string]$ZipPath)
    
    if (-not (Test-Path $ZipPath)) {
        Write-Host "Error: Zip file does not exist: $ZipPath" -ForegroundColor Red
        return $false
    }
    
    # Ensure extraction happens inside the artifact folder
    $parentFolder = Split-Path $ZipPath -Parent
    $extractFolderName = [System.IO.Path]::GetFileNameWithoutExtension($ZipPath)
    $extractPath = Join-Path $parentFolder $extractFolderName
    
    Write-Host "Extracting: $ZipPath" -ForegroundColor Cyan
    
    try {
        Expand-Archive -Path $ZipPath -DestinationPath $extractPath -Force
        
        # Remove zip file after successful extraction
        Remove-Item $ZipPath -Force
        
        Write-Host "Extracted successfully to: $extractPath" -ForegroundColor Green
        
        # Check for double archive (nested ZIP files)
        $nestedZips = Get-ChildItem -Path $extractPath -Filter "*.zip" -Recurse
        $nestedExtractionPaths = @()
        
        if ($nestedZips) {
            Write-Host "Found nested ZIP file(s), extracting..." -ForegroundColor Yellow
            foreach ($nestedZip in $nestedZips) {
                $nestedExtractFolder = [System.IO.Path]::GetFileNameWithoutExtension($nestedZip.Name)
                $nestedExtractPath = Join-Path $parentFolder $nestedExtractFolder
                Write-Host "Extracting nested ZIP: $($nestedZip.Name) to $nestedExtractPath" -ForegroundColor Cyan
                try {
                    Expand-Archive -Path $nestedZip.FullName -DestinationPath $nestedExtractPath -Force
                    # Remove nested zip after extraction
                    Remove-Item $nestedZip.FullName -Force
                    Write-Host "Nested ZIP extracted to: $nestedExtractPath" -ForegroundColor Green
                    # Track nested extraction paths for prioritized search
                    $nestedExtractionPaths += $nestedExtractPath
                } catch {
                    Write-Host "Warning: Failed to extract nested ZIP $($nestedZip.Name): $($_.Exception.Message)" -ForegroundColor Yellow
                }
            }
        }
        
        # Return both the main extract path and nested extraction paths
        return @{
            Success = $true
            ExtractPath = $extractPath
            NestedPaths = $nestedExtractionPaths
        }
    } catch {
        Write-Host "Error extracting $ZipPath`: $($_.Exception.Message)" -ForegroundColor Red
        return @{
            Success = $false
            ExtractPath = $null
            NestedPaths = @()
        }
    }
}

# Function to launch Scopy
function Start-Scopy {
    param([string]$ExecutablePath)
    
    if (-not (Test-Path $ExecutablePath)) {
        Write-Host "Error: Scopy executable not found: $ExecutablePath" -ForegroundColor Red
        return $false
    }
    
    Write-Host "Launching Scopy..." -ForegroundColor Green
    Write-Host "Location: $ExecutablePath" -ForegroundColor Cyan
    
    try {
        Start-Process -FilePath $ExecutablePath -WindowStyle Normal
        Write-Host "Scopy launched successfully!" -ForegroundColor Green
        return $true
    } catch {
        Write-Host "Failed to launch Scopy: $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

# Function to process artifact (download, extract, launch)
function Invoke-ArtifactProcessing {
    param([PSObject]$Artifact)
    
    $artifactName = $Artifact.name
    $expired = $Artifact.expired
    $sizeBytes = $Artifact.size_in_bytes
    
    if ($expired -eq $true) {
        Write-Host "Skipping expired artifact: $artifactName" -ForegroundColor Yellow
        return $false
    }
    
    # Check disk space for this artifact
    $sizeMB = [math]::Ceiling($sizeBytes / 1MB)
    $requiredMB = $sizeMB + 50  # Buffer for extraction
    
    if (-not (Test-DiskSpace -RequiredMB $requiredMB)) {
        Write-Host "Artifact size: ${sizeMB}MB, Required disk space: ${requiredMB}MB" -ForegroundColor Cyan
        return $false
    }
    
    # Build folder name: MM_DD_YYYY_SHA
    $createdDateTime = Get-Date $Artifact.created_at -Format "MM_dd_yyyy_HH_mm_ss"
    $folderName = "${createdDateTime}_${CommitSha}"
    if (-not (Test-Path $folderName)) {
        New-Item -ItemType Directory -Path $folderName | Out-Null
    }

    # Download the artifact into the folder
    $artifactName = $Artifact.name
    $downloadPath = Join-Path $folderName "$artifactName.zip"
    $downloaded = $false
    try {
        $downloadUrl = $Artifact.archive_download_url
        Write-Host "Downloading: $artifactName to $downloadPath" -ForegroundColor Cyan
        Invoke-WebRequest -Uri $downloadUrl -Headers $headers -OutFile $downloadPath -UseBasicParsing
        if (Test-Path $downloadPath) {
            Write-Host "Downloaded successfully: $downloadPath" -ForegroundColor Green
            $downloaded = $true
        } else {
            Write-Host "Download failed: $artifactName" -ForegroundColor Red
        }
    } catch {
        Write-Host "Error downloading $artifactName`: $($_.Exception.Message)" -ForegroundColor Red
        Remove-FailedDownload $downloadPath
    }
    if (-not $downloaded) {
        return $false
    }

    # Extract the artifact inside the folder
    $extractResult = Expand-WorkflowArtifact $downloadPath
    if (-not $extractResult.Success) {
        return $false
    }


    # Look for Scopy executable - use absolute extraction paths
    $extractPath = $extractResult.ExtractPath
    $nestedPaths = $extractResult.NestedPaths

    Write-Host "Searching for Scopy executable in: $extractPath" -ForegroundColor Cyan
    if ($nestedPaths.Count -gt 0) {
        Write-Host "Prioritizing nested extraction paths: $($nestedPaths -join ', ')" -ForegroundColor Cyan
    }

    # Create search paths with nested extraction paths first (highest priority)
    $searchPaths = @()
    foreach ($nestedPath in $nestedPaths) {
        $searchPaths += $nestedPath
    }
    $searchPaths += $extractPath

    # Add subdirectories as fallback (avoid duplicates)
    $allDirs = Get-ChildItem -Path $extractPath -Directory -Recurse -ErrorAction SilentlyContinue
    $deepestDirs = $allDirs | Sort-Object { $_.FullName.Split('_').Count } -Descending
    foreach ($dir in $deepestDirs) {
        if ($searchPaths -notcontains $dir.FullName) {
            $searchPaths += $dir.FullName
        }
    }

    Write-Host "Search paths (nested extraction paths first):" -ForegroundColor Gray
    foreach ($path in $searchPaths) {
        Write-Host "  - $path" -ForegroundColor Gray
    }

    # Try to find the executable - look for Scopy.exe (capitalized only)
    $scopyPath = $null
    foreach ($path in $searchPaths) {
        $scopyPath = Get-ChildItem -Path $path -Filter "Scopy.exe" -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($scopyPath) {
            Write-Host "Found Scopy.exe in: $path" -ForegroundColor Green
            break
        }
    }

    if ($scopyPath) {
        Write-Host "Found Scopy executable: $($scopyPath.FullName)" -ForegroundColor Green
        Start-Scopy $scopyPath.FullName
        return $true
    } else {
        Write-Host "Scopy executable not found in extracted files" -ForegroundColor Yellow
        Write-Host "Directory structure:" -ForegroundColor Gray
        $allItems = Get-ChildItem -Path $extractPath -Recurse
        foreach ($item in $allItems) {
            if ($item.PSIsContainer) {
                Write-Host "  [DIR]  $($item.FullName)" -ForegroundColor Blue
            } else {
                Write-Host "  [FILE] $($item.Name) (Path: $($item.FullName))" -ForegroundColor Gray
            }
        }
        return $false
    }
}

Write-Host "GitHub API connection..." -ForegroundColor Blue
Write-Host "Repository: $Owner/$Repo" -ForegroundColor Green

try {
    # Get workflow runs filtered by commit SHA
    $urlWithSha = "$apiUrl/repos/$Owner/$Repo/actions/runs?head_sha=$CommitSha"
    
    $responseWithSha = Invoke-RestMethod -Uri $urlWithSha -Headers $headers -Method Get
    
    if (-not (Test-ApiResponse $responseWithSha "workflow runs with SHA")) {
        exit 1
    }
    
    if ($responseWithSha.total_count -gt 0) {
        Write-Host "Found $($responseWithSha.total_count) runs for commit $($CommitSha.Substring(0,7))" -ForegroundColor Green
        
        # Filter by workflow name
        $filteredRuns = $responseWithSha.workflow_runs | Where-Object { $_.name -eq $WorkflowName }
        
        if ($filteredRuns -and (@($filteredRuns).Count -gt 0)) {
            Write-Host "Found matching workflow run: $WorkflowName" -ForegroundColor Green
            
            # Get artifacts for the first matching run
            $runId = $filteredRuns[0].id
            $artifactsUrl = "$apiUrl/repos/$Owner/$Repo/actions/runs/$runId/artifacts"
            
            Write-Host "Fetching artifacts for run ID: $runId" -ForegroundColor Cyan
            $artifactsResponse = Invoke-RestMethod -Uri $artifactsUrl -Headers $headers -Method Get
            
            if (-not (Test-ApiResponse $artifactsResponse "artifacts")) {
                exit 1
            }
            
            if ($artifactsResponse.total_count -gt 0) {
                Write-Host "Found $($artifactsResponse.total_count) artifact(s)" -ForegroundColor Green
                
                # Filter artifacts to only include Windows x86_64 builds (excluding setup)
                $windowsArtifacts = $artifactsResponse.artifacts | Where-Object { 
                    $_.name -like "scopy-windows-x86_64*" -and $_.name -notlike "*setup*" 
                }
                
                if ($windowsArtifacts) {
                    Write-Host "Found $(@($windowsArtifacts).Count) Windows x86_64 artifact(s)" -ForegroundColor Green
                    
                    # Process each Windows artifact
                    $processedCount = 0
                    foreach ($artifact in $windowsArtifacts) {
                        Write-Host "`nProcessing artifact: $($artifact.name)" -ForegroundColor Magenta
                        Write-Host "Size: $([math]::Round($artifact.size_in_bytes / 1MB, 2)) MB" -ForegroundColor Gray
                        Write-Host "Created: $($artifact.created_at)" -ForegroundColor Gray
                        
                        if (Invoke-ArtifactProcessing $artifact) {
                            $processedCount++
                            break  # Stop after first successful artifact
                        }
                    }
                } else {
                    Write-Host "No Windows x86_64 artifacts found (looking for names starting with 'scopy-windows-x86_64' but excluding 'setup')" -ForegroundColor Yellow
                    Write-Host "Available artifacts:" -ForegroundColor Cyan
                    foreach ($artifact in $artifactsResponse.artifacts) {
                        Write-Host "  - $($artifact.name)" -ForegroundColor Gray
                    }
                    exit 1
                }
                
                if ($processedCount -eq 0) {
                    Write-Host "No artifacts were successfully processed" -ForegroundColor Yellow
                    exit 1
                } else {
                    Write-Host "Successfully processed artifact and launched Scopy!" -ForegroundColor Green
                }
            } else {
                Write-Host "No artifacts found for this workflow run" -ForegroundColor Yellow
                exit 1
            }
        } else {
            Write-Host "No '$WorkflowName' workflow found for commit $($CommitSha.Substring(0,7))" -ForegroundColor Yellow
            Write-Host "Available workflows for this commit:" -ForegroundColor Cyan
            foreach ($run in $responseWithSha.workflow_runs) {
                Write-Host "  - $($run.name) (Status: $($run.status))" -ForegroundColor Gray
            }
            exit 1
        }
    } else {
        Write-Host "No workflow runs found for commit $($CommitSha.Substring(0,7))" -ForegroundColor Yellow
        
        # Fallback: Show recent runs
        Write-Host "`nShowing recent workflow runs..." -ForegroundColor Yellow
        $url = "$apiUrl/repos/$Owner/$Repo/actions/runs"
        $response = Invoke-RestMethod -Uri $url -Headers $headers -Method Get
        
        Write-Host "Recent commits:" -ForegroundColor Cyan
        $response.workflow_runs | Select-Object -First 5 | ForEach-Object {
            Write-Host "  - $($_.name): $($_.head_sha.Substring(0,7)) (Status: $($_.status))" -ForegroundColor Gray
        }
        exit 1
    }
} catch {
    Write-Host "API call failed!" -ForegroundColor Red
    Write-Host "Error: $($_.Exception.Message)" -ForegroundColor Red
    
    if ($_.Exception.Response) {
        $statusCode = $_.Exception.Response.StatusCode.value__
        Write-Host "Status Code: $statusCode" -ForegroundColor Red
        
        switch ($statusCode) {
            401 { Write-Host "Authentication failed - check your token" -ForegroundColor Red }
            404 { Write-Host "Repository not found or no access" -ForegroundColor Red }
            403 { Write-Host "Forbidden - insufficient permissions" -ForegroundColor Red }
            default { Write-Host "HTTP Error: $statusCode" -ForegroundColor Red }
        }
    }
    exit 1
}

Write-Host "`nScript completed successfully." -ForegroundColor Blue
