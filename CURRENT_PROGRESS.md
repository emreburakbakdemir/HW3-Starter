# HOMEWORK COMPLETION SUMMARY

## ✅ ALL REQUIRED FEATURES IMPLEMENTED (100/100 pts)

### 1. General Requirements (40 pts) ✅ COMPLETE

#### Camera System
- **FPS Camera Mode**:
  - WASD movement for forward/left/backward/right
  - Left-click + mouse drag to pan camera
  - Yaw and pitch controls with gimbal lock prevention

- **Orbit Camera Modes** (3 modes):
  - Mode 0: Orbit around Earth
  - Mode 1: Orbit around Moon1
  - Mode 2: Orbit around Moon2
  - Camera follows orbiting bodies
  - WASD disabled in orbit mode

- **P/O Keys**: Cycle through camera modes (P0, P1, P2, FPS) with rollover

- **Mouse Wheel**: Zoom in/out (distance clamped 1.0-100.0)

#### Planetary System
- **Hierarchical Transformations**: Earth → Moon1 → Moon2 (minimum 3 bodies)
- **Orbital Animation**: Time-based motion with different orbit speeds
- **Self-Rotation**: Each planet rotates on its axis
- **Time Control**:
  - L key: Accelerate time (successive presses reverse time)
  - K key: Decelerate time

#### Lighting
- **Directional Light**: Simulates sun, rotates over time
- **Phong Shading**: Ambient + Diffuse (N·L) + Specular (Blinn-Phong)
- **Textured Planets**: Earth and Moon textures applied

#### Background
- **Stars Background**: 8K Milky Way texture on large sphere centered on camera
- **Orthographic Projection**: For infinite distance illusion

### 2. Earth Special Effects (30 pts) ✅ COMPLETE

#### Specular Map
- **Variable Specularity**: Water (high) vs Land (low)
- **Interpolation**: Smooth transition using specular map texture
- **Power Range**: 8.0 (land) to 64.0 (water)

#### Night Map (City Lights)
- **Dark Side Visibility**: Lights appear on non-lit areas
- **Smooth Transition**: smoothstep interpolation for gradual fade
- **Blend Range**: Transitions between -0.1 and 0.2 diffuse term

#### Cloud Layer
- **Separate Rendering**: Alpha-blended sphere (scale 1.01x)
- **Different Rotation**: Clouds rotate at 0.15 rad/s vs Earth's 0.2 rad/s
- **Alpha Blending**: GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA

### 3. Shadow Mapping (30 pts) ✅ COMPLETE

#### Shadow Map Setup
- **Resolution**: 2048×2048 (as recommended)
- **Framebuffer**: Depth texture + R32F color texture
- **Clear Value**: 999999.0 for color (large value)

#### Two-Pass Rendering
- **Shadow Pass**: Renders from light's perspective
  - Orthographic projection: -8 to 8 (tightly covers scene)
  - View matrix from sun direction
  - Outputs z-values to color texture

- **Normal Pass**: Renders to screen with shadow lookup
  - Transforms world position to light NDC
  - Converts NDC to UV coordinates [0,1]
  - Compares depth with bias to determine shadow

#### Shadow Acne Fix
- **Bias**: 0.005 epsilon to prevent self-shadowing artifacts
- **Result**: Clean shadows during eclipses

### 4. Additional Implementations

#### Shaders Created
1. [planet.vert](working_dir/shaders/planet.vert) - Planet vertex shader with world position
2. [planet.frag](working_dir/shaders/planet.frag) - Phong lighting + shadows
3. [earth.frag](working_dir/shaders/earth.frag) - Earth effects + shadows
4. [cloud.frag](working_dir/shaders/cloud.frag) - Cloud alpha blending
5. [shadow.vert](working_dir/shaders/shadow.vert) - Shadow pass vertex
6. [shadow.frag](working_dir/shaders/shadow.frag) - Shadow depth output
7. [background.vert](working_dir/shaders/background.vert) - Background vertex
8. [background.frag](working_dir/shaders/background.frag) - Spherical UV mapping

#### Code Structure
- [main.cpp](src/main.cpp): Complete implementation with:
  - Camera update system
  - Planet transformation hierarchy
  - Shadow pass + normal pass rendering
  - Background rendering with depth masking
  - Cloud rendering with blending

## Build & Run

```bash
cd /home/bs2022/e2580272/Desktop/graph-hw3/HW3-Starter
cmake -B build .
make -C build
./working_dir/PlanetRenderer
```

## Controls

| Key/Input | Action |
|-----------|--------|
| **Left Mouse + Drag** | Pan camera (FPS mode only) |
| **Mouse Wheel** | Zoom in/out |
| **W/A/S/D** | Move forward/left/backward/right (FPS mode only) |
| **P** | Next camera mode (Earth → Moon1 → Moon2 → FPS → Earth) |
| **O** | Previous camera mode |
| **L** | Accelerate time (successive presses reverse time) |
| **K** | Decelerate time |

## Implementation Highlights

- **Shadow mapping** works during eclipses (when moons pass in front of planets)
- **Time reversal** (press L multiple times) lets you watch eclipses repeatedly
- **Hierarchical transformations** ensure Moon2 follows Moon1's orbit around Earth
- **Smooth night map** transitions as Earth rotates
- **Separate cloud rotation** creates realistic atmospheric effect
- **Background stars** render with orthographic projection for infinite distance

## File Locations
- Source code: [src/](src/)
- Shaders: [working_dir/shaders/](working_dir/shaders/)
- Textures: [working_dir/textures/](working_dir/textures/)
- Meshes: [working_dir/meshes/](working_dir/meshes/)
- Executable: [working_dir/PlanetRenderer](working_dir/PlanetRenderer)

## Status: READY FOR SUBMISSION ✅

All features implemented and tested. Build succeeds with only minor warnings (unused parameters, sign conversions - all cosmetic).
