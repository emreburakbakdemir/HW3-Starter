# CENG 477 - Introduction to Computer Graphics
## Graphics Hardware and OpenGL

---

## Introduction

- Until now, we focused on graphic **algorithms** rather than hardware and implementation details
- But graphics, without using specialized tools and/or hardware would simply be too slow for most applications
- We will now learn about how a **GPU** works and how to program it using a specific API: **OpenGL**
- The presented ideas should apply to other APIs such as Direct3D with some modifications

---

## Graphics Hardware (GH)

- GH is a set of components which implements the forward rendering pipeline at a chip level called **GPU**
- Modern GPUs are **programmable**
- GPUs are massively **parallel** (orders of magnitude more parallel than CPUs)
- GPUs change continuously mainly due to the demands of the video game industry
- Big players:
  - AMD, Nvidia, Intel, Microsoft, Apple, Qualcomm, …

---

## Graphics Processing Unit (GPU)

- How parallel are GPUs? Let's watch this demo:

![GPU Demo](placeholder)

---

## GPGPU

- As a result of this performance, GPUs are used in many tasks that are not related to graphics at all:
  - Called **GPGPU**: General-purpose computing on GPU
- Nvidia developed the **CUDA** language for GPGPU
- **OpenCL** is supported by AMD, Nvidia, and Intel
- Nowadays, many computational intensive tasks are performed on the GPU:
  - Image and video processing
  - Analyzing big data
  - Bioinformatics
  - Optimization
  - Machine learning, …

---

## GPU Architecture

- GPUs are similar to CPUs in their building blocks (in fact they are somewhat simpler than CPUs):
  - Some logic to decode the instruction to be performed
  - Registers
  - Arithmetic logic units (ALUs)
  - Cache
  - Memory
- But they are massively parallel:
  - Data parallelism
  - Pipeline parallelism

---

## GPU Parallelism

- What makes GPUs parallel?
- GPUs are SIMD architectures
  - **SIMD**: Single instruction multiple data
  - The same instruction is applied to thousands of data elements at the same time
```
        SIMD      Instruction Pool
                        ↓
        Data Pool → Vector Unit
                    [PU PU PU PU]
```

---

## GPU Parallelism

- This works well for **independent** tasks such as:
  - Transforming vertices
  - Computing shading for each fragment
- Ideal if the task is the same but the data is different:
```
Data length
┌─────────────────────────────────────┐
│ 1  2  3  4  5  6  7  8  9  10  ...  │
│ ↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓       │
│/2 /2 /2 /2 /2 /2 /2 /2 /2 /2       │
│ ↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓       │
│0.5 1 1.5 2 2.5 3 3.5 4 4.5 5  ...  │
└─────────────────────────────────────┘
           Pipeline length
                 ⋮
```

---

## GPU vs CPU

- GPUs have a larger number of ALUs allowing for data parallelism:
```
     CPU                           GPU
┌──────────┬────┬────┐    ┌─┬────────────────┐
│ Control  │ALU │ALU │    │─│                │
│          ├────┼────┤    │─│                │
│          │ALU │ALU │    │─│                │
├──────────────────────┐  │─│                │
│      Cache           │  │─│                │
├──────────────────────┤  │─│                │
│      DRAM            │  │─│                │
└──────────────────────┘  │─│                │
                          └─┴────────────────┘
                          │     DRAM         │
                          └──────────────────┘
```

---

## GPU vs CPU

Let's compare a good GPU with a good CPU:

| Intel Core i9-13900K | Nvidia GeForce RTX 4090 |
|---------------------|------------------------|
| Cores: 24           | Cores: 16,384         |
| Clock: 3.0 GHz      | Clock: 2.3 GHz        |
| Power: 125W         | Power: 450W           |
| Memory BW: ~89.6 GB/s | Memory BW: ~1.01 TB/s |

---

## Overall GPU Architecture

### NVIDIA GeForce 8800 vs OpenGL Pipeline

**NVIDIA GeForce 8800:**
```
Application
    ↓
Data Assembler
    ↓
Vtx Thread Issue  Prim Thread Issue  Frag Thread Issue
    ↓                    ↓                    ↓
[SP SP][SP SP][SP SP][SP SP][SP SP][SP SP][SP SP][SP SP]
[L1  ][L1  ][L1  ][L1  ][L1  ][L1  ][L1  ][L1  ]
    ↓         ↓         ↓         ↓         ↓         ↓
[L2][FB]  [L2][FB]  [L2][FB]  [L2][FB]  [L2][FB]  [L2][FB]
```

**OpenGL Pipeline:**
```
Application
    ↓
Vertex assembly
    ↓
Vertex operations
    ↓
Primitive assembly
    ↓
Primitive operations
    ↓
Rasterization
    ↓
Fragment operations
    ↓
Framebuffer
```

---

## GPU Data Flow Model
```
┌──────────────┐         ┌─────────────┐         ┌─────────────┐
│ User Program │  Bus    │  Geometry   │         │  Fragment   │
│              ├────────→│ Processing  ├────────→│ Processing  │
│              │         │             │         │             │
└──────────────┘         └─────────────┘         └──────┬──────┘
     CPU                      GPU                       │ GPU
                                                         ↓
┌──────────────┐         ┌─────────────┐         ┌─────────────┐
│   Monitor    │ VGA/DVI │  Displayed  │         │ Framebuffer │
│              │←────────│   Image     │←────────│             │
│              │ HDMI... │             │         │             │
└──────────────┘         └─────────────┘         └─────────────┘
                                                       GPU
```

---

## User Program

- The user program is an **OpenGL** (or Direct3D) program which itself runs on the CPU
  - C, C++, C#, Python, … many languages are supported
- Also initially all data is in the main system memory
- The user program is responsible to arbitrate the overall flow and send data to GPU:
  - Open a window
  - Manage user interaction (mouse, keyboard, etc.)
  - Decide what to draw and when to draw
  - Ask GPU to compile shaders (programs to be later run on the GPU)

---

## Opening a Window

- Opening a window for rendering is not part of OpenGL
  - Each OS has a different mechanism
- There are some high-level APIs that simplify this process
  - GLFW is one such API
```c
GLFWwindow* window;
if (!glfwInit()) 
    exit(EXIT_FAILURE);
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); 
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
if (!window) { 
    glfwTerminate();
    exit(EXIT_FAILURE);
}
glfwMakeContextCurrent(window);
```

---

## Double Buffering

- By default GLFW windows are double buffered
- Double buffering avoids problems such as flickering and tearing:
```
┌─────────────────┐    ┌─────────────────┐
│  With Tearing   │    │   No Tearing    │
│                 │    │                 │
│    ─────►       │    │                 │
│                 │    │                 │
│    ─────►       │    │                 │
└─────────────────┘    └─────────────────┘
```

**Tearing Example:**
![Tearing visualization](placeholder)

---

## Double Buffering

- To ensure that tearing does not occur, make sure to use the following GLFW command as well:
  - `glfwSwapInterval(1)`
- The default value of the swap interval is 0, which can cause swapping of the buffers in the middle of the monitor's refresh cycle
- With `glfwSwapInterval(1)`, it is ensured that buffer swapping occurs at the same time as the monitor is refreshed (called the **vsync** time)
- This setting can be enforced using the driver's settings as well (e.g. NVIDIA control panel)

---

## Double Buffering

- In double buffering, we render to a **back buffer** and show that buffer only when drawing is complete (usually synchronized with monitor's refresh cycle)
  - Windowed mode requires a copy:

**Double Buffering Process:**
```
1. Draw
   ┌──────────┐         ┌──────────┐
   │  Image   │         │  Screen  │
   │ (graphics)│         │          │
   │Back Buffer│         │Primary   │
   └──────────┘         │Surface   │
                        └──────────┘

2. Blit (copy)
   ┌──────────┐         ┌──────────┐
   │  Image   │ ──────► │  Screen  │
   │          │         │ (graphics)│
   │Back Buffer│         │Primary   │
   └──────────┘         │Surface   │
                        └──────────┘
```

---

## Double Buffering

- In fullscreen mode, only the video pointer is flipped:

**Page Flipping:**
```
1. Draw
   Back Buffer          Primary Surface
   ┌──────────┐         ┌──────────┐
   │   [A]    │         │          │
   │ graphics │   ──►   │  Screen  │
   └──────────┘  video  └──────────┘
                 pointer

2. Flip
   Primary Surface      Back Buffer
   ┌──────────┐         ┌──────────┐
   │   [A]    │   ◄──   │          │
   │ graphics │  video  │          │
   └──────────┘  pointer└──────────┘
        │
        ▼
      Screen
```

---

## Managing User Interaction

- The user may interact with the program through input devices: traditionally keyboard and mouse
- GLFW also simplifies this task by registering callbacks:
```c
glfwSetKeyCallback(window, key_callback);
glfwSetMouseButtonCallback(window, mouse_button_callback);

static void key_callback(GLFWwindow* window, int key, int scancode,
                        int action, int mods)
{ 
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) 
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void mouse_button_callback(GLFWwindow* window, int button,
                          int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        popup_menu();
}
```

---

## Displaying/Resizing the Window

- Whenever a window is displayed or resized, certain settings (such as the viewport) may need to be updated:
- This function can also be registered by using GLFW:
```c
glfwSetWindowSizeCallback(window, window_resized);

void window_resized(GLFWwindow* window, int width, int height)
{
    // code to update viewport/projection etc.
    glViewport(0, 0, width, height);
}
```

---

## Rendering Each Frame

- We first clear all buffers, then render our frame, and finally swap buffers (remember double buffering):
```c
while (!glfwWindowShouldClose(window))
{ 
    renderScene();        // Clearing of the framebuffer
    animate();            // and rendering of the scene
    showFPS(window);      // is performed here
    glfwSwapBuffers(window); 
    glfwPollEvents();
}                         // We may insert any code to
                          // update the scene parameters in
glfwDestroyWindow(window); // our rendering loop
glfwTerminate();
exit(EXIT_SUCCESS);
```

---

## GLFW

- You may learn more about GLFW at:
  - https://www.glfw.org/docs/3.3/window_guide.html
- There are some other window managers as well:
  - GLUT (http://freeglut.sourceforge.net/)
  - SDL (https://www.libsdl.org/)
- General purpose GUI libraries can also be used:
  - E.g. wxWidgets, QT, FLTK, …
- You can even use the native window managers such as X11, Cocoa, or Windows' native library functions
  - In this case, your application will not be cross-platform

---

## Sending Geometry Data

- The user program must communicate the geometry information to the GPU
- A simple approach:
```c
glBegin(GL_LINES);
    glVertex3f(x0, y0, z0);
    glVertex3f(x1, y1, z1);
glEnd();
```

- We tell GPU that we want to draw a line from (x₀, y₀, z₀) to (x₁, y₁, z₁)

---

## Sending Geometry Data

- Attributes besides position can be sent as well:
```c
glBegin(GL_LINES);
    glColor3f(1, 0, 0); // red (note that range is in [0, 1])
    glVertex3f(x0, y0, z0);
    glColor3f(0, 1, 0); // green
    glVertex3f(x1, y1, z1);
glEnd();
```

- We tell GPU that we want to draw a line from (x₀, y₀, z₀) to (x₁, y₁, z₁)
- The endpoint colors are (1, 0, 0) and (0, 1, 0)

---

## Sending Geometry Data

- Triangles are similar:
```c
glBegin(GL_TRIANGLES);
    glVertex3f(x0, y0, z0);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
glEnd();
```

- Every group of **three vertices** define a triangle
- Drawing two triangles:
```c
glBegin(GL_TRIANGLES);
    glVertex3f(x0, y0, z0); glVertex3f(x1, y1, z1); glVertex3f(x2, y2, z2);
    glVertex3f(x3, y3, z3); glVertex3f(x4, y4, z4); glVertex3f(x5, y5, z5);
glEnd();
```

---

## Sending Geometry Data

- With this approach m triangles require **3m** vertex calls
- An improved method is to use **triangle strips** for meshes
- The first three vertices define the first triangle
- Every vertex afterwards defines a new triangle
- m triangles require **m+2** vertex calls
```c
glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(x0, y0, z0);    // Triangle: 0, 1, 2
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glVertex3f(x3, y3, z3);    // Triangle: 2, 1, 3
glEnd();
```
```
      0 ────── 2
       \      /│
        \    / │
         \  /  │
          1 ───┘3
```

---

## Winding Order

- **Winding order** determines the facing of a triangle
- Here both triangles are facing toward the viewer:
```c
glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(x0, y0, z0);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glVertex3f(x3, y3, z3);
glEnd();
```
```
      0 ────── 2
       \      /│
     0,1,2   / │
         \  / 2,1,3
          1 ───┘3
```

---

## Winding Order

- **Winding order** determines the facing of a triangle
- Here both triangles are facing away from the viewer:
```c
glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(x0, y0, z0);
    glVertex3f(x2, y2, z2);
    glVertex3f(x1, y1, z1);
    glVertex3f(x3, y3, z3);
glEnd();
```
```
      0 ────── 2
       \      /│
     0,2,1   / │
         \  / 1,2,3
          1 ───┘3
```

- It is important to use a consistent winding order when drawing a mesh due to **backface culling**

---

## Graphics State

- OpenGL is a **state machine**
- Various states are preserved until we change them
- In the example below, the color of each vertex is set to (0, 1, 0), that is green:
```c
glColor3f(0, 1, 0)
glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(x0, y0, z0);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glVertex3f(x3, y3, z3);
glEnd();
```

---

## Graphics State

- Below the first three vertices have the same color and normal
- The fourth vertex has a different color and normal:
```c
glColor3f(0, 1, 0)
glNormal3f(0, 0, 1)
glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(x0, y0, z0);
    glVertex3f(x2, y2, z2);
    glVertex3f(x1, y1, z1);
    glColor3f(1, 0, 0)
    glNormal3f(1, 0, 1)
    glVertex3f(x3, y3, z3);
glEnd();
```

---

## Sending Geometry Data

- Previous examples send data in **immediate mode**
- **Immediate mode is inefficient**: A large model would require too many glVertex calls
  - Each glVertex call is executed on the CPU and the corresponding data is sent to the GPU
- A better approach would be to send all vertex data to the GPU using a single call
- We use **vertex arrays** for that purpose

---

## Vertex Arrays

- There are several arrays such as vertex position array, vertex color array, vertex normal array, …
- Below is an example of vertex position array:
```
glVertexPointer(size, type, stride, pointer)
                 ↓      ↓       ↓        ↓
    Number of      Type of   Byte offset  Pointer to
    coordinates    each      between      vertex
    per vertex     coordinate consecutive  position data
                              vertices
```

- You must enable an array before using it:
```c
glEnableClientState(GL_VERTEX_ARRAY)
```

---

## Vertex Arrays

- In modern OpenGL, these explicit attribute names are replaced by a **generic** attribute array function:
```
glVertexAttribPointer(index, size, type, normalized, stride, pointer)
                       ↓      ↓      ↓        ↓          ↓        ↓
    Array         Number of   Type of  Whether    Byte offset  Pointer to
    index         coordinates each     integer    between      vertex
                  per vertex  coord    data should consecutive position data
                                      be normalized vertices
```

- Don't forget to enable it:
```c
glEnableVertexAttribArray(index)
```

---

## Drawing with Vertex Arrays

- We use a single draw call to draw using vertex arrays:
```
glDrawArrays(mode, first, count)
              ↓      ↓       ↓
         Primitive  Starting  Number of
         type       index in  indices to
                    enabled   be rendered
                    arrays
```

---

## Drawing with Vertex Arrays

- **glDrawArrays** may still be inefficient as vertex attribute data must be repeated for each primitive
- **glDrawElements** is designed to solve this issue by using indices:
```
glDrawElements(mode, count, type, indices)
                ↓      ↓      ↓       ↓
           Primitive  Number  Type of Pointer to
           type       of      each    indices
                      indices index
                      to be
                      rendered
```

---

## Drawing with Vertex Arrays

- When using client-side vertex arrays, the vertex attribute data is copied from the system memory (user pointer) to the GPU memory at every draw call
- There is a better alternative, known as **vertex buffers**
```
glVertexPointer(size, type, stride, pointer)
                                      ↓
┌─────────────────────┐         ┌─────────────────────┐
│  System memory      │ ──────► │  GPU memory         │
│  ……………………………………………… │  copy   │  ……………………………………………… │
│                     │  at     │                     │
│                     │  draw   │                     │
│                     │  call   │                     │
└─────────────────────┘         └─────────────────────┘
```

---

## Vertex Buffer Objects

- Previous methods required the data to be copied from the system memory to GPU memory at each draw
- **Vertex Buffer Objects (VBOs)** are designed to allow this copy to take place only once
- The copied data is reused at each draw

---

## Vertex Buffer Objects

- To use VBOs, we generate two buffers:
  - Vertex attribute buffer (position, color, normal, etc.)
  - Element array buffer (indices)
```c
GLuint vertexAttribBuffer, indexBuffer;
glGenBuffers(1, &vertexAttribBuffer);
glGenBuffers(1, &indexBuffer);
```

---

## Vertex Buffer Objects

- Next, we bind these buffers to locations that are meaningful for the GPU:
```c
glBindBuffer(GL_ARRAY_BUFFER, vertexAttribBuffer);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer)
```

---

## Vertex Buffer Objects

- We then ask the GPU to allocate memory for us and copy our data into this memory
```c
glBufferData(GL_ARRAY_BUFFER, aSize, aPtr, GL_STATIC_DRAW);
              ↓                   ↓      ↓
         Attribute data     Attribute data
         size in bytes      pointer

glBufferData(GL_ELEMENT_ARRAY_BUFFER, iSize, iPtr, GL_STATIC_DRAW);
                                        ↓       ↓
                                   Index data  Index data
                                   size in     pointer
                                   bytes
```

---

## Vertex Buffer Objects

- Once this is done, the CPU data can safely be deleted:
```c
glBufferData(GL_ARRAY_BUFFER, aSize, aPtr, GL_STATIC_DRAW);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, iSize, iPtr, GL_STATIC_DRAW);

delete[] aPtr;
delete[] iPtr;
```

---

## Vertex Buffer Objects

- Before drawing, we can specify an offset into our buffers
- It is accomplished by the same function as before
- But this time, pointer indicates a byte **offset** into our buffer (similar for glColorPointer, etc.)
```
glVertexPointer(size, type, stride, pointer)
                 ↓      ↓       ↓        ↓
    Number of      Type of   Byte offset  Offset to
    coordinates    each      between      vertex
    per vertex     coordinate consecutive  position data
                              vertices
```

---

## Vertex Buffer Objects

- Drawing is the same as before where index pointer is now also an **offset** to the element array buffer:
```
glDrawElements(mode, count, type, indices)
                ↓      ↓      ↓       ↓
           Primitive  Number  Type of Offset to
           type       of      each    indices
                      indices index
                      to be
                      rendered
```

---

## Vertex Buffer Objects

- The relevant buffers must still be enabled:
```c
glEnableClientState(GL_VERTEX_ARRAY)
glEnableClientState(GL_COLOR_ARRAY)
…
```

- Unfortunately, this is a very bad naming as it suggests client-side data is being used
- In modern OpenGL, these are replaced with:
```c
glEnableVertexAttribArray(0);
glEnableVertexAttribArray(1);
…
```

---

## Vertex Buffer Objects

- Note that in **glVertexPointer** and **glDrawElements** the last parameter is sometimes treated as pointer and sometimes offset
- OpenGL makes this decision as follows:
  - If a non-zero name is bound to **GL_ARRAY_BUFFER**, the last parameter glVertexPointer is treated as offset (otherwise pointer)
  - If a non-zero name is bound to **GL_ELEMENT_ARRAY_BUFFER**, the last parameter glDrawElements is treated as offset (otherwise pointer)

---

## Performance Comparison

- Drawing an Armadillo model comprised of 212,574 triangles at four distinct locations (resulting in a total of 850,296 triangles):

![Armadillo models](placeholder)

---

## Performance Comparison

- On AMD Mobility Radeon HD4650 and at resolution 640x480:
  - Using VBOs the frame rate was about **100 FPS**
  - Using client-side glDrawElements, the frame rate was about **20 FPS**
- Therefore, almost all modern games use VBOs for drawing complex models

---

## Transformations in OpenGL

- In classic OpenGL, transformations are performed using three commands:
```c
glTranslatef(deltaX, deltaY, deltaZ);
glRotatef(angle, axisX, axisY, axisZ);
glScalef(scaleX, scaleY, scaleZ);
```

- These commands update the current matrix
- Therefore the current matrix should be set as **GL_MODELVIEW** before calling these commands
- Note that **angle is in degrees (not radians)!**

---

## Transformations in OpenGL

- Transformations apply in the **reverse order**
- The command closest to the draw call takes effect first
```c
glTranslatef(deltaX, deltaY, deltaZ);
glRotatef(angle, axisX, axisY, axisZ);
glScalef(scaleX, scaleY, scaleZ);

drawCube();
```

- Here, the cube is first scaled, then rotated, and finally translated

---

## Transformations in OpenGL

- Transformations keep affecting the current matrix
- If you want to draw an object at the same position at each frame you need to reset the matrix to identity:
```c
glLoadIdentity();

glTranslatef(deltaX, deltaY, deltaZ);
glRotatef(angle, axisX, axisY, axisZ);
glScalef(scaleX, scaleY, scaleZ);

drawCube();
```

- Otherwise your object will quickly disappear!

---

## Transformations in OpenGL

- In OpenGL, we do not specify a camera position
- It is assumed that the camera is at **(0, 0, 0)** and looking down the **negative z axis**
- You can view a modelview transformation in two ways:
  - Transform all objects drawn after the transformation by keeping the camera fixed
  - Transform the camera (i.e. coordinate system) by the opposite transformations by keeping the objects fixed
- In reality, objects are transformed but both would produce the same result

---

## Transformations in OpenGL

- The following approach is in fact deprecated:
```c
glLoadIdentity();

glTranslatef(deltaX, deltaY, deltaZ);
glRotatef(angle, axisX, axisY, axisZ);
glScalef(scaleX, scaleY, scaleZ);

drawCube();
```

- In modern OpenGL, we create the transformation matrix and send it to the **"vertex shader"** as a **"uniform variable"**
- **GLM** library is commonly used for this purpose

---

## Transformations in OpenGL
```cpp
// Include the GLM headers
#include <glm/glm.hpp> // GL Math library header
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Specify the transformation
mat4 model = glm::mat4(1.0f); // identity matrix
model = glm::translate(model, glm::vec3(1, 2, 3));
model = glm::rotate(model, M_PI * (90. / 180.), glm::vec3(0, 1, 0));
                              // ↑ Angle is in radians
model = glm::scale(model, vec3(2, 2, 2));

// Send your matrix to the GPU (more on this later)
…

// In the shader, when applied on a vertex, v, you will get:
// v' = T * R * S * v
```

---

## Summary

- **OpenGL** is a cross-platform API for rendering 3D graphics
  - Many language bindings
- Today, modern OpenGL API is fully hardware accelerated
- **GLFW** is a cross-platform library for window management
  - User interaction with keyboard, mouse also part of it
- **GLM** is a header only library for transformations
  - This is CPU only
  - The computed matrix is uploaded to GPU by the user
- The idea is *minimize* CPU usage and *maximize* GPU utilization for high performance graphics

---

*CENG 477 – Computer Graphics*
*Middle East Technical University (METU)*