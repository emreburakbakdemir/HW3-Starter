# CENG 477 - Introduction to Computer Graphics
## Shadows in Forward Rendering

---

## Shadows

• Shadows are an important element of visual realism

![Three example scenes showing shadows in computer graphics - a medieval street scene, a tree casting shadows, and transparent objects casting colored shadows](visual examples)

*From Sintorn et al. – Siggraph Asia 2012*

---

## Shadows

• Shadows give important cues about light positions

![Two views of a Greek temple-like structure on a green plane - one without shadows showing ambiguous light position, one with shadows clearly showing light direction](comparison images)

*From wikipedia.com*

---

## Shadows

• Shadows also give cues about object positions

![Two checkerboard scenes with brown spheres - top image shows spheres floating above board, bottom image shows spheres with shadows indicating contact with surface](position comparison)

---

## Shadows in OpenGL

• OpenGL does not have built-in support for shadows

![Scene showing a horse, cube, and teacup rendered without shadows on a gray ground plane](no shadows example)

---

## Shadows in OpenGL

• Compare this to a ray traced image:

![Same scene as previous slide but with realistic shadows under all objects](ray traced shadows)

---

## Shadows in OpenGL

• OpenGL does not natively support generating shadows
  – That is, it does not have a function like glMakeShadow()!
  
• But, several shadowing algorithms can easily be implemented using features of OpenGL
  – Stencil buffer
  – Depth buffer
  – Render to texture
  – …

---

## Generating Shadows

• Two algorithms that are commonly used are:
  – Shadow volumes (Crow, 1977)
  – Shadow mapping (Williams, 1978)
  
• Both algorithm has advantages and disadvantages and many variants

• Still an active research area:
  – Wang, J., Li, Z., Ali, S. G., Sheng, B., Li, P., Chi, X., ... & Mao, L. (2021, September). Dynamic Shadow Synthesis Using Silhouette Edge Optimization. In Computer Graphics International Conference (pp. 421-432). Cham: Springer International Publishing.
  
• We'll study the basic versions of these algorithms

*CROW, F. C. 1977. Shadow algorithms for computer graphics. SIGGRAPH Comput. Graph. 11 (July), 242–248.*
*WILLIAMS, L. 1978. Casting curved shadows on curved surfaces. SIGGRAPH Comput. Graph. 12 (August), 270–274.*

---

## Shadow Volumes

• The idea is to create a 3D shape that represents the shadow that is casted by an object

![Diagram showing light source, shadow caster (red triangle), shadow volume extending to infinity, and unshadowed vs shadowed objects](shadow volume concept)

---

## Shadow Volumes

• A shadow volume can be created for any arbitrary object.

• We need to determine the **contour edges** (silhouette-edges) of the object as seen from the light source

• A **contour edge** has one adjacent polygon facing the light source and the other away from the light source

![Two views of a sphere showing contour edges highlighted in red - one from front view, one from side showing the extended shadow volume](contour edges example)

*From John Tsiombikas*

---

## Contour Edges

• A **contour edge** has one adjacent polygon facing the light source and the other away from the light source

• We can use dot product to decide whether a face is toward or away from the light source

![Diagram showing light source, shadow caster object with face normals indicated](dot product illustration)

---

## Contour Edges

```cpp
// transform the light to the coordinate system of the object
LightPosition = Inverse(ObjectWorldMatrix) * LightPosition;
for (every polygon) {
    IncidentLightDir = AveragePolyPosition - LightPosition;
    // if the polygon faces away from the light source....
    if (DotProduct(IncidentLightDir, PolygonNormal) >= 0.0) {
        for (every edge of the polygon) {
            if (the edge is already in the contour edge list) {
                // then it can't be a contour edge since it is
                // referenced by two triangles that are facing
                // away from the light
                remove the existing edge from the contour list;
            } else {
                add the edge to the contour list;
            }
        }
    }
}
```

*From John Tsiombikas*

---

## Extruding Contour Edges

• Once the contours are found, we need to extrude them to create a large shadow volume

![Diagram showing light source and shadow caster with contour edges being extruded away from light to create shadow volume](extrusion illustration)

---

## Extruding Contour Edges

• Extrusion amount should be large enough to cover all objects which can receive shadow from this light source

```cpp
ExtrudeMagnitude = A_BIG_NUMBER;
for (every edge) {
    ShadowQuad[i].vertex[0] = edge[i].vertex[0];
    ShadowQuad[i].vertex[1] = edge[i].vertex[1];
    ShadowQuad[i].vertex[2] = edge[i].vertex[1] + ExtrudeMagnitude *
        (edge[i].vertex[1] - LightPosition);
    ShadowQuad[i].vertex[3] = edge[i].vertex[0] + ExtrudeMagnitude *
        (edge[i].vertex[0] - LightPosition);
}
```

*From John Tsiombikas*

---

## Extruding Contour Edges

• The shadow caster object's contour vertices serve as the *cap* of the shadow volume.

• The *bottom* can also be capped to obtain a closed volume.

![Diagram showing complete shadow volume with top cap at object and bottom cap at infinity](capped volume)

---

## Extruding Contour Edges

• This is how it looks like for a complex object

![Two images of a teapot - one showing the rendered teapot with light, one showing the semi-transparent shadow volume extending from it](complex object example)

*From John Tsiombikas*

---

## Example

![Screenshot of a 3D room with white walls and a blue rectangular object casting a shadow](example render)

---

## Rendering Shadows

• Now what? Any ideas about how we can proceed?

![Diagram showing camera, light source, red sphere shadow caster, and blue cube shadow receiver with rays illustrated](rendering setup)

*From John Tsiombikas*

---

## Rendering Shadows

• Assume a ray originating from the eye

• If it **enters** the shadow volume from a front face and **exits** from a back face, the point it reaches is not in shadow

• However, **if it does not exit before hitting the object**, the point should be in shadow

![Same diagram as previous with emphasis on ray entering and potentially not exiting shadow volume before hitting object](ray-volume intersection)

---

## Rendering Shadows

• But we are not ray tracing! How can we know if the ray enters or exits?

• This is where **depth** and **stencil** buffers come in handy

• **Stencil buffer:**
  – An integer buffer that stores a value for every pixel (usually an 8-bit value)
  – We can clear it to any value that we want (glClearStencil(int) and glClear(GL_STENCIL_BUFFER_BIT))
  – Has operations such as **increment** and **decrement** based on the result of the depth test (glStencilOp(sfail, dfail, dpass))
  
• Think of stencil buffer as a **counter buffer**, which keeps a counter for every pixel

---

## Shadow Volume Algorithm (Part I)

• **Clear** the depth (to 1.0) and stencil buffer (to 0)

• **Enable** depth testing and **disable** stencil testing

• Render the scene with **ambient light** (note that ambient light does not produce shadows). This updates the depth buffer value for every pixel that corresponds to an object

• **Disable writing** to the depth buffer

**Color Buffer** (8x8 grid showing brown colored pixels for object)  
**Depth Buffer** (8x8 grid with 1.0 for background, 0.5 for object pixels)  
**Stencil Buffer** (8x8 grid all zeros)

---

## Shadow Volume Algorithm (Part II)

• Draw the **front faces** of the shadow volume. Increment the stencil value for every pixel that passes the depth test (glStencilOp(GL_KEEP, GL_KEEP, **GL_INCR**))

**Stencil Buffer – Front Pass**

```
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
0 0 1 1 1 1 0 0
0 0 1 1 1 1 0 0
0 0 1 1 1 1 0 0
0 0 1 1 1 1 0 0
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
```

*Stencil value for fragments behind the shadow volume's front faces will be incremented*

---

## Shadow Volume Algorithm (Part II)

• Draw the **back faces** of the shadow volume. Decrement the stencil value for every pixel that passes the depth test (glStencilOp(GL_KEEP, GL_KEEP, **GL_DECR**))

**Stencil Buffer – Back Pass**

```
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
0 0 1 1 1 1 0 0
0 0 1 1 1 1 0 0
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
```

*Stencil value for fragments behind the shadow volume's back faces will be decremented*

*Fragments outside the volume will have zero stencil value*

*Fragments inside the volume will have one stencil value*

---

## Shadow Volume Algorithm (Part III)

• Enable stencil testing such that **pixels whose stencil value is zero will be rendered**

• Enable writing to the depth buffer and clear it

• Enable the point light source

• Enable **additive blending** so the contribution of passing pixels will be added to the previous ambient values (glBlendFunc(GL_ONE, GL_ONE))

**Color Buffer** (showing lit object pixels with values like 150)  
**Depth Buffer** (reset to 1.0 everywhere except object at 0.5)  
**Stencil Buffer** (with 1s marking shadowed region)

---

## Shadow Volume Algorithm

• No blending versus blending:

![Two comparison images of a teapot - left shows harsh shadow cutoff, right shows proper blended lighting between shadowed and lit areas](blending comparison)

*From John Tsiombikas*

---

## Used Applications

• Doom 3 is the most well-known example (source code available at: https://github.com/id-Software/DOOM-3-BFG)

![Two screenshots from Doom 3 showing dramatic shadow volumes from monsters and characters](Doom 3 examples)

*From http://http.developer.nvidia.com/GPUGems/gpugems_ch09.html*

---

## Pros and Cons

• Requires **preprocessing** of the scene geometry to create the shadow volume

• Needs to be updated if lights and/or objects move

• Can be time consuming for complex geometry

• Requires **multiple rendering passes** for each frame
  – Ambient pass
  – For each light:
    • Compute SV
    • SV front-face pass
    • SV back-face pass
    • Final light source pass
    
• **No need to update shadow volume if only the camera moves**

---

## Pros and Cons

• Inaccurate models can cause **leaking artifacts**

![Image of a robot/mecha character with visible incorrect shadow streaks](leaking artifact example)

*From http://http.developer.nvidia.com/GPUGems/gpugems_ch09.html*

---

## Pros and Cons

• For speed-up, two versions of objects can be created:
  – **High polygon version** is used for actual rendering of the object
  – **Low polygon version** is used to cast shadows of the object

![Three views of a rabbit model showing: high-poly smooth version, medium-poly version, low-poly angular version for shadows](LOD comparison)

*From wikipedia.com*

---

## Pros and Cons

• The basic algorithm has variants to deal with:
  – Camera inside the shadow volume
    • Use **depth-fail** (Carmack's reverse)
    • Increment/decrement the stencil when the shadow volume **fails** the depth test
    
```cpp
// Front faces
glStencilOp(GL_KEEP, GL_INCR_WRAP, GL_KEEP);

// Back faces
glStencilOp(GL_KEEP, GL_DECR_WRAP, GL_KEEP);
```

  – Multiple light sources and multiple shadow casters
  – Transparent shadow casters
  
• Further reading:
  – http://http.developer.nvidia.com/GPUGems/gpugems_ch09.html
  – http://www.angelfire.com/games5/duktroa/RealTimeShadowTutorial.htm

---

## Shadow Mapping

• The idea introduced by Lance Williams in his 1978 paper "Casting Curved Shadows on Curved Surfaces"

• **Image space technique**

• **Advantages:**
  – No knowledge or processing of scene geometry is required
  – No need to use stencil buffer
  – Fewer rendering passes than shadow volumes
  
• **Disadvantages:**
  – Still requires multiple passes
  – Aliasing artifacts may occur (shadows may look jaggy)

---

## Shadow Mapping

• **Part I:** Render the scene from the point of view of the light source (as if the light source was a camera)
  – Objects that are **not visible** are **in shadow** with respect to that light source
  
• **Part II:** Determine whether an object as seen from the camera is in shadow in the "light's view"

---

## Part I: Rendering from the Light Source

• Pretend that there is a camera at the light position

• Use **perspective** projection for spot (and point) lights

• Use **orthographic** projection for directional lights

![Two side-by-side images: left shows original camera view of Greek temple, right shows view from light's perspective looking down at temple](view comparison)

*Original camera view* | *Light source view*

---

## Part I: Extracting the Depth Map

• Actually, we only need the **depth buffer** values from the light source view

• Save this depth buffer to an **off-screen texture** (FBOs)

![Grayscale depth map showing the temple from light's view - brighter areas are closer to light](depth map visualization)

*Depth map from the light's view*

• *1 – z* is shown for visualization purposes
• Normally, *z* is larger for further away points

---

## Part II: Rendering from the Camera

• Render the scene from the camera view as usual

• For every pixel, compare the depth value of that pixel w.r.t. the light source (**R**) to the stored value in the depth texture (**D**):

**R = D:** Object was directly visible from the light source  
**R > D:** Object was behind another object in the light's view

• But there is a problem:
  – Pixel (i, j) in the **camera view** will **not** belong to the same object as pixel (i, j) in the **light view** as they look at the scene from **different positions** (and with **different orientations**)

---

## Projective Texturing

• For every pixel in the camera view, we need to find the corresponding pixel in the light's view
  – Transform the camera view coordinate to the light view coordinate

![Diagram showing camera view and light view (labeled as "Carlight") with a car and how a point in one view maps to the other](projective texture illustration)

*From http://www.riemers.net/images/Tutorials/DirectX/Csharp/Series3*

---

## Projective Texturing

• Assume **inPos** represents the world coordinates of an object. Its camera coordinates are computed by:

```cpp
outPosCamera = cameraWorldViewProjection * inPos;
```

• Its light view coordinates are computed by:

```cpp
outPosLight = lightWorldViewProjection * inPos;
```

• We can use **outPosLight** to look up the depth texture we generated in Part I

• But this value is in range [-1,1] in all axis (CVV)
  – Need to transform it to the **normalized viewport coordinates**

---

## Projective Texturing - Bias

• To do so we multiply it with a *bias* matrix to bring all components to [0, 1] range:

```
        ⎡0.5  0    0    0.5⎤
        ⎢ 0   0.5  0    0.5⎥
bias =  ⎢ 0    0   0.5  0.5⎥
        ⎣ 0    0    0    1 ⎦
```

```cpp
outPosLight = bias * lightWorldViewProjection * inPos;
```

Use (x, y) for depth texture lookup → Use z for z-value w.r.t. the light (z = R)

---

## Shadow Check

• Remember that **R** is the depth value of the pixel from the light's view and **D** is the stored depth in the texture:

**R = D:** Object was directly visible from the light source  
**R > D:** Object was behind another object in the light's view

• **R** and **D** can be found by:

```cpp
outPosLight /= outPosLight.w          // Perspective divide
R = outputPosLight.z
D = texLookUp(shadowMap, outputPosLight.xy)
```

• Now we can perform the shadow check:

```cpp
if (R > (D + 0.00001))                // Darkening factor
    Output.Color = Input.Color * 0.5;
else
    Output.Color = Input.Color;
```

*Note that a small bias is added to avoid self-shadowing just like as in ray tracing*

---

## Shadow Map Resolution

• If the depth texture we create in Part I does not have enough resolution, we can see **blocking artifacts:**

![Two side-by-side images showing cubes and their shadows - left image (160x120) shows blocky pixelated shadows, right image (1280x960) shows smooth shadows](resolution comparison)

*160x120 shadow map* | *1280x960 shadow map*

*From http://bytechunk.net*

---

## Projection Artifacts

• If an object falls **outside** the viewing frustum of the light, we can see artifacts with a naïve implementation:
  – Mostly happens with directional lights
  – Treat as unshadowed (light reaches to the point) in this case
  – Shouldn't happen for point lights if we render multiple images to contain all possible directions (like a cubemap)

![Image showing incorrect shadow artifacts where shadows appear in wrong locations](projection artifact)

---

## Improving the Shadow Quality

• Shadow map can be filtered in various ways to create **soft shadows:**

![Three comparison images showing shadow edge quality: blurred soft shadows, percentage closer filtering, and hard-edged normal shadow mapping](filtering comparison)

*Screen Space Blur | 3x3 Percentage Closer Filtering | Normal Shadow Mapping*

*From http://www.gamedev.net*

---

## Applications Using Shadow Maps

• Most games use shadow mapping. Some examples:

![Collection of game screenshots showing shadow mapping in various games:
- Rage: Post-apocalyptic scene with vehicle
- Riddick 2: Character in dark environment
- Dragon Age: Marketplace with carousel
- Assassin's Creed: Action scene on rooftops](game examples)

---