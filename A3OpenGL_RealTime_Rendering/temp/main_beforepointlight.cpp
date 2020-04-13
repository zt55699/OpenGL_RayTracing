#include "assignment.hpp"

const GLint WIDTH = 1920, HEIGHT = 1080;

// ===--------------POINTLIGHT----------------===
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
Pointlight::Pointlight()
{
    // allocate the memory to hold the program and shader data
    mProgramHandle = glCreateProgram();
    mVertHandle = glCreateShader(GL_VERTEX_SHADER);
    mFragHandle = glCreateShader(GL_FRAGMENT_SHADER);
}

void Pointlight::loadShaders()
{
    std::string shaderRoot{ ShaderPath };
    vertexSource =
        glx::readShaderSource(shaderRoot + "point_light.vert", IncludeDir);
    fragmentSource =
        glx::readShaderSource(shaderRoot + "point_light.frag", IncludeDir);

    if (auto result{ glx::compileShader(vertexSource.sourceString, mVertHandle) };
        result)
    {
        throw OpenGLError(*result);
    }

    if (auto result =
        glx::compileShader(fragmentSource.sourceString, mFragHandle);
        result)
    {
        throw OpenGLError(*result);
    }

    // communicate to OpenGL the shaders used to render the Triangle
    glAttachShader(mProgramHandle, mVertHandle);
    glAttachShader(mProgramHandle, mFragHandle);

    if (auto result = glx::linkShaders(mProgramHandle); result)
    {
        throw OpenGLError(*result);
    }

    setupUniformVariables();
}

void Pointlight::loadDataToGPU(std::array<float, 216> const& vertices)
{
    // create buffer to hold triangle vertex data
    glCreateBuffers(1, &mVbo);
    // allocate and initialize buffer to vertex data
    glNamedBufferStorage(
        mVbo, glx::size<float>(vertices.size()), vertices.data(), 0);

    // create holder for all buffers
    glCreateVertexArrays(1, &mVao);
    // bind vertex buffer to the vertex array
    glVertexArrayVertexBuffer(mVao, 0, mVbo, 0, glx::stride<float>(6));

    // enable attributes for the two components of a vertex
    glEnableVertexArrayAttrib(mVao, 0);
    glEnableVertexArrayAttrib(mVao, 1);

    // specify to OpenGL how the vertices and colors are laid out in the buffer
    glVertexArrayAttribFormat(
        mVao, 0, 3, GL_FLOAT, GL_FALSE, glx::relativeOffset<float>(0));
    glVertexArrayAttribFormat(
        mVao, 1, 3, GL_FLOAT, GL_FALSE, glx::relativeOffset<float>(3));

    // associate the vertex attributes (coordinates and color) to the vertex
    // attribute
    glVertexArrayAttribBinding(mVao, 0, 0);
    glVertexArrayAttribBinding(mVao, 1, 0);
}

void Pointlight::reloadShaders()
{
    if (glx::shouldShaderBeReloaded(vertexSource))
    {
        glx::reloadShader(
            mProgramHandle, mVertHandle, vertexSource, IncludeDir);
    }

    if (glx::shouldShaderBeReloaded(fragmentSource))
    {
        glx::reloadShader(
            mProgramHandle, mFragHandle, fragmentSource, IncludeDir);
    }
}

void Pointlight::render(
    [[maybe_unused]] int width,
    [[maybe_unused]] int height)
{
    reloadShaders();

    auto projMat{ glm::perspective(glm::radians(60.0f),static_cast<float>(width) / height, nearVal, farVal) }; // define projection matrix
    auto viewMat{ glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp) }; // define view matrix
    auto modelMat{ glm::translate(math::Matrix4{1.0f}, math::Vector(lightPos)) };
    auto modelMat{ glm::scale(math::Matrix4{1.0f}, glm::vec3(0.2f))};

    // MVP = P*V*M
    // gl_position = MVP * vec4(position, 1.0);  // add 1 row for 3x3 position matrix


    // tell OpenGL which program object to use to render the Pointlight
    glUseProgram(mProgramHandle);

    glUniformMatrix4fv(
        mUniformProjectionLoc, 1, GL_FALSE, glm::value_ptr(projMat)
    );
    glUniformMatrix4fv(
        mUniformViewLoc, 1, GL_FALSE, glm::value_ptr(viewMat)
    );
    glUniformMatrix4fv(
        mUniformModelLoc, 1, GL_FALSE, glm::value_ptr(modelMat)
    );
    // tell OpenGL which vertex array object to use to render the Triangle
    glBindVertexArray(mVao);
    // actually render the Triangle
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void Pointlight::freeGPUData()
{
    // unwind all the allocations made
    glDeleteVertexArrays(1, &mVao);
    glDeleteBuffers(1, &mVbo);
    glDeleteShader(mFragHandle);
    glDeleteShader(mVertHandle);
    glDeleteProgram(mProgramHandle);
}

void Pointlight::setupUniformVariables()
{
    mUniformModelLoc = glGetUniformLocation(mProgramHandle, "model");
    // do 2 more times for other matrices
    mUniformViewLoc = glGetUniformLocation(mProgramHandle, "view");
    mUniformProjectionLoc = glGetUniformLocation(mProgramHandle, "proj");

}







// ===--------------CAMERA----------------===
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 4.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
GLfloat yaw = -90.0f;	
GLfloat pitch = 0.0f;
bool keys[1024];
void move_camera()
{
    GLfloat cameraSpeed = 0.05f;
    if (keys[GLFW_KEY_W])
        cameraPos += cameraSpeed * cameraFront;
    if (keys[GLFW_KEY_S])
        cameraPos -= cameraSpeed * cameraFront;
    if (keys[GLFW_KEY_A])
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (keys[GLFW_KEY_D])
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}




// ===---------------CUBE-----------------===

Cube::Cube()
{
    // allocate the memory to hold the program and shader data
    mProgramHandle = glCreateProgram();
    mVertHandle = glCreateShader(GL_VERTEX_SHADER);
    mFragHandle = glCreateShader(GL_FRAGMENT_SHADER);
}

void Cube::loadShaders()
{
    std::string shaderRoot{ ShaderPath };
    vertexSource =
        glx::readShaderSource(shaderRoot + "cube.vert", IncludeDir);
    fragmentSource =
        glx::readShaderSource(shaderRoot + "cube.frag", IncludeDir);

    if (auto result{ glx::compileShader(vertexSource.sourceString, mVertHandle) };
        result)
    {
        throw OpenGLError(*result);
    }

    if (auto result =
        glx::compileShader(fragmentSource.sourceString, mFragHandle);
        result)
    {
        throw OpenGLError(*result);
    }

    // communicate to OpenGL the shaders used to render the Triangle
    glAttachShader(mProgramHandle, mVertHandle);
    glAttachShader(mProgramHandle, mFragHandle);

    if (auto result = glx::linkShaders(mProgramHandle); result)
    {
        throw OpenGLError(*result);
    }

    setupUniformVariables();
}

void Cube::loadDataToGPU(std::array<float, 216> const& vertices)
{
    // create buffer to hold triangle vertex data
    glCreateBuffers(1, &mVbo);
    // allocate and initialize buffer to vertex data
    glNamedBufferStorage(
        mVbo, glx::size<float>(vertices.size()), vertices.data(), 0);

    // create holder for all buffers
    glCreateVertexArrays(1, &mVao);
    // bind vertex buffer to the vertex array
    glVertexArrayVertexBuffer(mVao, 0, mVbo, 0, glx::stride<float>(6));

    // enable attributes for the two components of a vertex
    glEnableVertexArrayAttrib(mVao, 0);
    glEnableVertexArrayAttrib(mVao, 1);

    // specify to OpenGL how the vertices and colors are laid out in the buffer
    glVertexArrayAttribFormat(
        mVao, 0, 3, GL_FLOAT, GL_FALSE, glx::relativeOffset<float>(0));
    glVertexArrayAttribFormat(
        mVao, 1, 3, GL_FLOAT, GL_FALSE, glx::relativeOffset<float>(3));

    // associate the vertex attributes (coordinates and color) to the vertex
    // attribute
    glVertexArrayAttribBinding(mVao, 0, 0);
    glVertexArrayAttribBinding(mVao, 1, 0);
}

void Cube::reloadShaders()
{
    if (glx::shouldShaderBeReloaded(vertexSource))
    {
        glx::reloadShader(
            mProgramHandle, mVertHandle, vertexSource, IncludeDir);
    }

    if (glx::shouldShaderBeReloaded(fragmentSource))
    {
        glx::reloadShader(
            mProgramHandle, mFragHandle, fragmentSource, IncludeDir);
    }
}

void Cube::render([[maybe_unused]] bool paused,
    [[maybe_unused]] int width,
    [[maybe_unused]] int height)
{
    reloadShaders();

    auto projMat{ glm::perspective(glm::radians(60.0f),static_cast<float>(width) / height, nearVal, farVal) }; // define projection matrix
    auto viewMat{ glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp) }; // define view matrix

    if (!paused) {
        position = static_cast<float>(glfwGetTime()) * 64.0f;  // something different than it was
    }

    auto modelMat{ glm::rotate(math::Matrix4{1.0f},
                    glm::radians(position),
                    math::Vector{0.0f, 1.0f, 0.0f}) }; // define model matrix

    // MVP = P*V*M
    // gl_position = MVP * vec4(position, 1.0);  // add 1 row for 3x3 position matrix


    // tell OpenGL which program object to use to render the Triangle
    glUseProgram(mProgramHandle);

    glUniformMatrix4fv(
        mUniformProjectionLoc, 1, GL_FALSE, glm::value_ptr(projMat)
    );
    glUniformMatrix4fv(
        mUniformViewLoc, 1, GL_FALSE, glm::value_ptr(viewMat)
    );
    glUniformMatrix4fv(
        mUniformModelLoc, 1, GL_FALSE, glm::value_ptr(modelMat)
    );
    // tell OpenGL which vertex array object to use to render the Triangle
    glBindVertexArray(mVao);
    // actually render the Triangle
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void Cube::freeGPUData()
{
    // unwind all the allocations made
    glDeleteVertexArrays(1, &mVao);
    glDeleteBuffers(1, &mVbo);
    glDeleteShader(mFragHandle);
    glDeleteShader(mVertHandle);
    glDeleteProgram(mProgramHandle);
}

void Cube::setupUniformVariables()
{
    mUniformModelLoc = glGetUniformLocation(mProgramHandle, "model");
    // do 2 more times for other matrices
    mUniformViewLoc = glGetUniformLocation(mProgramHandle, "view");
    mUniformProjectionLoc = glGetUniformLocation(mProgramHandle, "proj");

}

// ===------------IMPLEMENTATIONS-------------===

Program::Program(int width, int height, std::string title) :
    settings{}, callbacks{}, paused{}, mWindow{ nullptr }
{
    settings.size.width = width;
    settings.size.height = height;
    settings.title = title;

    if (!glx::initializeGLFW(errorCallback))
    {
        throw OpenGLError("Failed to initialize GLFW with error callback");
    }

    mWindow = glx::createGLFWWindow(settings);
    if (mWindow == nullptr)
    {
        throw OpenGLError("Failed to create GLFW Window");
    }

    glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    callbacks.keyPressCallback = [&](int key, int, int action, int) {
        if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {		//look at the documentation of GLFW_KEY_SPACE and RELEASE
            paused = !paused;
        }
        if (key >= 0 && key <= 1024){
            if (action == GLFW_PRESS)
                keys[key] = true;
            else if (action == GLFW_RELEASE)
                keys[key] = false;
        }
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, GL_TRUE);
    };


    callbacks.mouseMoveCallback = [&](double xpos, double ypos){
        GLfloat xoffset = (GLfloat)(xpos - lastX);
        GLfloat yoffset = (GLfloat)(lastY - ypos);		// Reversed since y-coordinates range from bottom to top
        lastX = (GLfloat)xpos;
        lastY = (GLfloat)ypos;

        GLfloat move_speed = 0.15f;	// Change this mouse move speed
        xoffset *= move_speed;
        yoffset *= move_speed;

        yaw += xoffset;
        pitch += yoffset;

        // keep pitch in bound, prevent screen flip
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
          
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);
    };

    createGLContext();
}

void Program::run(Cube& cub)
{
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(mWindow))
    {
        int width;
        int height;

        glfwGetFramebufferSize(mWindow, &width, &height);

        move_camera();
        // setup the view to be the window's size
        glViewport(0, 0, width, height);
        // tell OpenGL the what color to clear the screen to
        glClearColor(0, 0, 0, 1);
        // actually clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cub.render(paused, width, height);

        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }
}

void Program::freeGPUData()
{
    glx::destroyGLFWWindow(mWindow);
    glx::terminateGLFW();
}

void Program::createGLContext()
{
    using namespace magic_enum::bitwise_operators;

    glx::bindWindowCallbacks(mWindow, callbacks);
    glfwMakeContextCurrent(mWindow);
    glfwSwapInterval(1);

    if (!glx::createGLContext(mWindow, settings.version))
    {
        throw OpenGLError("Failed to create OpenGL context");
    }

    glx::initializeGLCallback(glx::ErrorSource::All,
        glx::ErrorType::All,
        glx::ErrorSeverity::High |
        glx::ErrorSeverity::Medium);
}

// ===-----------------DRIVER-----------------===

int main()
{
    try
    {
        // clang-format off
        std::array<float, 216> vertices
        {
            // Vertices          Colours
           -0.3f, -0.3f, -0.3f,   1.0f, 0.0f, 0.0f, //front
            0.3f, -0.3f, -0.3f,   0.0f, 1.0f, 0.0f,
            0.3f,  0.3f, -0.3f,   0.0f, 0.0f, 1.0f,
            0.3f,  0.3f, -0.3f,   1.0f, 0.0f, 0.0f,
           -0.3f,  0.3f, -0.3f,   0.0f, 1.0f, 0.0f,
           -0.3f, -0.3f, -0.3f,   0.0f, 0.0f, 1.0f,

           - 0.3f, -0.3f,  0.3f,   1.0f, 0.0f, 0.0f, //back
           0.3f, -0.3f,  0.3f,   0.0f, 1.0f, 0.0f,
            0.3f,  0.3f,  0.3f,   0.0f, 0.0f, 1.0f,
            0.3f,  0.3f,  0.3f,   1.0f, 0.0f, 0.0f,
           -0.3f,  0.3f,  0.3f,   0.0f, 1.0f, 0.0f,
           -0.3f, -0.3f,  0.3f,   0.0f, 0.0f, 1.0f,

           - 0.3f,  0.3f,  0.3f,   1.0f, 0.0f, 0.0f, //left
           -0.3f,  0.3f, -0.3f,   0.0f, 1.0f, 0.0f,
             -0.3f, -0.3f, -0.3f,   0.0f, 0.0f, 1.0f,
           -0.3f, -0.3f, -0.3f,   1.0f, 0.0f, 0.0f,
           -0.3f, -0.3f,  0.3f,   0.0f, 1.0f, 0.0f,
           -0.3f,  0.3f,  0.3f,   0.0f, 0.0f, 1.0f,

           0.3f,  0.3f,  0.3f,   1.0f, 0.0f, 0.0f, //right
           0.3f,  0.3f, -0.3f,   0.0f, 1.0f, 0.0f,
            0.3f, -0.3f, -0.3f,   0.0f, 0.0f, 1.0f,
           0.3f, -0.3f, -0.3f,   1.0f, 0.0f, 0.0f,
           0.3f, -0.3f,  0.3f,   0.0f, 1.0f, 0.0f,
           0.3f,  0.3f,  0.3f,   0.0f, 0.0f, 1.0f,

           -0.3f, -0.3f, -0.3f,   1.0f, 0.0f, 0.0f, //bottom
            0.3f, -0.3f, -0.3f,   0.0f, 1.0f, 0.0f,
            0.3f, -0.3f,  0.3f,   0.0f, 0.0f, 1.0f,
            0.3f, -0.3f,  0.3f,   1.0f, 0.0f, 0.0f,
           -0.3f, -0.3f,  0.3f,   0.0f, 1.0f, 0.0f,
           -0.3f, -0.3f, -0.3f,   0.0f, 0.0f, 1.0f,

           -0.3f,  0.3f, -0.3f,   1.0f, 0.0f, 0.0f, //top
            0.3f,  0.3f, -0.3f,   0.0f, 1.0f, 0.0f,
            0.3f,  0.3f,  0.3f,   0.0f, 0.0f, 1.0f,
           0.3f,  0.3f,  0.3f,   1.0f, 0.0f, 0.0f,
          -0.3f,  0.3f,  0.3f,   0.0f, 1.0f, 0.0f,
          -0.3f,  0.3f, -0.3f,   0.0f, 0.0f, 1.0f

        };
        // clang-format on

        Program prog{ WIDTH, HEIGHT, "CSC305 Ass 3" };
        Cube cub{};

        cub.loadShaders();
        cub.loadDataToGPU(vertices);

        prog.run(cub);

        prog.freeGPUData();
        cub.freeGPUData();

    }
    catch (OpenGLError & err)
    {
        fmt::print("OpenGL Error:\n\t{}\n", err.what());
    }

    return 0;
}









/*


// ===---------------TRIANGLE-----------------===

Triangle::Triangle()
{
    // allocate the memory to hold the program and shader data
    mProgramHandle = glCreateProgram();
    mVertHandle = glCreateShader(GL_VERTEX_SHADER);
    mFragHandle = glCreateShader(GL_FRAGMENT_SHADER);
}

void Triangle::loadShaders()
{
    std::string shaderRoot{ ShaderPath };
    vertexSource =
        glx::readShaderSource(shaderRoot + "triangle.vert", IncludeDir);
    fragmentSource =
        glx::readShaderSource(shaderRoot + "triangle.frag", IncludeDir);

    if (auto result{ glx::compileShader(vertexSource.sourceString, mVertHandle) };
        result)
    {
        throw OpenGLError(*result);
    }

    if (auto result =
        glx::compileShader(fragmentSource.sourceString, mFragHandle);
        result)
    {
        throw OpenGLError(*result);
    }

    // communicate to OpenGL the shaders used to render the Triangle
    glAttachShader(mProgramHandle, mVertHandle);
    glAttachShader(mProgramHandle, mFragHandle);

    if (auto result = glx::linkShaders(mProgramHandle); result)
    {
        throw OpenGLError(*result);
    }

    setupUniformVariables();
}

void Triangle::loadDataToGPU(std::array<float, 18> const& vertices)
{
    // create buffer to hold triangle vertex data
    glCreateBuffers(1, &mVbo);
    // allocate and initialize buffer to vertex data
    glNamedBufferStorage(
        mVbo, glx::size<float>(vertices.size()), vertices.data(), 0);

    // create holder for all buffers
    glCreateVertexArrays(1, &mVao);
    // bind vertex buffer to the vertex array
    glVertexArrayVertexBuffer(mVao, 0, mVbo, 0, glx::stride<float>(6));

    // enable attributes for the two components of a vertex
    glEnableVertexArrayAttrib(mVao, 0);
    glEnableVertexArrayAttrib(mVao, 1);

    // specify to OpenGL how the vertices and colors are laid out in the buffer
    glVertexArrayAttribFormat(
        mVao, 0, 3, GL_FLOAT, GL_FALSE, glx::relativeOffset<float>(0));
    glVertexArrayAttribFormat(
        mVao, 1, 3, GL_FLOAT, GL_FALSE, glx::relativeOffset<float>(3));

    // associate the vertex attributes (coordinates and color) to the vertex
    // attribute
    glVertexArrayAttribBinding(mVao, 0, 0);
    glVertexArrayAttribBinding(mVao, 1, 0);
}

void Triangle::reloadShaders()
{
    if (glx::shouldShaderBeReloaded(vertexSource))
    {
        glx::reloadShader(
            mProgramHandle, mVertHandle, vertexSource, IncludeDir);
    }

    if (glx::shouldShaderBeReloaded(fragmentSource))
    {
        glx::reloadShader(
            mProgramHandle, mFragHandle, fragmentSource, IncludeDir);
    }
}

void Triangle::render([[maybe_unused]] bool paused,
    [[maybe_unused]] int width,
    [[maybe_unused]] int height)
{
    reloadShaders();

    auto projMat{ glm::perspective(glm::radians(60.0f),static_cast<float>(width) / height, nearVal, farVal) }; // define projection matrix
    auto viewMat{ glm::lookAt(glm::vec3{1.0f, 0.0f, 1.0f},
                               glm::vec3{0.0f, 0.0f, 0.0f},
                               glm::vec3{0.0f, 1.0f, 0.0f}) }; // define view matrix

    if (!paused) {
        position = static_cast<float>(glfwGetTime()) * 64.0f;  // something different than it was
    }

    auto modelMat{ glm::rotate(math::Matrix4{1.0f},
                    glm::radians(position),
                    math::Vector{0.0f, 1.0f, 0.0f}) }; // define model matrix

    // MVP = P*V*M
    // gl_position = MVP * vec4(position, 1.0);  // add 1 row for 3x3 position matrix


    // tell OpenGL which program object to use to render the Triangle
    glUseProgram(mProgramHandle);

    glUniformMatrix4fv(
        mUniformProjectionLoc, 1, GL_FALSE, glm::value_ptr(projMat)
    );
    glUniformMatrix4fv(
        mUniformViewLoc, 1, GL_FALSE, glm::value_ptr(viewMat)
    );
    glUniformMatrix4fv(
        mUniformModelLoc, 1, GL_FALSE, glm::value_ptr(modelMat)
    );
    // tell OpenGL which vertex array object to use to render the Triangle
    glBindVertexArray(mVao);
    // actually render the Triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Triangle::freeGPUData()
{
    // unwind all the allocations made
    glDeleteVertexArrays(1, &mVao);
    glDeleteBuffers(1, &mVbo);
    glDeleteShader(mFragHandle);
    glDeleteShader(mVertHandle);
    glDeleteProgram(mProgramHandle);
}

void Triangle::setupUniformVariables()
{
    mUniformModelLoc = glGetUniformLocation(mProgramHandle, "model");
    // do 2 more times for other matrices
    mUniformViewLoc = glGetUniformLocation(mProgramHandle, "view");
    mUniformProjectionLoc = glGetUniformLocation(mProgramHandle, "proj");

}

// ===------------IMPLEMENTATIONS-------------===

Program::Program(int width, int height, std::string title) :
    settings{}, callbacks{}, paused{}, mWindow{ nullptr }
{
    settings.size.width = width;
    settings.size.height = height;
    settings.title = title;

    if (!glx::initializeGLFW(errorCallback))
    {
        throw OpenGLError("Failed to initialize GLFW with error callback");
    }

    mWindow = glx::createGLFWWindow(settings);
    if (mWindow == nullptr)
    {
        throw OpenGLError("Failed to create GLFW Window");
    }

    callbacks.keyPressCallback = [&](int key, int, int action, int) {
        if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {		//look at the documentation of GLFW_KEY_SPACE and RELEASE
            paused = !paused;
        }
    };

    createGLContext();
}

void Program::run(Triangle& tri)
{
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(mWindow))
    {
        int width;
        int height;

        glfwGetFramebufferSize(mWindow, &width, &height);
        // setup the view to be the window's size
        glViewport(0, 0, width, height);
        // tell OpenGL the what color to clear the screen to
        glClearColor(0, 0, 0, 1);
        // actually clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        tri.render(paused, width, height);

        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }
}

void Program::freeGPUData()
{
    glx::destroyGLFWWindow(mWindow);
    glx::terminateGLFW();
}

void Program::createGLContext()
{
    using namespace magic_enum::bitwise_operators;

    glx::bindWindowCallbacks(mWindow, callbacks);
    glfwMakeContextCurrent(mWindow);
    glfwSwapInterval(1);

    if (!glx::createGLContext(mWindow, settings.version))
    {
        throw OpenGLError("Failed to create OpenGL context");
    }

    glx::initializeGLCallback(glx::ErrorSource::All,
        glx::ErrorType::All,
        glx::ErrorSeverity::High |
        glx::ErrorSeverity::Medium);
}



// ===-----------------DRIVER-----------------===

int main()
{
    try
    {
        // clang-format off
        std::array<float, 18> vertices
        {
            // Vertices          Colours
            0.4f, -0.4f, 0.0f,   1.0f, 0.0f, 0.0f,
           -0.4f, -0.4f, 0.0f,   0.0f, 1.0f, 0.0f,
            0.0f,  0.4f, 0.0f,   0.0f, 0.0f, 1.0f
        };
        // clang-format on

        Program prog{ 1280, 720, "CSC305 Ass 3" };
        Triangle tri{};

        tri.loadShaders();
        tri.loadDataToGPU(vertices);

        prog.run(tri);

        prog.freeGPUData();
        tri.freeGPUData();
    }
    catch (OpenGLError & err)
    {
        fmt::print("OpenGL Error:\n\t{}\n", err.what());
    }

    return 0;
}
*/