#pragma once

#include "paths.hpp"

#include <exception>
#include <iostream>
#include <string>

#include <atlas/glx/Buffer.hpp>
#include <atlas/glx/Context.hpp>
#include <atlas/glx/ErrorCallback.hpp>
#include <atlas/glx/GLSL.hpp>
#include <atlas/utils/Cameras.hpp>
#include <atlas/utils/LoadObjFile.hpp>

#include <fmt/printf.h>
#include <magic_enum.hpp>

using namespace atlas;

static constexpr float nearVal{ 1.0f };
static constexpr float farVal{ 10000000000.0f };

static const std::vector<std::string> IncludeDir{ ShaderPath };

struct OpenGLError : std::runtime_error
{
    OpenGLError(const std::string& what_arg) : std::runtime_error(what_arg) {};
    OpenGLError(const char* what_arg) : std::runtime_error(what_arg) {};
};

class Pointlight
{
public:
    Pointlight();

    void loadShaders();

    void loadDataToGPU(std::array<float, 324> const& vertices);

    void reloadShaders();

    void render(int width, int height);

    void freeGPUData();

private:
    void setupUniformVariables();

    float position;

    // Vertex buffers.
    GLuint plVao;
    GLuint plVbo;

    // Shader data.
    GLuint plVertHandle;
    GLuint plFragHandle;
    GLuint plProgramHandle;
    glx::ShaderFile vertexSource;
    glx::ShaderFile fragmentSource;

    // Uniform variable data.
    GLuint mUniformModelLoc;
    GLuint mUniformViewLoc;
    GLuint mUniformProjectionLoc;
    GLuint mUniformlightColor;
};

class Cube
{
public:
    Cube();

    void loadShaders();

    void loadDataToGPU(std::array<float, 324> const& vertices);

    void reloadShaders();

    void render(bool paused, int width, int height);

    void freeGPUData();

private:
    void setupUniformVariables();

    float position;

    // Vertex buffers.
    GLuint mVao;
    GLuint mVbo;

    // Shader data.
    GLuint mVertHandle;
    GLuint mFragHandle;
    GLuint mProgramHandle;
    glx::ShaderFile vertexSource;
    glx::ShaderFile fragmentSource;

    // Uniform variable data.
    GLuint mUniformModelLoc;
    GLuint mUniformViewLoc;
    GLuint mUniformProjectionLoc;
    GLuint lightColourLoc;
    GLuint lightPosLoc;
    GLuint objectColorLoc;
    GLuint ambientStrengthLoc;
    GLuint EyeDirectionLoc;
    GLuint ConstantAttenuationLoc;
    GLuint LinearAttenuationLoc;
    GLuint QuadraticAttenuationLoc;
    GLuint ShininessLoc;
    GLuint matDiffuseLoc;
    GLuint matSpecularLoc;
    GLuint matShininessLoc;
    GLuint dirDiffuseLoc;
    GLuint dirDirectLoc;
    GLuint dirSpecLoc;
    GLuint plightDiffuseLoc;
    GLuint plSpecularLoc;
    GLuint viewPosLoc;
};

class Program
{
public:
    Program(int width, int height, std::string title);

    void run(Cube& object, Pointlight& point);


    void freeGPUData();

private:
    static void errorCallback(int code, char const* message)
    {
        fmt::print("error ({}): {}\n", code, message);
    }

    void createGLContext();

    GLFWwindow* mWindow;
    glx::WindowSettings settings;
    glx::WindowCallbacks callbacks;

    bool paused;
};
