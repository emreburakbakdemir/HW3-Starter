# CENG 477 - Introduction to Computer Graphics
## OpenGL and Programmable Shaders

---

## Table of Contents
1. [Introduction to Shaders](#introduction-to-shaders)
2. [Shader Languages](#shader-languages)
3. [What are we Targeting?](#what-are-we-targeting)
4. [Shader Capabilities](#shader-capabilities)
5. [Parallelism in Shaders](#parallelism-in-shaders)
6. [Vertex Processor](#vertex-processor)
7. [Fragment Processor](#fragment-processor)
8. [Activating a Shader](#activating-a-shader)
9. [Communicating with Shaders](#communicating-with-shaders)
10. [Setting Uniform Values](#setting-uniform-values)
11. [Sample Shaders](#sample-shaders)
12. [Shading Methods](#shading-methods)
13. [Learning GLSL](#learning-glsl)

---

## Introduction to Shaders

### What is a Shader?

A shader is:
- A **programmable logical unit on the GPU** which can replace the "fixed" functionality of OpenGL with user-generated code

### Key Concepts

- By using custom shaders, the user can now override the existing implementation of core **per-vertex** and **per-pixel** behavior
- Some tasks such as **clipping** and **rasterization** will still be performed automatically
- These operations remain fixed in the pipeline even with custom shaders

### Shader Languages

Shaders are written using GPU languages:

**Low-level:**
- Assembly language (rarely used today)

**High-level:**
- **Cg** (Nvidia only)
- **GLSL** (general - works across vendors)

### Execution Model

- Shaders **run on the GPU** while the application runs at the same time on the CPU
- This allows the CPU to perform computations other than graphics
- Shaders can create a variety of visual effects

---

## Shader Gallery I

Shaders enable stunning visual effects including:

**Microsoft XNA Demo:**
- Complex crystalline/glass torus structure with realistic refraction and reflection

**Nvidia Product Demo:**
- Stylized cel-shaded scene with ink outlines (non-photorealistic rendering)
- Wine bottle, plant, glasses on table with artistic rendering

**AMD Product Demo:**
- Photorealistic glass/crystal sphere with multiple smaller spheres
- Advanced light refraction, caustics, and reflections

These examples demonstrate the power and flexibility of programmable shaders.

---

## What are we Targeting?

### Control Points in the Pipeline

OpenGL shaders give the user control over:
- Each **vertex** (corner points of primitives)
- Each **fragment** (potential pixels) interpolated between vertices

### Visual Representation

```
Triangle with texture:
    • Vertex (at corner)
    □ Fragment (inside triangle, interpolated between vertices)
```

The diagram shows a textured triangle where:
- Dark points at corners represent vertices
- The interior contains many fragments that are rasterized and interpolated

### Pipeline Flow

1. **Each vertex** is passed to the **vertex shader**
2. After vertices are processed, polygons are **rasterized**
3. During rasterization, values like **position, color, depth**, and other attributes are **interpolated** across the polygon
4. The **interpolated values** are passed to the **fragment shader**

---

## Shader Capabilities

### Per Vertex Operations

The vertex shader can perform:

- **Vertex transformation**
  - Transform vertex positions from model space to screen space
  
- **Normal transformation and normalization**
  - Transform normals appropriately (using inverse transpose)
  
- **Texture coordinate assignment/transformation**
  - Generate or modify UV coordinates
  
- **Per-vertex lighting**
  - Calculate lighting at each vertex (Gouraud shading)
  
- **Material application**
  - Apply material properties to vertices
  
- And more...

### Per Fragment Operations

The fragment shader can perform:

- **Operations on interpolated values**
  - Work with rasterized data from vertex shader
  
- **Texture access**
  - Sample textures to get color/data
  
- **Texture application**
  - Apply textures to surface
  
- **Fog**
  - Distance-based atmospheric effects
  
- **Color summation**
  - Combine multiple color contributions
  
- **Per-fragment lighting**
  - Calculate lighting at each pixel (Phong shading)
  
- **Scale and bias**
  - Adjust color values
  
- **Color table lookup**
  - Index into color palettes
  
- **Convolution**
  - Apply image filters
  
- And more...

---

## Parallelism in Shaders

### Execution Model

- Shader source codes are read by the CPU program and given to OpenGL for compilation
- They **execute on the GPU at draw time**
  - No other way to execute a shader other than drawing something
  
### Massive Parallelism

- A shader executes **simultaneously** on its data
- All vertices/fragments are processed simultaneously (up to the physical limit allowed by the GPU)

```
Triangle with three vertices (marked with red circles)
Each vertex is processed in parallel by the vertex shader
```

### Key Point: No Explicit Looping

- **We don't loop over vertices** in a vertex shader
- The same applies for the fragment shader as well
- The GPU automatically runs the shader program on all data in parallel
- This is the fundamental paradigm of GPU programming: **data parallelism**

---

## Shading Languages

There are several popular languages for describing shaders:

### HLSL - High Level Shading Language
- **Author:** Microsoft
- **Platform:** DirectX 8+
- Used primarily in Windows/Xbox development

### Cg
- **Author:** Nvidia
- Nvidia-specific shader language
- Similar syntax to HLSL

### GLSL - OpenGL Shading Language
- **Author:** The Khronos Group
  - A self-sponsored group of industry affiliates from various vendors
  - Members include: AMD, Nvidia, Intel, Apple, and others
- **Platform:** OpenGL (cross-platform)
- This is what we'll focus on in this course

**Note:** GLSL is the most portable option and works across different GPU vendors and operating systems.

---

## Vertex Processor

### Inputs and Outputs

```
                    ┌─────────────────┐
                    │ Color           │
                    │ Normal          │
                    │ Position        │──┐
                    │ Texture coord   │  │
                    │ etc...          │  │
                    └─────────────────┘  │
                                         │
┌─────────────────┐                     │      ┌─────────────┐
│ Texture data    │────────┐            │      │ Color       │
└─────────────────┘        │            ├─────>│ Position    │
                           │            │      └─────────────┘
┌─────────────────┐        │   ┌────────┴────┐
│ Modelview matrix│        │   │             │ ┌──────────────────┐
│ Material data   │────────┼──>│   Vertex    │ │ Custom variables │
│ Light data      │        │   │  Processor  │─┤                  │
│ etc...          │        │   │             │ └──────────────────┘
└─────────────────┘        │   └─────────────┘
                           │
┌─────────────────┐        │
│ Custom variables│────────┘
└─────────────────┘
        │
        └─> Per-vertex attributes
```

### What the Vertex Shader Does

- **Vertex shader is executed once for each vertex**

- **Vertex position** is usually transformed using the modelview and projection matrices
  - This is the primary responsibility: transform vertices to clip space

- **Normals** are transformed with the appropriate matrix
  - Typically the inverse transpose of the modelview matrix

- **Texture coordinates** may be generated, passed along, or transformed
  - Can create procedural texture coordinates

- **Lighting computations** may be done for each vertex
  - This results in Gouraud shading

- **Vertex positions** may be modified based on texture values, etc.
  - Enables effects like displacement mapping, vertex animation

---

## Fragment Processor

### Inputs and Outputs

```
┌─────────────────┐
│ Color           │
│ Texture coords  │──┐
│ Fragment coords │  │
│ Front facing    │  │
└─────────────────┘  │
                     │
┌─────────────────┐  │      ┌──────────────────┐
│ Texture data    │──┤      │ Fragment color   │
└─────────────────┘  ├─────>│ Fragment depth   │
                     │      └──────────────────┘
┌─────────────────┐  │
│ Modelview matrix│  │
│ Material        │──┤
│ Lighting        │  │
│ etc...          │  │
└─────────────────┘  │
                     │
┌─────────────────┐  │
│ Custom variables│──┘
└─────────────────┘
```

### What the Fragment Shader Does

- **Fragment shader is executed once for each fragment**

- **Lighting** may be computed at each fragment using interpolated normals
  - Results in Phong shading (higher quality than Gouraud)

- **Texture data** may be fetched from the texture image
  - Using texture coordinates to sample textures

- **Effects** such as fog, blur, etc. may be added
  - Post-processing effects

- **Fragments can be killed**, their depth values can be altered
  - Enables alpha testing, custom depth modifications

- **Various post-processing effects** can be applied
  - Bloom, HDR, color grading, etc.

---

## Activating a Shader

### Step-by-Step Process

#### Step 1: Create a shader program
```c
GLuint programId = glCreateProgram();
```

#### Step 2: Create vertex and fragment shaders
```c
GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
```

#### Step 3: Provide the source code
```c
glShaderSource(vertexShaderId, ...);
glShaderSource(fragmentShaderId, ...);
```

#### Step 4: Compile the shaders
```c
glCompileShader(vertexShaderId);
glCompileShader(fragmentShaderId);
```

#### Step 5: Attach the shaders to the shader program
```c
glAttachShader(programId, vertexShaderId);
glAttachShader(programId, fragmentShaderId);
```

#### Step 6: Link the program
```c
glLinkProgram(programId);
```

#### Step 7: Activate the program
```c
glUseProgram(programId);
```

### Multiple Shader Programs

- **Many shader programs** may be linked but **only one can be active** at a time (i.e., for a single draw)
- It is possible to draw different models using different shader programs

**Example:**
```c
glUseProgram(prg1);
glDrawElements(...); // bunny

glUseProgram(prg2);
glDrawElements(...); // dolphin
```

The bunny uses one shader program while the dolphin uses another.

### Important Note

Once a shader program is activated, **certain operations will no longer be done for you**. It's up to you to replace it!

**You'll have to:**
- Transform each vertex into canonical viewing volume manually
  - i.e., multiply with modelview-projection matrix
  
- Shade each vertex or fragment manually
  - i.e., perform lighting computations

**The installed program replaces all OpenGL fixed functionality** for all renders until you remove it with:
```c
glUseProgram(0);
```

---

## Communicating with Shaders

### Communication Architecture

```
CPU Side                                   GPU Side
┌───────────┐                            ┌───────────┐
│  OpenGL   │────── Uniform ────────────>│  Shader   │
│  Program  │                            │  Program  │
└─────┬─────┘                            └─────┬─────┘
      │                                        │
      │                                   ┌────┴────┐
      │                                   │ Vertex  │  Varying  ┌──────────┐
      │                                   │ Shader  │─────────>│ Fragment │
      │                                   └────┬────┘          │  Shader  │
      │                                        │               └─────┬────┘
      │                                        │                     │
      └────────────────┬───────────────────────┴─────────────────────┘
                       │
               ┌───────┴────────┐
               │ Memory Buffers │
               └────────────────┘
```

### Communication Methods

OpenGL program can send data to shaders through:
- **Uniform variables** - global parameters set from CPU
- **Memory buffers** - attribute data stored in GPU memory

**Data Flow:**
- Vertex shader **can** send data to fragment shader (via varying variables)
- Fragment shader **cannot** send data back to vertex shader
- Data flow is **one-way only** through the pipeline

**Readback:**
- Vertex and fragment shaders can write data to memory buffers
- These buffers can be read back by the OpenGL program (if needed)

### Three Types of Shader Parameters in GLSL

```
                         Attributes
                              │
                              ▼
                    ┌─────────────────┐
                    │     Vertex      │
    Uniform ───────>│    Processor    │
     params         └────────┬────────┘
                             │
                        Varying params
                             │
                             ▼
                    ┌─────────────────┐
    Uniform ───────>│    Fragment     │
     params         │    Processor    │
                    └─────────────────┘
```

#### 1. Uniform Parameters
- Set from the CPU program
- Think of them as **global variables**
- Same value for all vertices/fragments in a draw call
- Examples: transformation matrices, light positions, material properties

#### 2. Attribute Parameters
- Set **per vertex**
- Different value for each vertex
- Examples: position, color, normal, texture coordinates
- Only available in vertex shader

#### 3. Varying Parameters
- Passed from vertex processor to fragment processor
- Automatically interpolated by the rasterizer
- Examples: rasterized position, color, texture coordinates, normals
- Output from vertex shader, input to fragment shader

---

## Shader Gallery II

### Real-Time Realistic Rendering

**Kevin Boulanger (PhD thesis, 2005)**
*"Real-Time Realistic Rendering of Nature Scenes with Dynamic Lighting"*

Two images showing:
- Realistic outdoor scene with house and trees
- Detailed grass rendering with individual blades visible

This demonstrates advanced vegetation rendering and lighting techniques achievable with shaders.

### Car Paint Shader

**Ben Cloward**

Image showing:
- Metallic blue sphere with complex surface reflections
- "Golf ball" dimpled surface pattern
- Realistic car paint appearance with:
  - Base coat color
  - Metallic flakes
  - Clear coat reflections
  - Anisotropic highlights

This showcases multi-layered material rendering possible with fragment shaders.

---

## Sample Shaders

### A Sample Vertex Shader

```glsl
#version 330

// Uniform variables that must be set by the OpenGL program
uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;

// Varying variables that will be rasterized for each fragment
varying vec4 fragWorldPos;
varying vec3 fragWorldNor;

// Attribute variables fetched from buffers
// (gl_Vertex and gl_Normal are built-in attributes)

void main(void)
{
    // Compute the world coordinates of the vertex and its normal.
    // These coordinates will be interpolated during the rasterization
    // stage and the fragment shader will receive the interpolated
    // coordinates.
    
    fragWorldPos = modelingMatrix * gl_Vertex;
    fragWorldNor = inverse(transpose(mat3x3(modelingMatrix))) * gl_Normal;
    
    gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * gl_Vertex;
}
```

**Key Elements:**
- **Version string:** `#version 330` specifies GLSL version 3.30
- **Uniform variables:** Transformation matrices set by CPU program
- **Varying variables:** Data passed to fragment shader (automatically interpolated)
- **Attribute variables:** Per-vertex data from buffers (position, normal)
- **gl_Position:** Special output variable - final vertex position in clip space

### A Sample Fragment Shader (1)

```glsl
#version 330

// All of the following variables could be defined in the OpenGL
// program and passed to this shader as uniform variables. This
// would be necessary if their values could change during runtime.
// However, we will not change them and therefore we define them
// here for simplicity.

vec3 I = vec3(1, 1, 1);              // point light intensity
vec3 Iamb = vec3(0.8, 0.8, 0.8);     // ambient light intensity
vec3 kd = vec3(1, 0.2, 0.2);         // diffuse reflectance coefficient
vec3 ka = vec3(0.3, 0.3, 0.3);       // ambient reflectance coefficient
vec3 ks = vec3(0.8, 0.8, 0.8);       // specular reflectance coefficient
vec3 lightPos = vec3(5, 5, 5);       // light position in world coordinates

uniform vec3 eyePos;

varying vec4 fragWorldPos;
varying vec3 fragWorldNor;
```

**Key Elements:**
- **Local variables:** Lighting parameters defined directly in shader
  - In production code, these would typically be uniforms
- **Uniform variable:** Eye/camera position from OpenGL program
- **Varying variables:** Interpolated data from vertex shader
  - Must match the varying declarations in vertex shader

### A Sample Fragment Shader (2)

```glsl
void main(void)
{
    // Compute lighting. We assume lightPos and eyePos are in world
    // coordinates. fragWorldPos and fragWorldNor are the interpolated
    // coordinates by the rasterizer.
    
    vec3 L = normalize(lightPos - vec3(fragWorldPos));
    vec3 V = normalize(eyePos - vec3(fragWorldPos));
    vec3 H = normalize(L + V);
    vec3 N = normalize(fragWorldNor);
    
    float NdotL = dot(N, L); // for diffuse component
    float NdotH = dot(N, H); // for specular component
    
    vec3 diffuseColor = I * kd * max(0, NdotL);
    vec3 specularColor = I * ks * pow(max(0, NdotH), 100);
    vec3 ambientColor = Iamb * ka;
    
    gl_FragColor = vec4(diffuseColor + specularColor + ambientColor, 1);
}
```

**Key Elements:**
- **Lighting calculation:** Phong reflection model
  - **L:** Light direction vector
  - **V:** View direction vector
  - **H:** Half-angle vector (for specular)
  - **N:** Surface normal (interpolated and normalized)
- **Components computed:**
  - Diffuse: `I * kd * max(0, N·L)`
  - Specular: `I * ks * (max(0, N·H))^100`
  - Ambient: `Iamb * ka`
- **gl_FragColor:** Special output variable - final fragment color

---

## Setting Uniform Values

### Process Overview

Uniform variables must be set by the main program using a three-step process:

#### Step 1: Query the variable location
```c
GLint location = glGetUniformLocation(programId, "variableName");
```

This returns an integer handle to the uniform variable.

#### Step 2: Make the program current
```c
glUseProgram(programId);
```

The program must be active before setting its uniforms.

#### Step 3: Set the variable using the appropriate command

The command depends on the variable type:

```c
// For a single float
glUniform1f(location, v1);

// For a vec3 (float array with 3 elements)
glUniform3fv(location, 1, v2);  // v2 is float v2[3]

// For a mat4 (4x4 matrix)
glUniformMatrix4fv(location, 1, GL_FALSE, v3);
// Uploads one 4x4 matrix in column major order (16 values from v3)
```

### Uniform Variable Properties

**Uniform variables are "sticky":**
- The last set value remains valid until you change it
- **No need to reupload** at each draw (or frame) if the value does not change
- Only update uniforms when their values actually change

### Using GLM Library

GLM library facilitates setting matrix values:

```cpp
float angleRad = (float)(angle / 180.0) * M_PI;

// Compute the modeling matrix
modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, -5.0f));
modelingMatrix = glm::rotate(modelingMatrix, angleRad, glm::vec3(0.0, 1.0, 0.0));

// Set the active program and the values of its uniform variables
glUseProgram(pId);
glUniformMatrix4fv(pId, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
```

**Key Points:**
- GLM provides convenient matrix operations (translate, rotate, scale)
- `glm::value_ptr()` converts GLM matrices to the pointer format OpenGL expects
- `GL_FALSE` means the matrix is in column-major order (OpenGL's default)

---

## Shading Methods

### Phong Shading

The previous example shaders implement a shading scheme known as **Phong shading**.

**Note:** Do not confuse this with the Phong Exponent in ray tracing. These are different concepts with the same name.

#### Basic Idea:
1. **Interpolate per-vertex normal** vectors across the surface
2. **Perform shading per-fragment** using the interpolated normals

#### Process:
- Vertex shader: Transform normals, pass to fragment shader as varying
- Rasterizer: Automatically interpolates normals
- Fragment shader: Receives interpolated normal, computes lighting

#### Quality:
- **Highest quality** of the three methods
- Captures highlights and lighting details accurately
- More computationally expensive (lighting per fragment)

---

### Gouraud Shading

An alternative method is known as **Gouraud shading**.

#### Basic Idea:
1. **Compute shading per-vertex** using the vertex normal
2. **Interpolate the resulting color** across the surface

#### Process:
- Vertex shader: Compute full lighting calculation, output color
- Rasterizer: Automatically interpolates colors
- Fragment shader: Receives interpolated color, uses it directly

#### Quality:
- **Medium quality**
- Faster than Phong (lighting only at vertices)
- May miss highlights that fall between vertices
- Can show banding artifacts on coarse meshes

---

### Flat Shading

An even simpler method is known as **flat shading**.

#### Basic Idea:
1. **Compute shading at a single vertex** of a primitive
2. **Set the entire primitive color** to the resulting value

#### Process:
- Compute lighting once per triangle
- No interpolation across the surface
- Entire triangle has uniform color

#### Quality:
- **Lowest quality**
- Fastest method (minimal computation)
- Faceted appearance - individual triangles clearly visible
- Suitable for low-poly or stylized looks

---

## Shading Comparison

### Visual Comparison: Flat Shading

Image showing a teapot with:
- Clear faceted appearance
- Each triangle is uniformly colored
- Sharp color discontinuities between triangles
- Very obvious geometric structure
- Suitable for low-poly aesthetic or debugging

**Characteristics:**
- Fastest rendering
- Lowest memory bandwidth
- Stylized/retro appearance

---

### Visual Comparison: Gouraud Shading

Image showing a teapot with:
- Smooth color gradients
- Softer appearance than flat shading
- Some loss of highlight detail
- Color transitions between vertices
- Acceptable quality for many applications

**Characteristics:**
- Medium rendering cost
- Good balance of performance and quality
- May miss sharp specular highlights

---

### Visual Comparison: Phong Shading

Image showing a teapot with:
- Smooth, realistic appearance
- Accurate specular highlights
- Proper lighting detail across entire surface
- Highest visual quality
- Professional rendering look

**Characteristics:**
- Highest rendering cost
- Best visual quality
- Standard for modern real-time rendering
- Captures all lighting details accurately

---

## Shader Gallery III

### Various Fragment Shading Results

Three spherical objects demonstrating different shader techniques:

#### Smooth Shading
- Simple gray sphere
- Basic Phong or Gouraud shading
- Shows fundamental lighting model
- Smooth gradation from light to dark

#### Environment Mapping
- Blue metallic sphere
- Reflects surrounding environment
- Uses cube map or sphere map texture
- Shows reflection of scene/skybox
- Creates realistic metallic appearance

#### Bump Mapping
- Colorful textured sphere with apparent surface detail
- Normal map creates illusion of geometric detail
- Orange, green, and yellow coloration
- Surface appears to have physical bumps
- Doesn't actually change geometry, only lighting

These examples show how fragment shaders enable sophisticated visual effects beyond basic shading.

---

## Learning GLSL

### Language Characteristics

**Syntax:**
- GLSL looks like C in terms of syntax
- Familiar control structures (if, for, while)
- Function definitions similar to C

**Special Features:**
- Many special **vector algebra functions** built-in
  - `dot()`, `cross()`, `normalize()`, `length()`, etc.
- Vector and matrix types are first-class citizens
  - `vec2`, `vec3`, `vec4`, `mat2`, `mat3`, `mat4`
- Swizzling: `vec.xyz`, `vec.rgb`, `vec.xxy`, etc.

### Debugging Challenges

**Major limitation:**
- **There is no print function!**
- Cannot output debug text

**Debugging technique:**
- Convert the value of interest to a color
- Visually look at the result on screen
- Example: To debug a normal vector, output it as RGB color

```glsl
// Debug normal vector by visualizing it as color
gl_FragColor = vec4(normal * 0.5 + 0.5, 1.0);
```

### Learning Resources

**Online tutorials:**
- Many good GLSL tutorials available online
- Example: http://www.lighthouse3d.com/tutorials/glsl-tutorial/
- Official OpenGL documentation
- Shader communities and forums

**For this course:**
- We only use a **subset of the language**
- Learning it in entirety is **not required**
- Focus on understanding:
  - Basic vertex transformations
  - Simple lighting models
  - Texture sampling
  - Uniform/attribute/varying variables

### Getting Started

**Recommended approach:**
1. Start with simple shaders (pass-through vertex shader)
2. Gradually add features (transformation, lighting)
3. Experiment with visual effects
4. Use existing examples as templates
5. Debug by visualization (not printing)

**Common patterns:**
- Always transform position in vertex shader
- Always output color in fragment shader
- Use varying variables to pass data between stages
- Use uniforms for global parameters

---

## Summary

### Key Takeaways

**Shaders are:**
- Programmable units on the GPU
- Replace fixed OpenGL functionality
- Written in GLSL (or HLSL/Cg)
- Execute in parallel on GPU

**Two main shader types:**
1. **Vertex Shader**
   - Processes each vertex independently
   - Transforms positions, normals
   - Prepares data for rasterization

2. **Fragment Shader**
   - Processes each fragment (potential pixel)
   - Determines final pixel color
   - Performs per-pixel lighting, texturing

**Communication:**
- **Uniforms:** Global parameters (CPU → Shader)
- **Attributes:** Per-vertex data (Buffer → Vertex Shader)
- **Varyings:** Interpolated data (Vertex Shader → Fragment Shader)

**Shading methods:**
- **Flat:** One color per triangle (fastest, lowest quality)
- **Gouraud:** Interpolate vertex colors (medium)
- **Phong:** Interpolate normals (slowest, highest quality)

**Pipeline control:**
- Shaders replace fixed functionality
- You must implement transformations manually
- Clipping and rasterization remain automatic
- Multiple shader programs can be created and switched

**Development:**
- GLSL syntax similar to C
- No print statements - debug visually
- Use existing examples as starting point
- Only subset needed for most applications

---

## Additional Resources

### Official Documentation
- OpenGL Shading Language Specification
- Khronos OpenGL Wiki

### Tutorials
- Lighthouse3D GLSL Tutorial: http://www.lighthouse3d.com/tutorials/glsl-tutorial/
- LearnOpenGL (Modern OpenGL tutorial)
- OpenGL SuperBible

### Tools
- RenderDoc (Graphics debugging)
- NSight (NVIDIA shader debugging)
- Shader Toy (Online GLSL playground)

### Reference Sites
- OpenGL API Reference
- GLSL Quick Reference Card
- Graphics Programming Forums

---

*CENG 477 - Computer Graphics*  
*Middle East Technical University*
