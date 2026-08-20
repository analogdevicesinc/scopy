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
./create_docker_image.sh          # → ...scopy2-flatpak-qt6:testing
./create_docker_image.sh slim     # → ...scopy2-flatpak-qt6:slim   (dependency-rework pass)
cd ../..

docker images | grep scopy2-flatpak-qt6   # confirm it exists
```

The tag argument defaults to `testing`. The variant lives in the **tag**, not the image name: slim
replaces the full image once validated, so a name suffix would have to be renamed at promotion and
would break every pull URL. `get_docker_tag.yml` resolves `*deps_rework*` branches to `slim`.

> On the slim manifest this step is **faster** — boost and gnuradio, the two longest modules, are
> gone. Expect a full cache miss the first time regardless: removing modules changes the cache key
> of every later module, so nothing from an older `:testing` image is reusable.

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
  docker.cloudsmith.io/adi/scopy-dockers/scopy2-flatpak-qt6:slim \
  /bin/bash -c "sudo chown -R runner:runner '$PWD' && '$PWD/ci/flatpak/flatpak_build_process.sh'"

echo "exit=$?"          # check the real exit code, not just that the wrapper finished
ls -lh Scopy.flatpak    # should now exist in the repo root
```

Swap `:slim` for `:testing` when validating the full manifest. On the slim manifest, also confirm
the bundle contains none of the dropped stacks:

```bash
ls ci/flatpak/build/files/lib | grep -E 'gnuradio|volk|spdlog|sigrokdecode|m2k|^libgr|boost|glog|fmt'
# expect no output
ls ci/flatpak/build/files/lib | grep -E 'ad9166|ad9361|libiio|qwt_scopy'
# expect all four
```

> **Afterwards, restore ownership of your checkout.** The `sudo chown -R runner:runner` in the
> command above runs against the *bind mount*, so it leaves the whole tree owned by the container's
> `runner` user (uid 1001) and unwritable by you. This is inherent to the bind-mount approach, not a
> failure:
>
> ```bash
> sudo chown -R "$USER:$USER" "$PWD"
> ```
>
> On a tree that also holds `ci/arm/staging` or a populated `ci/flatpak/.flatpak-builder` this walks
> ~100 GB of inodes and takes a while; `sudo chown -R "$USER:$USER" ci docs tasks` is enough to get
> back to editing.

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
# dependency-rework pass:
docker push docker.cloudsmith.io/adi/scopy-dockers/scopy2-flatpak-qt6:slim
```

> The `flatpak-qt6` job in `ci.yml` is currently `if: false` because no `:slim` tag existed.
> Re-enable it (the original condition is preserved in the comment below the `if:`) **only after
> this push succeeds** — otherwise every run fails at `docker pull`.

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
