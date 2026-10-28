# f3d-stl-thumbnailer

A KDE Dolphin thumbnailer that renders **STL** (3D model) files as preview
images using [F3D](https://f3d.app/).

## Why this exists

Dolphin does **not** read the freedesktop `.thumbnailer` text files — those only
work in GNOME Files (Nautilus) and XFCE (Tumbler). Dolphin loads **only** compiled
`ThumbCreator` plugins (`.so` files in `/usr/lib/qt6/plugins/kf6/thumbcreator/`).

The usual STL thumbnailer for KDE, [`stl-thumb-kde`](https://github.com/unlimitedbacon/stl-thumb-kde),
is broken on KDE/Wayland: its renderer (`stl-thumb`) uses a `winit` event loop that
panics when called repeatedly from the long-running thumbnail worker
([stl-thumb#132](https://github.com/unlimitedbacon/stl-thumb/issues/132)) and
renders blank frames. F3D renders STL **headlessly** (VTK offscreen, no display
needed), which is exactly what a thumbnail worker requires.

## Requirements

- `f3d` (in the official Arch repo: `sudo pacman -S f3d`)
- Qt 6 (`Qt6Core`, `Qt6Widgets`)
- KDE Frameworks 6 (`KF6::KIO`)
- `extra-cmake-modules` (build only)

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

This produces `build/stlthumbnail.so`.

## Install

```bash
sudo install -Dm755 build/stlthumbnail.so /usr/lib/qt6/plugins/kf6/thumbcreator/stlthumbnail.so
kbuildsycoca6
rm -rf ~/.cache/thumbnails/*   # clear any stale failed-thumbnail cache
```

Then restart Dolphin.

## Test

A test harness lives in `test/test_stlcreator.cpp`. It instantiates the creator
and asserts a non-blank render:

```bash
cmake -S . -B build -DF3D_STL_BUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/test_stlcreator /path/to/model.stl
```

**Note:** the test does **not** compile against **Qt 6.12.0**, which dropped
`noexcept` from `QVariant::~QVariant` and broke `QVariantList` default
construction (`static_assert(std::is_nothrow_destructible_v<QVariant>)`). Use
Qt < 6.12 or >= 6.12.1. The plugin itself is unaffected (it never constructs a
`QVariantList`).

## How it works

`StlCreator::create()` shells out to F3D with its built-in thumbnail preset:

```
f3d --config=thumbnail --verbose=quiet --output=<tmp.png> --resolution=<w>,<h> <model.stl>
```

then loads the resulting PNG into a `QImage` and returns it to Dolphin. No
window, no event loop, no display required.

## License

MIT — see [LICENSE](LICENSE).
