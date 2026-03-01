# Point Cloud Tool

Professional 3D point cloud visualization and annotation tool built on CloudCore engine.

## Building

### Development Build (Using Local CloudCore)

```bash
cd pointcloudtool
mkdir build && cd build
cmake .. -DUSE_LOCAL_CLOUDCORE=ON
make -j$(nproc)
./pointcloudtool
```

### Quick Start

From the root directory:

```bash
cd pointcloudtool
mkdir build && cd build
cmake ..
make -j$(nproc)
./pointcloudtool
```

The default is to use the local CloudCore from `../cloudanntool`.

## Features

- **3D Rendering**: High-performance OpenGL point cloud rendering
- **Camera Controls**: PCL-style orbit camera (rotate, pan, zoom)
- **Color Modes**: RGB, single color, axis-based coloring (X/Y/Z)
- **Screenshot**: Capture images in PNG/JPG/BMP formats
- **Video Recording**: Record frames with optional ffmpeg compilation
- **ImGui UI**: Intuitive menu-based interface

## Controls

| Action | Control |
|--------|---------|
| Rotate | Left Mouse Drag |
| Pan | Middle Mouse or Shift+Left |
| Zoom | Right Mouse, Ctrl+Left, or Mouse Wheel |
| Reset Camera | R |
| Screenshot | F11 |
| Record | F12 |
| Exit | ESC |

## Menu Bar

- **File**: Load point clouds, exit
- **Settings**: Camera FOV, point size, logging
- **Rendering**: Color modes, capture tools, selection
- **Help**: About, info panel

## Assets

Place your point cloud files in `assets/`:
- `assets/stack.pcd`
- `assets/pointcloud.ply`

Or use files from `../cloudanntool/assets/`
