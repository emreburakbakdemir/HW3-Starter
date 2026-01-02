# CENG477 - OpenGL Recitation

**Presenter:** Bora Yalçiner  
**Date:** December 15, 2025  
**Institution:** Middle East Technical University (METU)

---

## Table of Contents

1. [OpenGL Introduction](#opengl-introduction)
   - Setup
2. [Graphics Pipeline](#graphics-pipeline)
   - Primitive Assembly
   - Vertex Shader
   - Rasterizer
   - Fragment (Pixel) Shader
   - Write Back
3. [OpenGL State](#opengl-state)
   - Shaders
   - Buffers
   - Shader Uniforms
   - Textures
   - Framebuffer Objects
4. [Useful Links](#useful-links)

---

## OpenGL Introduction

### OpenGL Libraries

**OpenGL is not a library.**

- An API between the GPU/OS and the program
- OS provides the entry API through some DLL
- Application needs to query **all the functions**, by itself
  - **Windows:** `wglGetProcAddress`
  - **Linux:** `glXGetProcAddressARB`
- We use shim libraries like **glew, glad** etc. for this purpose

**Window & OpenGL context is created by OS**
- **Windows:** `CreateWindowEx`, `wglCreateContext`, ...
- **Linux:** `XCreateWindow`, `glXCreateContext` ... (for X11, Wayland is probably different)
- We use **GLFW** for platform independence

### Setup

**Window Creation**
- GLFW (Platform agnostic window creation, I-O)
- GLUT (legacy library for the same purpose)

**Platform Specific:**
- **Windows:** opengl32.lib
- **Linux:** X11.so, Xrandr.so

**Modern OpenGL do not have any matrix/vector operations**
- You calculate and send these to the shaders
- For matrix operations on the CPU side, we use **glm**

---

## Graphics Pipeline

### Pipeline Overview

```
Vertex Processing
    ↓
[Vertex Shader] → [Tess. Cont. Shader] → [Tess. Eval. Shader] → [Geom Shader]
    ↓
[Rasterizer]
    ↓
[Fragment Shader]
    ↓
[Framebuffer Write]

(Compute Shader - separate path)
```

### Graphics Pipeline - Simplified

| **Primitive Assembly** | **Vertex Shader** | **Rasterizer** | **Fragment Shader** | **Framebuffer Write** |
|------------------------|-------------------|----------------|---------------------|-----------------------|
| • Determines vertices data are sent<br>• Any custom data can be sent | • **Programmable**<br>• Used to transform vertices to the NDC<br>• Any custom data can be sent through rasterizer to the fragment shader (interpolated) | • **Hardware-accelerated**<br>• Implementation is unknown to the user<br>• Should obey rasterization rules of the OpenGL spec<br>• **Non-programmable**, you can change some states through the API | • **Programmable**<br>• User can do any operations as he/she likes<br>• Receives interpolated data from rasterizer<br>• Writes to framebuffer with single/multiple output channels | • Conditionally writes to the framebuffer<br>• Can blend with previous value on resulting pixel value |

---

## Primitive Assembly

Specify the data types and intake order from buffers:
- What type is the data? (float3, half3, int, int3 etc.)
- How the data is laid out on a buffer? (stride, offset etc.)
- Is it indexed?

**These are packed through Vertex Array Objects (VAOs)**
- Given a triangle list, you can only bind VAO to setup the primitive assembly

### Example: Vertex Layout

**Single Buffer Layout:**

| float3 | ... | float3 | float3 | ... | float3 | float2 | ... | float2 |
|--------|-----|--------|--------|-----|--------|--------|-----|--------|
| pos₀   | ... | posₙ   | normal₀| ... | normalₙ| tCoord₀| ... | tCoordₙ|

```cpp
glGenVertexArrays(1, &vaoId);
glBindVertexArray(vaoId);

// Pos (tightly packed vec3)
glBindVertexBuffer(0, vBufferId, offsets[0], sizeof(glm::vec3));
glEnableVertexAttribArray(0);
glVertexAttribFormat(0, 3, GL_FLOAT, false, 0);

// Normal (tightly packed vec3)
glBindVertexBuffer(1, vBufferId, offsets[1], sizeof(glm::vec3));
glEnableVertexAttribArray(1);
glVertexAttribFormat(1, 3, GL_FLOAT, false, 0);

// UV (tightly packed vec2)
glBindVertexBuffer(2, vBufferId, offsets[2], sizeof(glm::vec2));
glEnableVertexAttribArray(2);
glVertexAttribFormat(2, 2, GL_FLOAT, false, 0);

// Index Buffer Binding
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBufferId);
```

### Primitive Assembly → Vertex Shader

**Handshake between vertex shader, primitive assembly**
- 0, 1, 2 (on `glVertexAttribBinding`) is the attribute index

**CPU (C++)**
```cpp
static constexpr GLuint IN_POS = 0;
static constexpr GLuint IN_NORMAL = 1;
static constexpr GLuint IN_UV = 2;

glVertexAttribBinding(0, IN_POS);
glVertexAttribBinding(1, IN_NORMAL);
glVertexAttribBinding(2, IN_UV);
```

**GPU (Shader)**
```glsl
// Binding Point
#define IN_POS layout(location = 0)
#define IN_NORMAL layout(location = 1)
#define IN_UV layout(location = 2)

in IN_POS vec3 vPos;
in IN_UV vec2 fUV;
in IN_NORMAL vec3 fNormal;
```

---

## Draw Call

Some parts of the Index is on the draw call:
- Due to legacy reasons
- You set the integer type at the draw call (i.e. `GL_UNSIGNED_INT`)
- You can set the start offset here in bytes ("nullptr" variable)

**In draw call, you can specify the access structure:**
- `GL_TRIANGLES` (each successive 3 vertex defines a triangle)
- `GL_TRIANGLE_STRIP` (Use previous two vertex with a new vertex)
- `GL_TRIANGLE_FAN` (use starting vertex and next two vertex)
- `GL_POINTS` (Each vertex is a point)
- `GL_LINES` (Each successive 2 vertex defines a line)

```cpp
// Bind VAO
glBindVertexArray(mesh.vaoId);
glDrawElements(GL_TRIANGLES, mesh.indexCount,
               GL_UNSIGNED_INT, nullptr);
```

---

## Vertex Shader

Shader that processes data from primitive assembly:
- **Programmable**
- You can set common state via uniform variables
- You need to send position data to the rasterizer
  - Special Variable `gl_Position`
  - `gl_Position` must be in **normalized device coordinates (NDC)**
  - Pre perspective divide (aka. clip space), left-handed

```glsl
out gl_PerVertex {vec4 gl_Position;};
out OUT_UV vec2 fUV;
out OUT_NORMAL vec3 fNormal;

void main(void)
{
    fUV = vUV;
    fNormal = normalize(uNormalMatrix * vNormal);
    fNormal = vNormal;
    // Rasterizer
    gl_Position = uProjection * uView * uModel *
                  vec4(vPos.xyz, 1.0f);
}
```

### NDC Space Visualization

The normalized device coordinate (NDC) space is a cube:
- x: -1 to 1 (left to right)
- y: -1 to 1 (bottom to top)
- z: 0 to 1 (near to far, left-handed)

*Reference: [LearnWebGPU Projection Matrices](https://eliemichel.github.io/LearnWebGPU/basic-3d-rendering/3d-meshes/projection-matrices.html)*

---

## Rasterizer

Does a proprietary algorithm (hw-accelerated) and creates fragments (potential pixels):
- Back-face Culling
- Clipping

**States can be adjusted via OpenGL calls:**
- `glEnable(GL_CULL_FACE)`
- `glPolygonMode(...)` (Wireframe or fill)

**Interpolates the given data for each fragment:**
- "out" variables of the vertex shader

---

## Fragment (Pixel) Shader

**Programmable shader:**
- User code process data and writes to framebuffer
- Can have texture bindings
  - Modern GPUs supports textures on Vertex Shader as well
- Can have uniform variables

```glsl
in IN_UV vec2 fUV;
in IN_NORMAL vec3 fNormal;
out OUT_FBO vec4 fboColor;

void main(void)
{
    // Pure Red
    fboColor = vec4(1, 0, 0, 1);
}
```

---

## Framebuffer Write

**Conditionally Writes the output of the Pixel Shader:**
- Depth Culling
- Stencil Operations
- Blending (Alpha Blending)

**In HW3 you probably will use:**
```cpp
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
```
Which will do a linear interpolation (blend) between fbo's color value and the fragment's color value.

---

## OpenGL States

### Overview

**Various states needs to be adjusted:**
- All states in OpenGL is **persistent** (Once you set it, it stays set throughout the program)
- We will cover the memory-related states and uniform states of shaders
- For other states: [OpenGL Reference Pages](https://registry.khronos.org/OpenGL-Refpages/gl4/)

**Create / Bind / Delete usage:**
1. You create an OpenGL object
2. Bind it to appropriate location
3. Commands to that binding acts on the bind object
   - No direct state change for a specific object
   - Little bit convoluted
4. Use it (probably indirectly via a draw call)
5. Destroy it

---

## Shaders

Since OpenGL is platform agnostic, shaders are sent to GPU as source:
- GPU compiles the shaders (vertex, fragment), and create the entire pipeline
- Modern APIs (such as Vulkan) sends a semi-compiled, intermediate representation to the GPU (called **SPIR-V**)

**With an extension you can separately compile and bind vertex and fragment shaders:**
- Starter code has this approach
- Course slides have the more traditional approach
- You can use both

### Shader Compilation Example

```cpp
glGenProgramPipelines(1, &renderPipeline);
glBindProgramPipeline(renderPipeline);

GLuint shaderGL = glCreateShader(t);
glShaderSource(shaderGL, 1, &srcPtr, &sourceSize);
glCompileShader(shaderGL);

GLint isCompiled = GL_FALSE;
glGetShaderiv(shaderGL, GL_COMPILE_STATUS, &isCompiled);
if(isCompiled == GL_FALSE)
{
    // Handle error
}

shaderId = glCreateProgram();
glProgramParameteri(shaderId, GL_PROGRAM_SEPARABLE, GL_TRUE);
glAttachShader(shaderId, shaderGL);
glLinkProgram(shaderId);

GLint isLinked = GL_FALSE;
glGetProgramiv(shaderId, GL_LINK_STATUS, &isLinked);
if(isLinked == GL_FALSE)
{
    // Handle error
}
```

### Shader Usage

**This approach is more flexible**

Although this is an extension, all GPUs will support this since another graphics API (DirectX) has this approach.

```cpp
// During Usage
glUseProgramStages(renderPipeline, GL_VERTEX_SHADER_BIT, shaderId);
glActiveShaderProgram(renderPipeline, shaderId);
```

---

## Buffers

**GPU owned memory:**
- Think of it as arrays (or just some allocation)

**Common Binding Points:**
- `GL_ARRAY_BUFFER` (vertex data)
- `GL_ELEMENT_ARRAY_BUFFER` (index data)
- `GL_UNIFORM_BUFFER` (uniform data in memory of the GPU - shader uniforms are different!)

```cpp
glGenBuffers(1, &vBufferId);
glBindBuffer(GL_ARRAY_BUFFER, vBufferId);
glBufferStorage(GL_ARRAY_BUFFER, offsets.back(), nullptr,
                GL_DYNAMIC_STORAGE_BIT);

glBufferSubData(GL_ARRAY_BUFFER, offsets[0], sizes[0],
                linPositions.data());
glBufferSubData(GL_ARRAY_BUFFER, offsets[1], sizes[1],
                linNormals.data());
glBufferSubData(GL_ARRAY_BUFFER, offsets[2], sizes[2],
                linUVs.data());
```

---

## Uniform Binding

**Uniforms are constant data of the shaders:**
- Read-only
- You set it from the CPU side and read from shaders
- Via `glUniform*` family of functions

**Each uniform variable in shader has a location:**
- Either query it via `glGetUniformLocation(programId, "variableName")`
- Or directly bind it to a location (similar to the VAO)

### CPU (C++)

```cpp
static constexpr GLuint U_TRANSFORM_MODEL = 0;

glUniformMatrix4fv(U_TRANSFORM_MODEL, 1, false,
                   glm::value_ptr(model));
```

### GPU (Shader)

```glsl
#define U_TRANSFORM_MODEL layout(location = 0)

U_TRANSFORM_MODEL uniform mat4 uModel;
```

---

## Textures

**Special 1/2/3D data:**
- Hardware-accelerated interpolation/normalization
- Automatic out-of-bounds handling
- Can be accessed parametrically (with values between 0-1, instead of indices)
- Read-only (on shader side)
- Indirectly can be written via framebuffer objects

### Memory Allocation

```cpp
glGenTextures(1, &textureId);
glBindTexture(GL_TEXTURE_2D, textureId);
glTexStorage2D(GL_TEXTURE_2D, GLsizei(mipCount),
               internalFormatSized, width, height);
glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                internalFormat, pixType, rawPixels);
```

### Sampling State

```cpp
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                GL_LINEAR_MIPMAP_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
```

---

## Texture Binding

**Similar to the Uniforms:**
- Syntax is slightly different (instead of specifying "location" we specify "binding" point)
- As far as I know, there's no programmatical way to get binding location from CPU side

### CPU (C++)

```cpp
static constexpr GLuint T_ALBEDO = 0;

glActiveTexture(GL_TEXTURE0 + T_ALBEDO);
glBindTexture(GL_TEXTURE_2D, tex.textureId);
```

### GPU (Shader)

```glsl
#define T_ALBEDO layout(binding = 0)
uniform T_ALBEDO sampler2D tAlbedo;

void main(void)
{
    fboColor = texture2D(tAlbedo, fUV);
}
```

---

## Framebuffer Objects

**Framebuffers are output targets of the Graphics Pipeline**

**Useful when:**
- Using the GPU but not displaying image (cmd application)
- Applying techniques such as shadow mapping

**When windows is created an implicit framebuffer is available (fbo 0):**
- A color buffer and a depth buffer

**FBOs can have multiple outputs**

**You can create framebuffers with render targets or textures:**
- Render targets can't be bound to shaders
- Textures can be used for other draw calls etc.

**Don't forget to call `glViewport(...)` after you bind a framebuffer**

```cpp
glGenFramebuffers(1, &shadowFBO);
// Create Textures
// ...

glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadowFBO);
glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                       GL_TEXTURE_2D, colorTex, 0);
glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
                       GL_DEPTH_STENCIL_ATTACHMENT,
                       GL_TEXTURE_2D, depthTex, 0);
```

---

## Framebuffer Output

**Similar to the VAOs you can specify output location variables explicitly:**
- Data types needs to match (or GPU will silently discard the values)
  - GL_RGBA8 texture, should have vec3 variable in the shader
- Dimensions of the textures must match

### CPU (C++)

```cpp
glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
                       GL_COLOR_ATTACHMENT0,
                       GL_TEXTURE_2D, colorTex0, 0);
glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
                       GL_COLOR_ATTACHMENT1,
                       GL_TEXTURE_2D, colorTex1, 0);
```

### GPU (Shader)

```glsl
#define OUT_FBO_0 layout(location = 0)
#define OUT_FBO_1 layout(location = 1)

out OUT_FBO_0 float fboColor0;
out OUT_FBO_1 vec3 fboColor1;

void main(void)
{
    // ...
}
```

---

## Useful Links

### OpenGL Reference Pages
- [https://registry.khronos.org/OpenGL-Refpages/gl4/](https://registry.khronos.org/OpenGL-Refpages/gl4/)
- This page also has all the GLSL intrinsics

### GLFW Tutorial
- [https://www.glfw.org/docs/latest/quick.html](https://www.glfw.org/docs/latest/quick.html)

### Beginner Tutorial
- [http://www.opengl-tutorial.org/beginners-tutorials/](http://www.opengl-tutorial.org/beginners-tutorials/)

---

## Questions?

**IT CAN ALSO BE ABOUT HW3**

---

*End of Recitation*
