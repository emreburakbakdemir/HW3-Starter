# CENG 477 - Introduction to Computer Graphics
## Fragment Processing

---

## Table of Contents
1. [Fragment Processing Overview](#fragment-processing-overview)
2. [Fragment Pipeline Stages](#fragment-pipeline-stages)
3. [Pixel Ownership Test](#pixel-ownership-test)
4. [Scissor Test](#scissor-test)
5. [Multi-Sample Operations](#multi-sample-operations)
6. [Stencil Test](#stencil-test)
7. [Depth Buffer Test](#depth-buffer-test)
8. [Z-Fighting](#z-fighting)
9. [Occlusion Culling](#occlusion-culling)
10. [Alpha Blending](#alpha-blending)
11. [sRGB Conversion](#srgb-conversion)
12. [Summary](#summary)

---

## Fragment Processing Overview

The graphics pipeline consists of two main sections:

### Vertex Pipeline
- The previous stages of the pipeline (up to rasterization) is generally known as the **vertex pipeline**

### Rasterization
- Rasterization creates a set of **fragments** that make up the interior region of the primitive

### Fragment Pipeline
- The rest of the pipeline which operates on these fragments is called the **fragment pipeline**
- Fragment pipeline is comprised of several operations
- The end result of fragment processing is the update of corresponding locations in the **framebuffer**

### Pipeline Structure

```
Vertices → Vertex Pipeline → Rasterization → Fragment Pipeline → Framebuffer
                                                                      ├─ Color Buffer
                                                                      ├─ Depth Buffer
                                                                      └─ Stencil Buffer
```

---

## Fragment Pipeline Stages

Fragment pipeline is comprised of many stages. The following is OpenGL's handling of the fragment pipeline. Different renderers may implement a different set of stages.

### OpenGL Fragment Pipeline Flow

```
Fragment (or sample) + Associated Data
    ↓
Pixel Ownership Test
    ↓
Scissor Test
    ↓
Multisample Fragment Operations
    ↓
Stencil Test ← Framebuffer
    ↓
Depth Buffer Test ← Framebuffer
    ↓
Occlusion Query
    ↓
Blending ← Framebuffer
    ↓
sRGB Conversion
    ↓
Dithering
    ↓
Logicop ← Framebuffer
    ↓
To Framebuffer
```

**Reference:** https://www.khronos.org/opengl/wiki/Per-Sample_Processing

---

## Pixel Ownership Test

### Purpose
Does the renderer (e.g., OpenGL) has the ownership of the current framebuffer pixel?

### Window Overlap Example
When multiple windows overlap on screen:
- **Pixels owned by OpenGL**: The visible parts of the OpenGL window that can be rendered to
- **Pixels owned by other windows**: Parts of the OpenGL window that are occluded by other application windows

### Visual Example
- An OpenGL window displaying rendered teapots may be partially covered by a "Display Properties" dialog box
- The renderer needs to determine which pixels it actually owns before attempting to render to them
- Pixels covered by other windows are not owned by OpenGL and cannot be written to

---

## Scissor Test

### Definition
Scissor test is a per-fragment operation that discards fragments outside a certain rectangular region.

### Operation
1. Define a scissor rectangle within the viewport
2. Fragments inside the rectangle pass the test
3. Fragments outside the rectangle are discarded

### Example Process

**Without scissor:**
- Full triangle is rendered

**Scissor rectangle applied:**
- A rectangular region is defined over part of the triangle

**Result:**
- Only the portion of the triangle within the scissor rectangle is rendered
- The rest is clipped/discarded

### OpenGL Implementation
- In OpenGL, `glScissor` command is used for this purpose
- **Note:** This operation is different from clipping
  - Clipping occurs in the vertex pipeline
  - Scissor test occurs in the fragment pipeline

---

## Multi-Sample Operations

### Purpose
Each framebuffer pixel may contain multiple samples. This allows a smoother border of primitives at the cost of extra processing and memory.

### Anti-Aliasing
Multi-sampling is used for anti-aliasing to reduce the "jagged edge" appearance of rendered primitives.

### How It Works
- Instead of a single sample per pixel, multiple sub-pixel samples are taken
- Each pixel is subdivided into multiple sample points
- Coverage is computed for each sample point
- Final pixel color is determined by averaging the covered samples

### Visual Example
A grid showing a diagonal edge passing through pixels:
- Dark pixels: Fully covered by the primitive
- Light pink pixels: Partially covered (different levels of coverage)
- White pixels: Not covered

This results in smoother edges compared to simple on/off determination per pixel.

**Reference:** https://learnopengl.com/Advanced-OpenGL/Anti-Aliasing

---

## Stencil Test

### Purpose
While scissor test can be used to mask out rectangles, stencil test can be used to mask arbitrary fragments.

### Requirements
Requires a different buffer known as the **stencil buffer**.

### Example Operation

**Original Image:**
- A pyramid-like shape with brick texture

**Stencil Buffer:**
- A checkerboard pattern (black and white squares)
- Defines which fragments should be kept or discarded

**Result:**
- Only the portions of the original image that correspond to white areas in the stencil buffer are visible
- Creates a checkerboard masking effect on the rendered object

### Buffer Structure

Typically, depth and stencil buffers are combined to produce a single buffer made of:
- **24-bit depth** information
- **8-bit stencil** information
for each pixel

### Buffer Layout

```
Pixel 0                          Pixel 1
┌────────┬───┬────────┬───┬────────┬───┬───
│   D    │ S │   D    │ S │   D    │ S │ ...
├────────┼───┼────────┼───┼────────┼───┼───
│   D    │ S │   D    │ S │   D    │ S │ ...
├────────┼───┼────────┼───┼────────┼───┼───
│   D    │ S │   D    │ S │   D    │ S │ ...
├────────┼───┼────────┼───┼────────┼───┼───
│   D    │ S │   D    │ S │   D    │ S │ ...
└────────┴───┴────────┴───┴────────┴───┴───
    ⋮
```

Where:
- **D** = Depth (24 bits)
- **S** = Stencil (8 bits)

### Use Case: Shadow Volumes

Stencil buffer and stencil test can also be used to implement one type of shadowing algorithms (covered later in the course).

**Example:** Doom 3 used stencil buffer shadows extensively for real-time dynamic shadow rendering.

**Source:** research.ncl.ac.uk

---

## Depth Buffer Test

### The Problem
Among the fragment operations, the **depth buffer test** is very important to render primitives in correct order.

### Painter's Algorithm

Without depth buffer, the programmer must ensure to render primitives in a **back to front order**, known as the painter's algorithm.

**Visual Example:**
```
Step 1: Draw sky (background)
Step 2: Draw mountains (middle)
Step 3: Draw trees (foreground)
```

Each layer paints over the previous one, like a painter creating a landscape.

### Binary Space Partitioning (BSP) Trees

BSP trees may be used to determine rendering order:

**2D Example:**

```
Scene with 5 colored circles and dividing planes:
    
    1 (red circle)
    2 (yellow circle)
    C (camera position)
    3 (purple circle)
    4 (cyan circle)
    5 (green circle)
```

**Corresponding BSP Tree:**
```
           Root
          /    \
         /      \
    [Left]    [Right]
     / \        / \
    1   2      /   3
              /
          [Node]
           / \
          5   4
```

The tree structure determines the order in which objects should be rendered based on camera position.

### Limitations of Painter's Algorithm

However, BSP trees are costly to generate and may require splitting primitives due to **impossible ordering cases**:

**Cyclic Overlap Problem:**
Three polygons (red, green, blue) arranged in a way where:
- Red is in front of green
- Green is in front of blue  
- Blue is in front of red

This creates a cyclic dependency with no valid rendering order without splitting primitives.

### Historical Context

When memory was a very valuable resource, such algorithms were implemented:

- **Quake 3** was one of the main games that used painter's algorithm using BSP trees
- Each game level was stored as a huge BSP tree

**Read more at:** https://www.bluesnews.com/abrash/chap64.shtml

---

## Depth Buffer Test (Z-Buffer)

### Main Idea

1. At each pixel, keep track of the distance to the closest fragment that has been drawn in a separate buffer
2. Discard fragments that are further away than that distance
3. Otherwise, **draw** the fragment and **update** the z-buffer value with the z value of that fragment

### Requirements

- Requires an extra memory region, called the **depth buffer**, to implement this solution
- At the beginning of every frame, the depth buffer is reset to infinity (1.0f in practice if the depth range is [0.0f, 1.0f])
- Depth buffer is also known as **z-buffer**

### Example Operation

Consider two overlapping triangles:

**Initial State:**
```
Depth buffer filled with ∞ (or 1.0):
∞ ∞ ∞ ∞ ∞ ∞ ∞ ∞
∞ ∞ ∞ ∞ ∞ ∞ ∞ ∞
∞ ∞ ∞ ∞ ∞ ∞ ∞ ∞
...
```

**After First Triangle:**
Z-values of the first triangle are written:
```
5 5 5 5 5 5 5 ∞
5 5 5 5 5 5 ∞ ∞
5 5 5 5 5 ∞ ∞ ∞
5 5 5 5 ∞ ∞ ∞ ∞
5 5 5 ∞ ∞ ∞ ∞ ∞
5 5 ∞ ∞ ∞ ∞ ∞ ∞
5 ∞ ∞ ∞ ∞ ∞ ∞ ∞
∞ ∞ ∞ ∞ ∞ ∞ ∞ ∞
```

**Z-values of Second Triangle:**
```
5 5 5 5 5 5 5 ∞
5 5 5 5 5 5 ∞ ∞
5 5 5 5 5 ∞ ∞ ∞
5 5 5 5 ∞ ∞ ∞ ∞
5 5 5 ∞ ∞ ∞ ∞ ∞
5 5 ∞ ∞ ∞ ∞ ∞ ∞
5 ∞ ∞ ∞ ∞ ∞ ∞ ∞
∞ ∞ ∞ ∞ ∞ ∞ ∞ ∞

      7
    6 7
  5 6 7
4 5 6 7
3 4 5 6 7
2 3 4 5 6 7
∞ ∞ ∞ ∞ ∞ ∞ ∞ ∞
```

**Resulting Depth Buffer:**
After comparison, the minimum depth values are kept:
```
5 5 5 5 5 5 5 ∞
5 5 5 5 5 5 ∞ ∞
5 5 5 5 5 ∞ ∞ ∞
5 5 5 5 ∞ ∞ ∞ ∞
4 5 5 7 ∞ ∞ ∞ ∞
3 4 5 6 7 ∞ ∞ ∞
2 3 4 5 6 7 ∞ ∞
∞ ∞ ∞ ∞ ∞ ∞ ∞ ∞
```

Only fragments with depth values less than the current buffer value are rendered and update the buffer.

**Source:** wikipedia.com

---

## Depth Range

### Configuration

The range of values written to the depth buffer can generally be controlled by the programmer.

### OpenGL Commands

- In OpenGL, the command `glDepthRange(zMin, zMax)` is used
- The default depth range is **[0, 1]**

### Transformation

- The z-value in the canonical viewing volume (CVV), which is in range **[-1, 1]**, is scaled to this range during the viewport transform
- `glDepthRange` is to the z-values what `glViewport(x, y, width, height)` is to the x and y-values

---

## Z-Fighting

### The Problem

Remember that the z-values get compressed to [0, 1] range from the [-n, -f] range after projection and viewport transforms.

### Depth Compression Visualization

**Example 1: n = 10, f = 50**

Graph showing z′ (normalized depth) vs z (original depth):
- X-axis: z values from -10 to -50
- Y-axis: z′ values from 0 to 1
- The curve shows rapid increase near the near plane, then flattening

**Example 2: n = 10, f = 200**

Graph showing z′ vs z:
- X-axis: z values from -20 to -200
- Y-axis: z′ values from 0 to 1
- Even more extreme compression with larger depth range
- Most of the [0, 1] range is used for near objects
- Far objects are heavily compressed

### Limited Precision Effects

With a limited precision depth buffer, fragments that are close in depth may get mapped to the same z-value:

**Quantized Depth Values (with limited precision):**

The curve shows discrete "steps" rather than smooth gradients, especially in the far range:
- Near objects get good depth resolution
- Far objects get poor depth resolution
- Multiple different z values map to the same quantized z′ value

### The Z-Fighting Artifact

The compression is more severe with larger depth range. This may cause a problem known as **z-fighting**:

- Objects with originally different (but close) z-values get mapped to the same final z-value (due to limited precision)
- This makes it impossible to distinguish which one is in front and which one is behind
- Results in flickering artifacts as different fragments "fight" for the same pixel

**Visual Example:**
Two overlapping colored rectangles (red and green) showing:
- Left image: Flickering/noise pattern where depths are too close
- Right image: Different frame showing different "winner" in the depth test

Both rectangles are at similar depths, causing the renderer to inconsistently determine which should be visible.

### Real-World Example

Screenshot showing z-fighting artifacts:
- A 3D scene with a building/structure
- Ground plane showing characteristic "shimmering" or "crawling" pixels
- The ground appears to flicker between two textures or surfaces that are at nearly identical depths

**Source:** http://wiki.reflexfiles.com/

### Solutions to Z-Fighting

To avoid z-fighting:

1. **Minimize depth range**
   - The separation of the far and near planes should be kept as small as possible
   - This keeps the compression less severe
   - Use the tightest possible near/far planes for your scene

2. **Use floating point depth buffer**
   - Unavailable in older hardware
   - Supported in all modern GPUs
   - Provides much better precision, especially for far objects

3. **Polygon offset**
   - The command `glPolygonOffset` can be used to push and pull polygons slightly
   - Helps avoid z-fighting between coplanar or nearly coplanar surfaces
   - Useful for rendering decals, outlines, or shadows on surfaces

---

## Occlusion Culling

### Definition

The removal of geometry that is **within** the view volume but is **occluded** by other geometry closer to the camera.

### Visual Example

```
        Bunny (occluded)
           ↓
    [Triangle]
    (occluder)      w → • eye
                        (camera)
```

The bunny is within the view frustum but cannot be seen because it's blocked by the blue triangle in front of it.

### Purpose

Avoid rendering objects that won't be visible anyway, improving performance.

### OpenGL Support

OpenGL supports **occlusion queries** to assist the user in occlusion culling:
- By a fast rendering pass, it counts how many pixels of the tested object will be rendered
- This is commonly used in games to skip rendering expensive geometry that won't be visible

### Implementation in OpenGL

The typical workflow for occlusion queries:

1. **Create a query**

2. **Disable rendering to screen** (set the color mask of all channels to False)

3. **Disable writing to depth buffer** (just test against, but don't update, the depth buffer)

4. **Issue query begin** (which resets the counter of visible pixels)

5. **"Render" the object's bounding box**
   - It'll only do depth testing
   - Pixels that pass depth testing will not be rendered on-screen because rendering and depth writing were disabled

6. **End query** (stop counting visible pixels)

7. **Enable rendering to screen**

8. **Enable depth writing** (if required)

9. **Get query result** (the number of "visible" pixels)

10. **If the number of visible pixels is greater than 0** (or some threshold):
    - Render the complete object

### Benefits

- Avoids expensive rendering of fully occluded objects
- Bounding box test is much faster than rendering the full object
- Can use hierarchical queries for complex scenes

**For details see:** http://developer.nvidia.com/GPUGems/gpugems_ch29.html

---

## Alpha Blending

### Purpose

Alpha blending is another fragment operation in which new objects can be blended with the existing contents of the color buffer for a variety of effects.

### Use Cases

- Transparency
- Glass effects
- Translucent materials
- Fog
- Volumetric effects

### Visual Example

**Without blending:**
- Red sphere fully opaque
- Blue cube fully opaque
- Clean, sharp edges where they overlap

**With blending:**
- Red sphere semi-transparent
- Can see the blue cube through the sphere
- Blended colors where they overlap
- More realistic transparent/translucent appearance

### Implementation

Blending combines:
- **Source color**: The new fragment being rendered
- **Destination color**: The existing color in the framebuffer

Using a blend function (e.g., `src_alpha * src_color + (1 - src_alpha) * dst_color`)

**Source:** Computer Desktop Encyclopedia, Reprinted with permission. © 1998 Intergraph Computer Systems

---

## sRGB Conversion

### Purpose

The fragment values are converted to the **sRGB color space** before being written to the framebuffer. This serves the purpose of **gamma correction**.

### Why Gamma Correction?

- Monitors display colors non-linearly
- sRGB conversion ensures colors appear as intended
- Corrects for monitor gamma response
- Improves color accuracy and perception

### Visual Example

**Without Gamma Correction:**
- Scene appears darker
- Less detail in shadows
- Colors look washed out
- Linear color values don't match perception

**With Gamma Correction:**
- Scene appears brighter and more natural
- Better detail in all lighting conditions
- Colors are more vibrant and accurate
- Matches human visual perception

The example shows a character in a fantasy landscape at sunset. The gamma-corrected version shows improved brightness, contrast, and overall visual quality.

**Source:** BLOG.WOLFIRE.COM

---

## Summary

### Complete Graphics Pipeline

At the end of the pipeline, input vertices with connectivity information end up populating certain regions of the framebuffer.

```
Vertices
   ↓
Vertex Pipeline
   ↓
Rasterization
   ↓
Fragment Pipeline
   ↓
Framebuffer
├─ Color Buffer
├─ Depth Buffer
└─ Stencil Buffer
```

### Implementation Options

This pipeline can be implemented on:
- **Software (CPU)**: Full control, slower
- **Hardware (GPU)**: Highly optimized, much faster
- **Both (CPU + GPU)**: Hybrid approach, flexibility + performance

### Key Takeaways

1. **Fragment Pipeline** processes rasterized fragments before writing to framebuffer
2. **Multiple buffers** work together: color, depth, and stencil
3. **Depth testing** solves visibility automatically (no need for manual sorting)
4. **Z-fighting** occurs with limited depth precision - minimize depth range
5. **Stencil buffer** enables complex masking effects
6. **Occlusion culling** improves performance by skipping hidden geometry
7. **Alpha blending** enables transparency and translucency effects
8. **Gamma correction** ensures accurate color display

---

## Additional Resources

- OpenGL Per-Sample Processing: https://www.khronos.org/opengl/wiki/Per-Sample_Processing
- LearnOpenGL Anti-Aliasing: https://learnopengl.com/Advanced-OpenGL/Anti-Aliasing
- Quake 3 BSP: https://www.bluesnews.com/abrash/chap64.shtml
- GPU Gems Occlusion Culling: http://developer.nvidia.com/GPUGems/gpugems_ch29.html

---

*CENG 477 - Computer Graphics*  
*Middle East Technical University*
