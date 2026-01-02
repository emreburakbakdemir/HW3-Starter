# HW3 Implementation Plan - Planetary Body Renderer

## Current Status

### ✅ Already Implemented (Starter Code)
1. **Project Structure**: CMake build system configured
2. **Window Management**: GLFW setup with OpenGL 4.4 context
3. **Shader System**: Separate vertex/fragment shader loading
4. **Texture Loading**: STB image integration for PNG/JPG
5. **OBJ Loader**: Wavefront OBJ mesh loading with positions, normals, UVs
6. **Basic Rendering**: Simple triangle rendering with texture mapping
7. **Utility Classes**: `GLState`, `ShaderGL`, `MeshGL`, `TextureGL`

### 🚧 In Progress
1. **Camera System Enhancement**: Added variables to `GLState` for:
   - Camera yaw/pitch angles
   - Mouse interaction (left button pressed, last position)
   - Camera modes (orbit vs FPS)
   - Time control (scale, current time)

## Implementation TODO List

### 1. Camera System (High Priority)
- [ ] **Mouse Controls** (`main.cpp`)
  - Implement `MouseMoveCallback`: Pan camera when left mouse button pressed
  - Implement `MouseButtonCallback`: Track left mouse button state
  - Implement `MouseScrollCallback`: Zoom in/out (adjust camera distance)

- [ ] **Keyboard Controls** (update `KeyboardCallback` in `main.cpp`)
  - WASD movement in FPS mode (forward, left, backward, right)
  - P key: Next camera mode (rollover)
  - O key: Previous camera mode (rollover)
  - L key: Accelerate time (successive presses reverse time)
  - K key: Decelerate time

- [ ] **Camera Mode Implementation**
  - Mode 0-2: Orbit around planets (disable WASD, lock gaze to planet position)
  - Mode 3: FPS mode (enable WASD, free camera movement)
  - Update camera position/gaze based on mode in render loop

### 2. Planetary Body Hierarchy (High Priority)
- [ ] **Scene Structure** (new file: `scene.h/cpp` or in `main.cpp`)
  - Create `Planet` struct with: position, rotation, scale, orbit parameters
  - Hierarchy: Earth → Moon1 → Moon2 (minimum 3 bodies)
  - Add Jupiter and its moon as optional extras

- [ ] **Transformation System**
  - Implement hierarchical transformations (parent-child)
  - Orbit animation based on `currentTime`
  - Self-rotation animation
  - Model matrix calculation for each planet

- [ ] **Time System Integration**
  - Update `currentTime` in render loop using `timeScale`
  - Handle time reversal (negative `timeScale`)

### 3. Lighting Shaders (High Priority)
- [ ] **Planet Vertex Shader** (`planet.vert`)
  - Transform vertices to world space
  - Pass world position to fragment shader
  - Transform normals properly
  - Pass UVs

- [ ] **Basic Planet Fragment Shader** (`planet.frag`)
  - Implement Phong lighting model:
    - Ambient component
    - Diffuse component (N · L)
    - Specular component (N · H)^shininess
  - Directional light (sun)
  - Sample albedo texture
  - Apply lighting to textured color

### 4. Earth Special Effects (Medium Priority)
- [ ] **Enhanced Earth Fragment Shader** (`earth.frag`)
  - **Specular Map**:
    - Sample specular map texture
    - Interpolate between low/high specular power
    - Water (high specular), Land (low specular)

  - **Night Map (City Lights)**:
    - Sample night map texture
    - Blend based on diffuse term (or diffuse + specular)
    - Smooth transition using smoothstep or similar
    - Only visible on dark side

  - **Cloud Layer** (separate draw call):
    - Render slightly larger sphere after Earth
    - Enable alpha blending: `glEnable(GL_BLEND)`, `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`
    - Rotate clouds at different speed than Earth
    - Use cloud alpha map

### 5. Shadow Mapping (High Priority - 30 pts)
- [ ] **Shadow Map Framebuffer**
  - Create FBO with:
    - 2048x2048 depth texture
    - 2048x2048 single-channel 32-bit float color texture
  - Clear color with large value (999999.0f)

- [ ] **Shadow Pass** (render from light's view)
  - Create orthographic projection (tight fit around scene)
  - Create view matrix: forward = sun direction, up = Y axis
  - Render all planets to shadow FBO
  - Shadow vertex shader: output z value to color texture
  - Save light's view-projection matrix

- [ ] **Normal Rendering with Shadows**
  - Pass shadow map and light VP matrix to fragment shader
  - Transform fragment world position to light NDC
  - Convert NDC xy from [-1,1] to [0,1] for texture lookup
  - Compare fragment z with shadow map z
  - If `z + epsilon < shadowMapZ`: lit, else: ambient only
  - Bias epsilon ≈ 0.001 to 0.01 (tune to fix shadow acne)

### 6. Background Elements (Medium Priority)
- [ ] **Stars Background**
  - Create large sphere (radius ≈ 100) centered on camera
  - Apply stars texture
  - Render with **orthographic projection**
  - Disable depth writing: `glDepthMask(GL_FALSE)`
  - Render first (before planets)

- [ ] **Sun Sphere**
  - Create small sphere
  - Position far away: `cameraPos + (-sunDirection) * largeDistance`
  - Render with **orthographic projection**
  - Bright emissive shader (no lighting)

### 7. Shaders to Create
1. `generic.vert` - ✅ Already exists (basic transform)
2. `planet.vert` - New: Planet with world position output
3. `planet.frag` - New: Phong lighting + albedo texture
4. `earth.frag` - New: Phong + specular map + night map
5. `cloud.frag` - New: Cloud with alpha
6. `shadow.vert` - New: Transform to light space
7. `shadow.frag` - New: Output z value
8. `background.vert` - New: Orthographic projection
9. `background.frag` - New: Simple texture mapping
10. `sun.frag` - New: Emissive bright color

### 8. Assets Needed
Available in `working_dir/textures/`:
- ✅ `2k_earth_daymap.jpg` - Earth albedo
- ✅ `2k_earth_clouds_alpha.png` - Earth clouds
- ✅ `2k_earth_nightmap_alpha.png` - City lights
- ✅ `2k_earth_specular_map.png` - Water/land specularity
- ✅ `2k_moon.jpg` - Moon texture
- ✅ `2k_jupiter.jpg` - Jupiter texture
- ✅ `8k_stars_milky_way.jpg` - Background stars

Available in `working_dir/meshes/`:
- ✅ `sphere_2k.obj` - Low poly sphere
- ✅ `sphere_5k.obj` - Medium poly sphere
- ✅ `sphere_20k.obj` - High poly sphere

## Implementation Order (Recommended)

### Phase 1: Get Basic Scene Running
1. Implement camera controls (mouse, keyboard, zoom)
2. Create planet hierarchy (3+ bodies with orbits)
3. Implement basic Phong lighting shader
4. Render planets with diffuse textures

### Phase 2: Advanced Features
5. Implement Earth special effects (specular map, night map, clouds)
6. Implement background stars and sun
7. Add time controls

### Phase 3: Shadow Mapping
8. Create shadow mapping system
9. Integrate shadows into planet rendering
10. Fix shadow acne with bias

### Phase 4: Polish & Testing
11. Test all camera modes
12. Test time reversal for eclipses
13. Tune shadow quality
14. Fix any visual artifacts

## Key Constants & Values

```cpp
// Uniform Locations
#define U_MODEL 0
#define U_VIEW 1
#define U_PROJ 2
#define U_NORMAL 3
#define U_LIGHT_DIR 4
#define U_LIGHT_COLOR 5
#define U_EYE_POS 6
#define U_LIGHT_VP 7  // For shadow mapping

// Texture Bindings
#define T_ALBEDO 0
#define T_SPECULAR 1
#define T_NIGHT 2
#define T_CLOUD 3
#define T_SHADOW_MAP 4

// Camera modes
#define CAM_ORBIT_PLANET0 0
#define CAM_ORBIT_PLANET1 1
#define CAM_ORBIT_PLANET2 2
#define CAM_FPS 3
```

## Testing Checklist
- [ ] FPS camera: WASD movement works
- [ ] FPS camera: Mouse pan works when left button pressed
- [ ] Mouse wheel zoom works in all modes
- [ ] P/O keys cycle through camera modes correctly
- [ ] Orbit modes lock to correct planet
- [ ] WASD disabled in orbit modes
- [ ] L key accelerates time, successive presses reverse
- [ ] K key decelerates time
- [ ] Planets orbit correctly
- [ ] Planets rotate on their axes
- [ ] Hierarchical transformations (moon follows planet)
- [ ] Phong lighting looks correct
- [ ] Earth specular map shows water specularity
- [ ] Night lights appear on dark side smoothly
- [ ] Clouds rotate at different speed
- [ ] Clouds have proper alpha blending
- [ ] Shadows appear during eclipses
- [ ] No severe shadow acne
- [ ] Background stars visible
- [ ] Sun sphere rendered correctly

## Build & Run Commands
```bash
cd /home/bs2022/e2580272/Desktop/graph-hw3/HW3-Starter
cmake -B build .
make -C build
./working_dir/PlanetRenderer
```

## Submission Checklist
- [ ] All source files in `src/`
- [ ] All shaders in `working_dir/shaders/`
- [ ] CMakeLists.txt properly configured
- [ ] Code compiles on inek machine
- [ ] Executable named `PlanetRenderer`
- [ ] Can extract and build with: `tar -xf PlanetRenderer.tar.gz && cmake -B build . && make -C build`
- [ ] All required features implemented
- [ ] No compilation warnings (or minimal)

## Notes
- Due date: January 2, 2026 23:55
- Total points: 100 (General: 40, Earth Effects: 30, Shadows: 30)
- Shadow map resolution: 2048x2048 recommended
- Some shadow artifacts acceptable except shadow acne (Figure 1 in PDF)
- Orthographic projection for background/sun is IMPORTANT
