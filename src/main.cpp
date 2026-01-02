#include <array>
#include <cstdio>

#include "utility.h"

#include <GLFW/glfw3.h>

#include <glm/ext.hpp> // for matrix calculation

// Planet structure
struct Planet {
  glm::vec3 position;
  float scale;
  float orbitRadius;
  float orbitSpeed;
  float rotationSpeed;
  int parentIndex;       // -1 for no parent
  glm::vec3 localOffset; // Offset from parent
};

// Planet data: Earth, Moon1 (orbits Earth), Moon2 (orbits Moon1)
Planet g_planets[3] = {
    // Earth (index 0)
    {glm::vec3(0.0f), 1.0f, 0.0f, 0.0f, 0.2f, -1, glm::vec3(0.0f)},
    // Moon1 (index 1) - orbits Earth
    {glm::vec3(0.0f), 0.3f, 3.0f, 0.5f, 0.3f, 0, glm::vec3(0.0f)},
    // Moon2 (index 2) - orbits Moon1
    {glm::vec3(0.0f), 0.15f, 1.5f, 1.0f, 0.4f, 1, glm::vec3(0.0f)}};

// Planet positions (will be updated with orbit animations)
glm::vec3 g_planetPositions[3] = {glm::vec3(0.0f, 0.0f, 0.0f),
                                  glm::vec3(3.0f, 0.0f, 0.0f),
                                  glm::vec3(4.5f, 0.0f, 0.0f)};

void UpdatePlanetTransforms(float time) {
  // Update Earth (stationary at origin, just rotates)
  g_planets[0].position = glm::vec3(0.0f);
  g_planetPositions[0] = g_planets[0].position;

  // Update Moon1 (orbits around Earth)
  float angle1 = time * g_planets[1].orbitSpeed;
  glm::vec3 moon1Offset(g_planets[1].orbitRadius * glm::cos(angle1), 0.0f,
                        g_planets[1].orbitRadius * glm::sin(angle1));
  g_planets[1].position = g_planets[0].position + moon1Offset;
  g_planetPositions[1] = g_planets[1].position;

  // Update Moon2 (orbits around Moon1)
  float angle2 = time * g_planets[2].orbitSpeed;
  glm::vec3 moon2Offset(g_planets[2].orbitRadius * glm::cos(angle2), 0.0f,
                        g_planets[2].orbitRadius * glm::sin(angle2));
  g_planets[2].position = g_planets[1].position + moon2Offset;
  g_planetPositions[2] = g_planets[2].position;
}

void UpdateCamera(GLState &state, GLFWwindow *wnd, float deltaTime) {
  if (state.cameraMode == 3) // FPS mode
  {
    // WASD movement
    float moveSpeed = 5.0f * deltaTime;
    glm::vec3 forward(glm::cos(state.cameraPitch) * glm::sin(state.cameraYaw),
                      glm::sin(state.cameraPitch),
                      glm::cos(state.cameraPitch) * glm::cos(state.cameraYaw));
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

    if (glfwGetKey(wnd, GLFW_KEY_W) == GLFW_PRESS)
      state.pos += forward * moveSpeed;
    if (glfwGetKey(wnd, GLFW_KEY_S) == GLFW_PRESS)
      state.pos -= forward * moveSpeed;
    if (glfwGetKey(wnd, GLFW_KEY_A) == GLFW_PRESS)
      state.pos -= right * moveSpeed;
    if (glfwGetKey(wnd, GLFW_KEY_D) == GLFW_PRESS)
      state.pos += right * moveSpeed;

    state.gaze = state.pos + forward;
  } else // Orbit mode (modes 0, 1, 2)
  {
    // Get target planet position
    glm::vec3 targetPos = g_planetPositions[state.cameraMode];

    // Calculate camera position relative to target
    float yaw = state.cameraYaw;
    float pitch = state.cameraPitch;
    float dist = state.cameraDistance;

    glm::vec3 offset(dist * glm::cos(pitch) * glm::sin(yaw),
                     dist * glm::sin(pitch),
                     dist * glm::cos(pitch) * glm::cos(yaw));

    state.pos = targetPos + offset;
    state.gaze = targetPos;
  }
}

void MouseMoveCallback(GLFWwindow *wnd, double x, double y) {
  GLState *state = static_cast<GLState *>(glfwGetWindowUserPointer(wnd));

  if (state->mouseLeftPressed) {
    double deltaX = x - state->lastMouseX;
    double deltaY = y - state->lastMouseY;

    // Only allow panning in FPS mode (cameraMode == 3)
    if (state->cameraMode == 3) {
      // Update yaw and pitch based on mouse movement
      state->cameraYaw += static_cast<float>(deltaX) * 0.005f;
      state->cameraPitch -= static_cast<float>(deltaY) * 0.005f;

      // Clamp pitch to avoid gimbal lock
      state->cameraPitch = glm::clamp(state->cameraPitch, -1.5f, 1.5f);
    } else {
      // In orbit mode, rotate around the planet
      state->cameraYaw += static_cast<float>(deltaX) * 0.005f;
      state->cameraPitch -= static_cast<float>(deltaY) * 0.005f;
      state->cameraPitch = glm::clamp(state->cameraPitch, -1.5f, 1.5f);
    }
  }

  state->lastMouseX = x;
  state->lastMouseY = y;
}

void MouseButtonCallback(GLFWwindow *wnd, int button, int action, int) {
  GLState *state = static_cast<GLState *>(glfwGetWindowUserPointer(wnd));

  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      state->mouseLeftPressed = true;
      glfwGetCursorPos(wnd, &state->lastMouseX, &state->lastMouseY);
    } else if (action == GLFW_RELEASE) {
      state->mouseLeftPressed = false;
    }
  }
}

void MouseScrollCallback(GLFWwindow *wnd, double dx, double dy) {
  GLState *state = static_cast<GLState *>(glfwGetWindowUserPointer(wnd));

  // Zoom in/out by adjusting camera distance
  state->cameraDistance -= static_cast<float>(dy) * 0.5f;
  state->cameraDistance = glm::clamp(state->cameraDistance, 1.0f, 100.0f);
}

void FramebufferChangeCallback(GLFWwindow *wnd, int w, int h) {
  GLState *state = static_cast<GLState *>(glfwGetWindowUserPointer(wnd));
  state->width = w;
  state->height = h;
}

void KeyboardCallback(GLFWwindow *wnd, int key, int scancode, int action,
                      int modifier) {
  GLState *state = static_cast<GLState *>(glfwGetWindowUserPointer(wnd));

  if (action == GLFW_RELEASE) {
    // Camera mode switching
    if (key == GLFW_KEY_P) {
      state->cameraMode =
          (state->cameraMode == 3) ? 0 : (state->cameraMode + 1);
    }
    if (key == GLFW_KEY_O) {
      state->cameraMode =
          (state->cameraMode == 0) ? 3 : (state->cameraMode - 1);
    }

    // Time control
    if (key == GLFW_KEY_L) {
      // Accelerate time, successive presses reverse
      if (state->timeScale > 0.0f && state->timeScale < 4.0f) {
        state->timeScale *= 2.0f;
      } else if (state->timeScale >= 4.0f) {
        state->timeScale = -1.0f; // Reverse time
      } else {
        state->timeScale *= 2.0f; // Continue reversing faster
      }
    }
    if (key == GLFW_KEY_K) {
      // Decelerate time
      state->timeScale *= 0.5f;
      if (glm::abs(state->timeScale) < 0.01f) {
        state->timeScale = 0.01f; // Minimum speed
      }
    }

    // Debug mode switching (keep existing functionality)
    uint32_t mode = state->mode;
    if (key == GLFW_KEY_1)
      mode = 0;
    if (key == GLFW_KEY_2)
      mode = 1;
    if (key == GLFW_KEY_3)
      mode = 2;
    if (key == GLFW_KEY_4)
      mode = 3;
    state->mode = mode;
  }
}

int main(int argc, const char *argv[]) {
  GLState state = GLState("Planet Renderer", 1280, 720, CallbackPointersGLFW());
  // Load planet shaders
  ShaderGL planetVShader =
      ShaderGL(ShaderGL::VERTEX, "working_dir/shaders/planet.vert");
  ShaderGL planetFShader =
      ShaderGL(ShaderGL::FRAGMENT, "working_dir/shaders/planet.frag");
  ShaderGL earthFShader =
      ShaderGL(ShaderGL::FRAGMENT, "working_dir/shaders/earth.frag");
  ShaderGL cloudFShader =
      ShaderGL(ShaderGL::FRAGMENT, "working_dir/shaders/cloud.frag");
  // Shadow shaders
  ShaderGL shadowVShader =
      ShaderGL(ShaderGL::VERTEX, "working_dir/shaders/shadow.vert");
  ShaderGL shadowFShader =
      ShaderGL(ShaderGL::FRAGMENT, "working_dir/shaders/shadow.frag");
  // Background shaders
  ShaderGL bgVShader =
      ShaderGL(ShaderGL::VERTEX, "working_dir/shaders/background.vert");
  ShaderGL bgFShader =
      ShaderGL(ShaderGL::FRAGMENT, "working_dir/shaders/background.frag");

  // Load sphere meshes
  MeshGL sphereMesh = MeshGL("working_dir/meshes/sphere_80k.obj");
  MeshGL bgSphere = MeshGL("working_dir/meshes/sphere_80k.obj");

  // Load textures
  TextureGL earthTex = TextureGL("working_dir/textures/2k_earth_daymap.jpg",
                                 TextureGL::LINEAR, TextureGL::REPEAT);
  TextureGL earthSpecTex =
      TextureGL("working_dir/textures/2k_earth_specular_map.png",
                TextureGL::LINEAR, TextureGL::REPEAT);
  TextureGL earthNightTex =
      TextureGL("working_dir/textures/2k_earth_nightmap_alpha.png",
                TextureGL::LINEAR, TextureGL::REPEAT);
  TextureGL earthCloudTex =
      TextureGL("working_dir/textures/2k_earth_clouds_alpha.png",
                TextureGL::LINEAR, TextureGL::REPEAT);
  TextureGL moonTex = TextureGL("working_dir/textures/2k_moon.jpg",
                                TextureGL::LINEAR, TextureGL::REPEAT);
  TextureGL starsTex = TextureGL("working_dir/textures/8k_stars_milky_way.jpg",
                                 TextureGL::LINEAR, TextureGL::REPEAT);

  // Create shadow map framebuffer
  const int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
  GLuint shadowFBO, shadowDepthTex, shadowColorTex;

  // Create FBO
  glGenFramebuffers(1, &shadowFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

  // Create depth texture
  glGenTextures(1, &shadowDepthTex);
  glBindTexture(GL_TEXTURE_2D, shadowDepthTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SHADOW_WIDTH,
               SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         shadowDepthTex, 0);

  // Create color texture (for storing z values)
  glGenTextures(1, &shadowColorTex);
  glBindTexture(GL_TEXTURE_2D, shadowColorTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, SHADOW_WIDTH, SHADOW_HEIGHT, 0,
               GL_RED, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         shadowColorTex, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::printf("Shadow framebuffer not complete!\n");

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Set unchanged state(s)
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  // =============== //
  //   RENDER LOOP   //
  // =============== //
  float lastTime = static_cast<float>(glfwGetTime());

  while (!glfwWindowShouldClose(state.window)) {
    // Poll inputs from the OS via GLFW
    glfwPollEvents();

    // Calculate delta time
    float currentFrameTime = static_cast<float>(glfwGetTime());
    float deltaTime = currentFrameTime - lastTime;
    lastTime = currentFrameTime;

    // Update time
    state.currentTime += deltaTime * state.timeScale;

    // Update planet positions based on current time
    UpdatePlanetTransforms(state.currentTime);

    // Update camera based on mode
    UpdateCamera(state, state.window, deltaTime);

    // Uniform locations
    static constexpr GLuint U_TRANSFORM_MODEL = 0;
    static constexpr GLuint U_TRANSFORM_VIEW = 1;
    static constexpr GLuint U_TRANSFORM_PROJ = 2;
    static constexpr GLuint U_TRANSFORM_NORMAL = 3;
    static constexpr GLuint U_LIGHT_DIR = 4;
    static constexpr GLuint U_LIGHT_COLOR = 5;
    static constexpr GLuint U_EYE_POS = 6;
    static constexpr GLuint U_LIGHT_VP = 7;
    static constexpr GLuint U_USE_SHADOWS = 8;

    // Rotating sun direction
    float sunAngle = state.currentTime * 0.1f;
    glm::vec3 sunDir =
        glm::normalize(glm::vec3(glm::cos(sunAngle), 0.3f, glm::sin(sunAngle)));
    glm::vec3 sunColor = glm::vec3(1.0f, 1.0f, 0.95f);

    // ========================================
    // SHADOW PASS - Render from light's view
    // ========================================
    glm::mat4x4 lightView = glm::lookAt(
        -sunDir * 20.0f, // Light position (far away in opposite direction)
        glm::vec3(0.0f), // Look at origin
        glm::vec3(0.0f, 1.0f, 0.0f) // Up vector
    );
    glm::mat4x4 lightProj = glm::ortho(-8.0f, 8.0f, -8.0f, 8.0f, 0.1f, 50.0f);
    glm::mat4x4 lightVP = lightProj * lightView;

    // Render to shadow FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(999999.0f, 999999.0f, 999999.0f, 1.0f); // Large value

    glUseProgramStages(state.renderPipeline, GL_VERTEX_SHADER_BIT,
                       shadowVShader.shaderId);
    glUseProgramStages(state.renderPipeline, GL_FRAGMENT_SHADER_BIT,
                       shadowFShader.shaderId);
    glBindVertexArray(sphereMesh.vaoId);

    // Render all planets to shadow map
    for (int i = 0; i < 3; i++) {
      glm::mat4x4 model = glm::identity<glm::mat4x4>();
      model = glm::translate(model, g_planets[i].position);
      model = glm::rotate(model, state.currentTime * g_planets[i].rotationSpeed,
                          glm::vec3(0, 1, 0));
      model = glm::scale(model, glm::vec3(g_planets[i].scale));

      glActiveShaderProgram(state.renderPipeline, shadowVShader.shaderId);
      glUniformMatrix4fv(U_TRANSFORM_MODEL, 1, false, glm::value_ptr(model));
      glUniformMatrix4fv(U_LIGHT_VP, 1, false, glm::value_ptr(lightVP));

      glDrawElements(GL_TRIANGLES, sphereMesh.indexCount, GL_UNSIGNED_INT,
                     nullptr);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // ========================================
    // NORMAL PASS - Render to screen
    // ========================================
    glm::mat4x4 proj = glm::perspective(
        glm::radians(50.0f), float(state.width) / float(state.height), 0.01f,
        100.0f);
    glm::mat4x4 view = glm::lookAt(state.pos, state.gaze, state.up);

    glViewport(0, 0, state.width, state.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);

    // ========================================
    // BACKGROUND RENDERING (Stars)
    // ========================================
    glDepthMask(GL_FALSE);    // Don't write to depth buffer
    glDisable(GL_CULL_FACE);  // Inside the sphere, so we see back faces (or
                              // disable culling)
    glDisable(GL_DEPTH_TEST); // Ensure background draws over clear color (since
                              // z=1.0)

    glUseProgramStages(state.renderPipeline, GL_VERTEX_SHADER_BIT,
                       bgVShader.shaderId);
    glUseProgramStages(state.renderPipeline, GL_FRAGMENT_SHADER_BIT,
                       bgFShader.shaderId);
    glBindVertexArray(bgSphere.vaoId);

    // Large sphere centered on camera with orthographic projection
    glm::mat4x4 bgModel = glm::identity<glm::mat4x4>();
    bgModel = glm::translate(bgModel, state.pos);
    bgModel = glm::scale(bgModel, glm::vec3(100.0f));

    glm::mat4x4 bgProj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.01f, 200.0f);

    glActiveShaderProgram(state.renderPipeline, bgVShader.shaderId);
    glUniformMatrix4fv(U_TRANSFORM_MODEL, 1, false, glm::value_ptr(bgModel));
    glUniformMatrix4fv(U_TRANSFORM_VIEW, 1, false, glm::value_ptr(view));
    glUniformMatrix4fv(U_TRANSFORM_PROJ, 1, false, glm::value_ptr(bgProj));

    glActiveShaderProgram(state.renderPipeline, bgFShader.shaderId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, starsTex.textureId);

    glDrawElements(GL_TRIANGLES, bgSphere.indexCount, GL_UNSIGNED_INT, nullptr);

    glDepthMask(GL_TRUE);    // Re-enable depth writing
    glEnable(GL_CULL_FACE);  // Restore for planets
    glEnable(GL_DEPTH_TEST); // Restore depth test

    // ========================================
    // PLANET RENDERING
    // ========================================
    glUseProgramStages(state.renderPipeline, GL_VERTEX_SHADER_BIT,
                       planetVShader.shaderId);
    glBindVertexArray(sphereMesh.vaoId);
    glDisable(GL_CULL_FACE); // Disable culling to see full spheres

    // Bind shadow map
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, shadowColorTex);

    // Render all planets
    for (int i = 0; i < 3; i++) {
      // Calculate model matrix for this planet
      glm::mat4x4 model = glm::identity<glm::mat4x4>();
      model = glm::translate(model, g_planets[i].position);
      model = glm::rotate(model, state.currentTime * g_planets[i].rotationSpeed,
                          glm::vec3(0, 1, 0));
      model = glm::scale(model, glm::vec3(g_planets[i].scale));

      // Normal matrix
      glm::mat3x3 normalMatrix = glm::inverseTranspose(glm::mat3(model));

      // Set vertex shader uniforms
      glActiveShaderProgram(state.renderPipeline, planetVShader.shaderId);
      glUniformMatrix4fv(U_TRANSFORM_MODEL, 1, false, glm::value_ptr(model));
      glUniformMatrix4fv(U_TRANSFORM_VIEW, 1, false, glm::value_ptr(view));
      glUniformMatrix4fv(U_TRANSFORM_PROJ, 1, false, glm::value_ptr(proj));
      glUniformMatrix3fv(U_TRANSFORM_NORMAL, 1, false,
                         glm::value_ptr(normalMatrix));

      // Use Earth shader for Earth, regular planet shader for moons
      if (i == 0) {
        // Use Earth shader with special textures
        glUseProgramStages(state.renderPipeline, GL_FRAGMENT_SHADER_BIT,
                           earthFShader.shaderId);
        glActiveShaderProgram(state.renderPipeline, earthFShader.shaderId);
        glUniform3fv(U_LIGHT_DIR, 1, glm::value_ptr(sunDir));
        glUniform3fv(U_LIGHT_COLOR, 1, glm::value_ptr(sunColor));
        glUniform3fv(U_EYE_POS, 1, glm::value_ptr(state.pos));
        glUniformMatrix4fv(U_LIGHT_VP, 1, false, glm::value_ptr(lightVP));
        glUniform1i(U_USE_SHADOWS, 0); // Disabled for debugging

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, earthTex.textureId);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, earthSpecTex.textureId);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, earthNightTex.textureId);
      } else {
        // Use regular planet shader
        glUseProgramStages(state.renderPipeline, GL_FRAGMENT_SHADER_BIT,
                           planetFShader.shaderId);
        glActiveShaderProgram(state.renderPipeline, planetFShader.shaderId);
        glUniform3fv(U_LIGHT_DIR, 1, glm::value_ptr(sunDir));
        glUniform3fv(U_LIGHT_COLOR, 1, glm::value_ptr(sunColor));
        glUniform3fv(U_EYE_POS, 1, glm::value_ptr(state.pos));
        glUniformMatrix4fv(U_LIGHT_VP, 1, false, glm::value_ptr(lightVP));
        glUniform1i(U_USE_SHADOWS, 0); // Disabled for debugging

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, moonTex.textureId);
      }

      // Draw the planet
      glDrawElements(GL_TRIANGLES, sphereMesh.indexCount, GL_UNSIGNED_INT,
                     nullptr);
    }

    // Render Earth clouds separately - DISABLED FOR DEBUGGING
    /*
    {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDepthMask(GL_FALSE);

      // Cloud model matrix (slightly larger sphere, different rotation speed)
      float cloudScale = 1.01f;
      float cloudRotationSpeed = 0.15f; // Different from Earth's rotation
      glm::mat4x4 cloudModel = glm::identity<glm::mat4x4>();
      cloudModel = glm::translate(cloudModel, g_planets[0].position);
      cloudModel =
          glm::rotate(cloudModel, state.currentTime * cloudRotationSpeed,
                      glm::vec3(0, 1, 0));
      cloudModel =
          glm::scale(cloudModel, glm::vec3(g_planets[0].scale * cloudScale));

      glm::mat3x3 cloudNormalMatrix =
          glm::inverseTranspose(glm::mat3(cloudModel));

      // Set vertex shader uniforms
      glActiveShaderProgram(state.renderPipeline, planetVShader.shaderId);
      glUniformMatrix4fv(U_TRANSFORM_MODEL, 1, false,
                         glm::value_ptr(cloudModel));
      glUniformMatrix4fv(U_TRANSFORM_VIEW, 1, false, glm::value_ptr(view));
      glUniformMatrix4fv(U_TRANSFORM_PROJ, 1, false, glm::value_ptr(proj));
      glUniformMatrix3fv(U_TRANSFORM_NORMAL, 1, false,
                         glm::value_ptr(cloudNormalMatrix));

      // Use cloud shader
      glUseProgramStages(state.renderPipeline, GL_FRAGMENT_SHADER_BIT,
                         cloudFShader.shaderId);
      glActiveShaderProgram(state.renderPipeline, cloudFShader.shaderId);
      glUniform3fv(U_LIGHT_DIR, 1, glm::value_ptr(sunDir));
      glUniform3fv(U_LIGHT_COLOR, 1, glm::value_ptr(sunColor));

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, earthCloudTex.textureId);

      // Draw clouds
      glDrawElements(GL_TRIANGLES, sphereMesh.indexCount, GL_UNSIGNED_INT,
                     nullptr);

      glDepthMask(GL_TRUE);
      glDisable(GL_BLEND);
    }
    */

    glfwSwapBuffers(state.window);
  }
}
