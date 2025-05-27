
name: Build Scopy Package (Ubuntu 22.04)

on:
  push:
  pull_request:

env:
  BUILD_HOST: ubuntu-22.04
  USERNAME: github-actions

jobs:
  build_package:
    runs-on: ubuntu-22.04
    container:
      image: cristianbindea/scopy2-ubuntu22:latest
      options: --user root

    steps:
      - name: Checkout Package Repository (this repo)
        uses: actions/checkout@v4
        with:
          path: package

      - name: Get Repository Name
        id: repo
        run: echo "REPO_NAME=$(basename $GITHUB_REPOSITORY)" >> $GITHUB_ENV

      - name: Clone Scopy Repository
        uses: actions/checkout@v4
        with:
          repository: analogdevicesinc/scopy
          path: scopy
          ref: main-pkg-manager

      - name: Check package generator
        run: |
          cd scopy/tools/packagegenerator
          pip3 install -r requirements.txt
          python3 ./package_generator.py -h

      - name: Inject Package into Scopy
        run: |
          mkdir -p scopy/packages/$REPO_NAME
          cp -r package/* scopy/packages/$REPO_NAME/

      - name: Build Package in Scopy
        run: |
          git config --global --add safe.directory $GITHUB_WORKSPACE/scopy
          cd scopy
          mkdir -p build && cd build
          cmake .. -DCMAKE_PREFIX_PATH=/opt/Qt/5.15.2/gcc_64/lib/cmake/Qt5
          make -j$(nproc)
          cp -r packages/$REPO_NAME $GITHUB_WORKSPACE || echo "Package directory not found!"
      
      - name: Archive using package generator
        run: |
          cd scopy/tools/packagegenerator
          python3 ./package_generator.py -a --src=$GITHUB_WORKSPACE/$REPO_NAME --dest=$GITHUB_WORKSPACE

      - name: Upload artifact
        uses: actions/upload-artifact@v4
        with:
          name: ${"${{ env.REPO_NAME }}"}-ubuntu22
          path: ${"${{ github.workspace }}"}/${"${{ env.REPO_NAME }}"}.zip