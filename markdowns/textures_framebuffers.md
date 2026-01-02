# CENG 477 - Introduction to Computer Graphics
## Textures and Framebuffers

---

## Until Now

- We assumed that objects have **fixed R, G, B reflectance values**
- Surface **orientation** and light **direction** determined the shading of objects
- Our best image so far is something like:

![Simple shaded spheres example]

---

## Texture Mapping

### Goal
- Increase visual realism by using **images** to simulate reflectance characteristics of objects
- A cheap and effective way to **spatially vary** surface reflectance

### Concept
- Image (texture) sizes and object sizes may vary
- We need a uniform way so that any object can be mapped by any texture

---

## Texture Mapping Process

### Step 1: Associate (u, v) Coordinate System
- Associate a (u, v) coordinate system with the texture image where (u, v) ∈ [0,1]×[0,1]

```
u →
v ↓
[Texture Image]
```

---

### Step 2: Parameterize the Surface

**Example: Sphere**

Assuming that the center is at (0, 0, 0):

```
x = r·sin(Θ)·cos(φ)
y = r·cos(Θ)
z = r·sin(Θ)·sin(φ)

Θ = arccos(y / r)
φ = arctan(z / x)
```

In practice, φ = atan2(z, x)

- atan2(±0, x) returns ±pi for x < 0
- atan2(±0, x) returns ±0 for x > 0

(Θ, φ) ∈ [0, π]×[-π, π]

---

### Step 3: Map Texture Coordinates

Assume you want to wrap the image such that:

```
u = (-φ + π) / (2π)
v = Θ / π
```

This mapping wraps the texture around the sphere appropriately.

---

### Step 4: Find Texture Image Coordinate

Find the texture image coordinate (i, j) at the given (u, v) coordinate:

```
i = u·nₓ
j = v·nᵧ

where:
nₓ = texture image width
nᵧ = texture image height
```

**Note:** i, j can be fractional!

---

### Step 5: Choose Texel Color Using Interpolation

#### Nearest Neighbor
- Fetch texel at nearest coordinate (faster)

```
Color(x, y, z) = fetch(round(i, j))
```

#### Bilinear Interpolation
- Average four closest neighbors (smoother)

```
p = floor(i)
q = floor(j)
dx = i - p
dy = j - q

Color(x, y, z) = fetch(p, q)·(1 - dx)·(1 - dy) +
                 fetch(p+1, q)·(dx)·(1 - dy) +
                 fetch(p, q+1)·(1 - dx)·(dy) +
                 fetch(p+1, q+1)·(dx)·(dy)
```

---

## Nearest Neighbor vs Bilinear Interpolation

### Visual Comparison
- **Nearest-neighbor:** Produces blocky, pixelated results
- **Bilinear:** Produces smoother, more continuous results

The difference is especially noticeable on curved surfaces and when textures are magnified.

---

## Texture Color Usage

### Once we have the texture color, what to do with it?

Several options possible:
- Replace shading color
- Blend with shading color
- Replace kd component, etc.

### Example: Replace the kd Component

```
Lₒᵈ(x, wo) = kd·cos(θ')·Eᵢ(x, wᵢ)

where:
cos(θ') = max(0, wᵢ·n)
kd = textureColor / 255
```

---

## Texture Color Replacement Comparison

**Replacing kd typically improves realism over replacing the entire diffuse color**

- **Diffuse color replaced:** Loses lighting information, appears flat
- **kd component replaced:** Maintains proper lighting and shading

---

## Texture Mapping Triangles

### Basic Approach
1. First, we must associate (u, v) coordinates for each vertex
2. The (u, v) coordinates inside the triangle can be found using **barycentric coordinates**

### Barycentric Interpolation

Position of point p at barycentric coordinates (β, γ):
```
p(β, γ) = a + β(b - a) + γ(c - a)
```

Texture coordinates can be interpolated similarly:
```
u(β, γ) = uₐ + β(uᵦ - uₐ) + γ(uᶜ - uₐ)
v(β, γ) = vₐ + β(vᵦ - vₐ) + γ(vᶜ - vₐ)
```

---

## Texture Mapping Triangles - Example

### Rectangle Made of Two Triangles
- Triangle 1: v₃, v₁, v₂
- Triangle 2: v₁, v₃, v₀

### Texture Coordinate Assignment
- v₃, v₁, v₂: (1, 0), (0, 1), (1, 1)
- v₁, v₃, v₀: (0, 1), (1, 0), (0, 0)

This way, the texture image gets stretched to match the triangles' positions.

---

## Tiling

### Problem
For large surfaces, stretching may excessively distort the texture image making it unrealistic.

### Solution: Tiling
- **Tiling** is the process of repeating a texture instead of stretching
- Multiple copies of the texture fill the surface
- Tiling may also look unrealistic if repetition is clear

### Implementation
To support tiling, texture coordinates are not limited to [0, 1] range:

```
u = ubarycentric - floor(ubarycentric)
v = vbarycentric - floor(vbarycentric)
```

Example: Coordinates (0, 0), (6, 0), (6, 2), (0, 2) will tile the texture 6×2 times.

---

## Texture Atlases

### Motivation
- Most objects in CG will have a texture applied to it
- This requires using multiple textures, sometimes thousands or more, for a moderately complex environment
- Many times text is also retrieved from texture images

### Solution
Instead of using multiple textures, a **texture atlas** combines textures of many objects into a single image.

### Benefits
- Reduces texture switching overhead
- Improves rendering performance
- Simplifies texture management

---

## Texture Mapping using OpenGL

### Step 1: Generate Names
Generate a name for your texture and sampler (these are just handles):

```cpp
GLuint mySampler, myTexture;

glGenSamplers(1, &mySampler);
glGenTextures(1, &myTexture);
```

---

### Step 2: Set Sampling Parameters

```cpp
glSamplerParameteri(mySampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glSamplerParameteri(mySampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glSamplerParameteri(mySampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glSamplerParameteri(mySampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
```

**Parameters:**
- **GL_TEXTURE_MIN_FILTER:** What to do if your texture needs to be minified
- **GL_TEXTURE_MAG_FILTER:** What to do if your texture needs to be magnified
- **GL_TEXTURE_WRAP_S/T:** What to do if you make out-of-bounds access

---

### Step 3: Bind Sampler to Texture Unit

Bind mySampler to unit 0 so that texture fetches from unit 0 will be done according to the above sampling properties:

```cpp
// Bind mySampler to unit 0
glBindSampler(0, mySampler);
```

---

### Step 4: Activate Unit and Bind Texture

Activate the desired unit and bind your texture to the proper target of that unit:

```cpp
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, myTexture);
```

---

### Step 5: Upload Image Data

Read the texture image from an image file (.jpg, .png, etc.) into a one dimensional array and tell OpenGL about the address of this array:

```cpp
// When reading a texture image, do not assume that it is aligned
// to any boundary larger than a single byte
glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

// Upload the image to the texture
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
             width, height, 0, GL_RGB,
             GL_UNSIGNED_BYTE, imageID);
```

**imageID** = Pointer to the first byte of your image

---

### Texture Unit Architecture

At this point we have the following picture:

```
[Sampler] → [Texture Unit 0] → [1D]
                              → [2D] → [Image]
                              → [3D]

            [Texture Unit 1] (Not activated)
            [...]
```

---

### Step 6: Provide UV Coordinates for Each Vertex

**In immediate mode:**
- Use `glTexCoord2f`

**If using vertex arrays:**
- Must provide the texture coordinates in an array (as we did for vertex positions, colors, etc.)
- Use `glTexCoordPointer` or `glVertexAttribPointer`
- As before, this array can be on the system memory or uploaded to GPU memory (VBOs)

```cpp
glTexCoordPointer(size, type, stride, pointer)
```

**Parameters:**
- **size:** Number of coordinates per texture vertex
- **type:** Type of each coordinate
- **stride:** Byte offset between consecutive texture vertices
- **pointer:** Pointer to texture vertex coordinate data

---

### Step 7: Vertex Shader

In the vertex shader, pass along these texture coordinates to the rasterizer:

```glsl
void main(void)
{
    gl_FrontColor = gl_Color;        // vertex color defined by the programmer
    gl_TexCoord[0] = gl_MultiTexCoord0; // pass along to the rasterizer
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
```

**Data Sources:**
- `gl_Color` comes from vertex array data provided by `glColorPointer`
- `gl_MultiTexCoord0` comes from vertex array data provided by `glTexCoordPointer`
- `gl_Vertex` comes from vertex array data provided by `glVertexPointer`
- `gl_TexCoord[0]` is a built-in varying variable which will be automatically interpolated

---

### Step 8: Fragment Shader

In the fragment shader, fetch from the texture image using a suitable sampling method:

```glsl
uniform sampler2D mySampler;

void main(void)
{
    // get the color from the texture
    gl_FragColor = texture2D(mySampler, gl_TexCoord[0].st);
}
```

**mySampler** represents the texture unit index. If its value is zero it will fetch from texture unit 0. Its value is given such as:

```cpp
glUniform1i(mySamplerLoc, 0)
```

---

### Sampler Connection Diagram

```
[Sampler] ──→ [Texture Unit 0] → [1D]
    ↑                            → [2D] → [Image]
    │                            → [3D]
    │
mySampler    [Texture Unit 1] (Not activated)
             [...]
```

If we call `glUniform1i(mySamplerLoc, 1)`, mySampler would point to Texture Unit 1 instead.

---

### Blending Texture with Fragment Color

What to do once we have the texture color? We have several options.

For instance to blend the texture color with the color of the fragment:

```glsl
void main(void)
{
    // get the color from the texture
    gl_FragColor = alpha * gl_Color +
                   (1 - alpha) * texture2D(mySampler, gl_TexCoord[0].st);
}
```

**alpha** = User-defined interpolation parameter. Can be a uniform. Can also be read from the texture if it has an alpha channel.

---

## Sampling

### Definition
**Sampling** is the process of fetching the value from a texture image given its texture coordinate.

### Methods
- **Nearest-neighbor** and **bilinear interpolation** are two examples
- Need to tell OpenGL about the type of sampling we want

### Previously Set Sampling Parameters
```cpp
glSamplerParameteri(mySampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glSamplerParameteri(mySampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glSamplerParameteri(mySampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glSamplerParameteri(mySampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
```

### Important Concept
There is another important concept called **mipmapping**.

---

## Mipmapping

### Problem
Mipmapping deals with cases when the resolution of the primitive is different from the resolution of the texture (which often is the case).

### Three Cases

1. **Polygon same size as texture**
   - Map as usual

2. **Polygon larger than texture**
   - Texture needs to be **magnified**

3. **Polygon smaller than texture**
   - Texture needs to be **minified**

---

## Minification Problem

### Issue
**Minification:** A change of 1 pixel in image space causes a change of >1 pixel in texture space.

### Extreme Case
1 pixel change in image space corresponds to as many pixels as the width of the texture in texture space.

### Requirement
For accurate mapping, this requires computing the **average value of the entire row** – otherwise **aliasing artifacts** will occur.

---

## Aliasing

**Aliasing artifacts** produce disturbing visual patterns, especially noticeable in:
- Static images with high-frequency patterns
- Animated scenes (temporal aliasing)

The checkerboard pattern on a sphere or teapot is a classic example of aliasing artifacts when using nearest-neighbor sampling without mipmapping.

---

## Fixing Aliasing

### Problem Analysis
- Aliasing artifacts are even more disturbing if **animation** is present in the scene
- Aliasing artifacts occur as we are sampling a **high frequency texture** at very **low frequencies**
- Our sample does not faithfully represent the real signal
  - It adopts a different persona – thus called **aliasing**

### Why Not Just Sample More?
- Sampling at a higher rate is not an option as samples are determined by our fragments

### Solution
- **Reduce the frequency** of the original signal by low-pass filtering (blurring)

### Problem with Solution
- **Expensive** to continuously filter in runtime

---

## Mipmapping Solution

### Approach
**Pre-filter** images to create smaller resolution versions during initialization (or offline):

```
Original:  [Full Resolution]
Level 1:   [1/2 Resolution]
Level 2:   [1/4 Resolution]
Level 3:   [1/8 Resolution]
...
```

Then sample from the appropriate resolution in runtime.

### Memory Requirement
How much memory does a mipmap chain require?

```
A + A/4 + A/16 + A/64 + ... = 4A/3
```

**Only 33% more memory than the original texture!**

---

## OpenGL Mipmap Support

### Option 1: Create Offline
Mipmap levels can be created offline and then given to OpenGL. This allows custom filtering for each level:

```cpp
for (int level = 0; level < numLevels; ++level)
{
    glTexImage2D(GL_TEXTURE_2D, level, GL_RGB,
                 width, height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, image[level]);
}
```

### Option 2: Automatic Generation
Alternatively, we can ask OpenGL to automatically generate mipmap levels for us:

```cpp
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
             width, height, 0,
             GL_RGB, GL_UNSIGNED_BYTE, imageID);

glGenerateMipmap(GL_TEXTURE_2D);
```

---

### Using Mipmapping

To use mipmapping, we must set the sampler parameters correctly:

```cpp
// Without mipmapping (previous example)
glSamplerParameteri(mySampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

// With mipmapping options
glSamplerParameteri(mySampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
glSamplerParameteri(mySampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glSamplerParameteri(mySampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glSamplerParameteri(mySampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
```

---

## Filtering Results Comparison

### Visual Quality
1. **Nearest:** Most aliasing, sharp but with artifacts
2. **Bilinear:** Smoother than nearest, but still has aliasing when minified
3. **Trilinear (mipmapping):** Smoothest, least aliasing

### On Spheres
- **Nearest vs. Bilinear:** Bilinear removes pixelation
- **Nearest vs. Trilinear:** Trilinear eliminates the shimmering and moiré patterns

### On Perspective Views
The difference is most noticeable on surfaces that recede into the distance, where:
- Nearest shows severe aliasing
- Bilinear shows some improvement
- Trilinear provides smooth, artifact-free rendering

---

## Framebuffer Objects (FBOs)

### Motivation
- Until now, we always rendered to the screen
- But many visual effects require rendering an image to an **off-screen buffer** and processing it before displaying it

### Examples of Effects
- **Motion Blur:** Accumulate multiple frames with motion
- **Depth of Field:** Blur based on depth information
- **Post-processing effects:** Bloom, HDR, color grading, etc.

---

## Framebuffer Objects - Setup

### Step 1: Generate and Bind FBO

```cpp
GLuint gFBOId;
glGenFramebuffers(1, &gFBOId);
glBindFramebuffer(GL_FRAMEBUFFER, gFBOId);
```

---

### Step 2: Allocate Color Buffer

Next we must allocate memory for its color and (optionally) depth buffers. These memories are allocated as textures.

**For color buffer:**

```cpp
glGenTextures(1, &gColorTextureId);
glBindTexture(GL_TEXTURE_2D, gColorTextureId);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gFBOWidth, gFBOHeight, 0,
             GL_RGB, GL_UNSIGNED_BYTE, 0);
```

---

### Step 2: Allocate Depth Buffer

**For depth buffer:**

```cpp
glGenTextures(1, &gDepthTextureId);
glBindTexture(GL_TEXTURE_2D, gDepthTextureId);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, gFBOWidth, gFBOHeight, 0,
             GL_DEPTH_COMPONENT, GL_FLOAT, 0);
```

---

### Step 3: Attach Textures to FBO

We must attach these textures to the FBO:

```cpp
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                       GL_TEXTURE_2D, gColorTextureId, 0);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                       GL_TEXTURE_2D, gDepthTextureId, 0);
```

---

### Step 4: Check FBO Completeness

Make sure that FBO is complete:

```cpp
GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
assert(status == GL_FRAMEBUFFER_COMPLETE);
```

---

## Using Framebuffer Objects

### Rendering to FBO
When we render while this FBO is bound, the attached textures' contents will be updated.

### Important: Set Viewport
Before rendering make sure that you set your viewport to match the resolution of this framebuffer:

```cpp
glViewport(0, 0, gFBOWidth, gFBOHeight)
```

**Why?** The size of the window (for which the viewport was originally set) can be different from the size of our FBO.

---

### Switching Back to Default Framebuffer

Once you make the FBO rendering pass, you can detach your textures and switch back to the default framebuffer:

```cpp
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                       GL_TEXTURE_2D, 0, 0);

glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                       GL_TEXTURE_2D, 0, 0);

glBindFramebuffer(GL_FRAMEBUFFER, 0);
```

### Usage
Now you can use these textures as **source textures** for various special effects.

One such usage is for generating **shadows** as we will learn next week.

---

## Summary

### Key Takeaways

1. **Texture mapping** is critical for improving visual quality of rendered images

2. **Texture coordinates** can be computed automatically for parametric surfaces like spheres

3. They can be provided by the user for triangular meshes
   - UV unwrapping is a partially automated technique for solving this problem for complex meshes (not covered in this class)

4. **Mipmapping** is a technique for reducing aliasing with small extra memory overhead

5. **Textures** can be both source or target (framebuffer objects)
   - Not at the same pass, though
   - Render into as target in the first pass, read them as source in the second pass

---

## End of Lecture

**Course:** CENG 477 - Introduction to Computer Graphics  
**Institution:** Middle East Technical University (ODTÜ/METU)  
**Topic:** Textures and Framebuffers
