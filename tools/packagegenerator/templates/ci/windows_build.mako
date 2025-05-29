name: Build Scopy Package (Windows - MinGW)

on:
  push:
  pull_request:

env:
  BUILD_HOST: windows-2019
  USERNAME: github-actions

jobs:
  build_package:
    runs-on: windows-2019

    steps:
      - name: Checkout Package Repository (this repo)
        uses: actions/checkout@v4
        with:
          fetch-depth: 0
          path: package

      - name: Clone Scopy Repository
        uses: actions/checkout@v4
        with:
          repository: analogdevicesinc/scopy
          path: scopy
          ref: main-pkg-manager

      - name: Pull the Docker Image
        run: docker pull cristianbindea/scopy2-mingw64:latest

      - name: Run Docker Image
        shell: cmd
        run: |
              mkdir %GITHUB_WORKSPACE%\artifacts & echo %GITHUB_WORKSPACE%
              set "BUILD_HOST=windows-2019"
              set "USERNAME=github-actions"

              for /F "tokens=1,2 delims=/" %%a in ("%GITHUB_REPOSITORY%") do (
                set "GITHUB_REPO_OWNER=%%a"
                set "GITHUB_REPO_NAME=%%b"
              )

              echo %BUILD_HOST% & echo %USERNAME% & echo %GITHUB_REPO_OWNER% & echo %GITHUB_REPO_NAME%

              docker run ^
                -v %GITHUB_WORKSPACE%:C:\msys64\home\docker\scopy:rw ^
                -v %GITHUB_WORKSPACE%\artifacts:C:\msys64\home\docker\artifact_x86_64:rw ^
                -e GITHUB_WORKSPACE=%GITHUB_WORKSPACE% ^
                -e BUILD_HOST=%BUILD_HOST% ^
                -e USERNAME=%USERNAME% ^
                -e GITHUB_SERVER_URL=%GITHUB_SERVER_URL% ^
                -e GITHUB_API_URL=%GITHUB_API_URL% ^
                -e GITHUB_REPOSITORY_OWNER=%GITHUB_REPO_OWNER% ^
                -e GITHUB_REPOSITORY=%GITHUB_REPO_NAME% ^
                -e GITHUB_RUN_ID=%GITHUB_RUN_ID% ^
                -e GITHUB_RUN_NUMBER=%GITHUB_RUN_NUMBER% ^
                -e GITHUB_JOB=%GITHUB_JOB% ^
                -e RUNNER_ARCH=%RUNNER_ARCH% ^
                -e CI_SCRIPT=ON ^
                cristianbindea/scopy2-mingw64:testing C:\msys64\usr\bin\bash.exe -c "/home/docker/scopy/package/scripts/build_mingw_pkg.sh"

      - name: Set short git commit SHA
        shell: bash
        run: echo "commit_sha=$(git rev-parse --short ${"${{ github.sha }}"})" >> "$GITHUB_ENV"

      - name: Set REPO_NAME env var
        shell: bash
        run: echo "REPO_NAME=$(basename $GITHUB_REPOSITORY)" >> "$GITHUB_ENV"
        
      - name: Upload artifact
        uses: actions/upload-artifact@v4
        with:
          name: ${"${{ env.REPO_NAME }}"}-windows
          path: artifacts/*