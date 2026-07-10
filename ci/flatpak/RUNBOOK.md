# Scopy Qt6 Flatpak — Manual Runbook

Steps to build the Flatpak Docker image, test the Flatpak generation locally with
that image, push the image to Cloudsmith, and validate the CI. Run everything from
the repo root (`scopy/`) unless a step says otherwise.

The build targets the KDE Qt6 runtime `org.kde.Platform` / `org.kde.Sdk` **6.8**.
The CI image lives at `docker.cloudsmith.io/adi/scopy-dockers/scopy2-flatpak-qt6`.

> **Ordering gotcha:** the image (steps 1–3) must exist on Cloudsmith **before** the
> CI runs (step 4), otherwise the `flatpak-qt6` job fails at `docker pull`.

## 0. Prerequisites (once)

```bash
# Docker + the flathub shared-modules submodule are required to build the image
git submodule update --init ci/flatpak/shared-modules

# Cloudsmith CLI, for the push step later
pip install --upgrade cloudsmith-cli
```

## 1. Build the Docker image locally

`create_docker_image.sh` builds the image **and** bakes the full dependency cache
into it (`build_flatpak_deps.sh` runs `flatpak-builder --stop-at=scopy` inside).
This is the long step (~30–60 min — builds boost, gnuradio, etc.).

```bash
cd ci/flatpak
./create_docker_image.sh
# → produces image: docker.cloudsmith.io/adi/scopy-dockers/scopy2-flatpak-qt6:testing
cd ../..

docker images | grep scopy2-flatpak-qt6   # confirm it exists
```

> All dependency branches/tags in `ci/flatpak/org.adi.Scopy.json.c` are pinned to the
> exact same versions as the AppImage/Ubuntu/Windows Qt6 builds (KDDockWidgets `2.2`,
> extra-cmake-modules / karchive `v6.8.0`, libiio `v0.26`, gnuradio `scopy2-maint-3.10`,
> etc.), so no further version surprises are expected. If a Qt6 dependency does fail,
> cross-check its config against `ci/x86_64/x86-64_appimage_process.sh` and fix the
> branch/tag in the manifest, then re-run.

## 2. Test Flatpak generation locally using that image

This mirrors exactly what the CI job does — mounts your checkout, builds Scopy from
the local source, and emits `Scopy.flatpak`:

```bash
docker run --rm --privileged \
  -e CI_SCRIPT=ON \
  -e GITHUB_WORKSPACE="$PWD" \
  --mount type=bind,source="$PWD",target="$PWD" \
  docker.cloudsmith.io/adi/scopy-dockers/scopy2-flatpak-qt6:testing \
  /bin/bash -c "sudo chown -R runner:runner '$PWD' && '$PWD/ci/flatpak/flatpak_build_process.sh'"

ls -lh Scopy.flatpak   # should now exist in the repo root
```

Optional — install & launch it (needs flatpak + the KDE 6.8 runtime on the host):

```bash
flatpak remote-add --if-not-exists --user flathub https://dl.flathub.org/repo/flathub.flatpakrepo
flatpak install --user flathub org.kde.Platform//6.8 org.kde.Sdk//6.8 -y
flatpak install --user Scopy.flatpak -y
flatpak run org.adi.Scopy
# if the window fails to appear on Wayland, force xcb:
# flatpak run --env=QT_QPA_PLATFORM=xcb org.adi.Scopy
```

## 3. Push the image to Cloudsmith (only if step 2 worked)

```bash
# Authenticate to the Cloudsmith docker registry
# (CI uses OIDC; locally use a token / API key)
docker login docker.cloudsmith.io -u token -p "$CLOUDSMITH_API_KEY"

docker push docker.cloudsmith.io/adi/scopy-dockers/scopy2-flatpak-qt6:testing
```

Alternatively, do steps 1 + 3 from the GitHub UI:
**Actions → "Push Qt6 Docker Image to Cloudsmith" → Run workflow →
platform `flatpak_qt6`, tag `testing`.**

## 4. Test the CI

The `flatpak-qt6` job in `.github/workflows/ci.yml` runs on push/PR and pulls the
image tagged by branch name. Branch `scopy_qt6_flatpack_ci` maps to the **`testing`**
tag (see `get_docker_tag.yml`), matching what you pushed in step 3.

```bash
git add -A
git commit -m "ci: port flatpak build to qt6"
git push origin scopy_qt6_flatpack_ci
```

Then in **Actions → "Build & Test Qt6"**, confirm the `flatpak-qt6` job:
1. pulls `scopy2-flatpak-qt6:testing`,
2. runs `ci/flatpak/flatpak_build_process.sh`,
3. uploads the `scopy-x86_64-flatpak-<sha>` artifact.
