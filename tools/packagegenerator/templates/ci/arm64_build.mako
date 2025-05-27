
name: Build Scopy Package (AppImage-arm64)

on:
  push:
  pull_request:

env:
  BUILD_HOST: ubuntu-22.04
  USERNAME: github-actions

jobs:
  build_arm64_package:
    runs-on: ubuntu-22.04
    steps:
      - name: Checkout Package Repository (this repo)
        uses: actions/checkout@v4
        with:
          set-safe-directory: 'true'
          fetch-depth: 0
          path: package

      - name: Clone Scopy Repository
        uses: actions/checkout@v4
        with:
          set-safe-directory: 'true'
          repository: analogdevicesinc/scopy
          path: scopy
          ref: main-pkg-manager

      - name: Check package generator
        run: |
          cd $GITHUB_WORKSPACE/scopy/tools/packagegenerator
          pip3 install -r requirements.txt
          python3 ./package_generator.py -h

      - name: Pull the Docker Image
        run: docker pull cristianbindea/scopy2-arm64-appimage:latest

      - name: Get Repository Name
        id: repo
        run: echo "REPO_NAME=$(basename $GITHUB_REPOSITORY)" >> $GITHUB_ENV

      - name: Build package
        shell: bash
        run: |
          echo 'Copy package to scopy/packages'
          mkdir -p $GITHUB_WORKSPACE/scopy/packages/$REPO_NAME 
          cp -r $GITHUB_WORKSPACE/package/* $GITHUB_WORKSPACE/scopy/packages/$REPO_NAME/
          
          cd $GITHUB_WORKSPACE/scopy
          sudo apt update
          ./ci/arm/create_sysroot.sh arm64  install_packages
          ./ci/arm/arm_build_process.sh arm64 generate_ci_envs
<%text>
          docker run \
                  --mount type=bind,source="$GITHUB_WORKSPACE/scopy",target=/home/runner/scopy \
                  --env-file $GITHUB_WORKSPACE/scopy/ci/general/gh-actions.envs \
                  --env REPO_NAME=$REPO_NAME \
                  cristianbindea/scopy2-arm64-appimage:latest \
                  /bin/bash -c    'cd $HOME && \
                                  sudo chown -R runner:runner scopy && \
                                  cd $HOME/scopy && \
                                  ./ci/arm/arm_build_process.sh arm64 install_packages move_tools move_sysroot build_scopy'
</%text>
      - name: Set short git commit SHA
        shell: bash
        run: echo "commit_sha=$(git rev-parse --short ${"${{ github.sha }}"})" >> "$GITHUB_ENV"
      - name: Archive using package generator
        run: |
          cd $GITHUB_WORKSPACE/scopy/tools/packagegenerator
          python3 ./package_generator.py -a --src=$GITHUB_WORKSPACE/scopy/build/packages/$REPO_NAME --dest=$GITHUB_WORKSPACE/
      - uses: actions/upload-artifact@v4
        with:
          name: linux-arm64-${"${{ env.REPO_NAME }}"}
          path: ${"${{ github.workspace }}"}/${"${{ env.REPO_NAME }}"}.zip