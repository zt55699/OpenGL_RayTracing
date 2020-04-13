#pragma warning(disable:4996)
#include "assignment.hpp"

// ===-------------PARAMETERS---------------===
const GLint WIDTH = 1920, HEIGHT = 1080;
const GLfloat ConstantAttenuation = 1.0f;
const GLfloat LinearAttenuation = 0.5f;
const GLfloat QuadraticAttenuation = 3.0f;
const GLfloat Shininess = 35.0f;
const glm::vec3 matdiffuse = glm::vec3(1.0f, 0.5f, 0.31f);
const glm::vec3 matSpecular = glm::vec3(matdiffuse.x+0.4, matdiffuse.y + 0.4, matdiffuse.z + 0.4);
const glm::vec3 ambientStrength = glm::vec3(0.2f, 0.2f, 0.2f);
glm::vec3 pointLightPositions[] = {
    glm::vec3(1.0f, 1.0f, -1.3f),
    glm::vec3(-0.3f, -0.5f, 1.5f),
     glm::vec3(1.0f, 2.0f, 1.5f) }
;
glm::vec3 pointLightDiffuses[] = {
    glm::vec3(3.0f, 3.0f, 3.0f),
    glm::vec3(2.0f, 2.0f, 2.0f),
    glm::vec3(2.3f, 2.3f, 2.3f) }
;
glm::vec3 pointLightSpecs[] = {
    glm::vec3(6.0f, 6.0f, 6.0f),
    glm::vec3(2.3f, 2.3f, 2.3f),
    glm::vec3(2.6f, 2.6f, 2.6f) }
;
glm::vec3 dirDiffuse = glm::vec3(0.0f, 0.4f, 0.0f);
glm::vec3 dirDirect = glm::vec3(3.0f, 0.0f, 0.0f);
glm::vec3 dirSpec = glm::vec3(0.3f, 1.0f, 0.3f);


// ===--------------PINHOLE CAMERA----------------===
glm::vec3 cameraPos = glm::vec3(-0.3f, 2.2f, 3.4f);
glm::vec3 cameraFront = glm::vec3(0.0f, -0.3f, -1.0f);
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



// ===--------------LIGHT CUBE----------------===
Pointlight::Pointlight()
{
    // allocate the memory to hold the program and shader data
    plProgramHandle = glCreateProgram();
    plVertHandle = glCreateShader(GL_VERTEX_SHADER);
    plFragHandle = glCreateShader(GL_FRAGMENT_SHADER);
}

void Pointlight::loadShaders()
{
    std::string shaderRoot{ ShaderPath };
    vertexSource =
        glx::readShaderSource(shaderRoot + "point_light.vert", IncludeDir);
    fragmentSource =
        glx::readShaderSource(shaderRoot + "point_light.frag", IncludeDir);

    if (auto result{ glx::compileShader(vertexSource.sourceString, plVertHandle) };
        result)
    {
        throw OpenGLError(*result);
    }

    if (auto result =
        glx::compileShader(fragmentSource.sourceString, plFragHandle);
        result)
    {
        throw OpenGLError(*result);
    }

    // communicate to OpenGL the shaders used to render the Light Cube
    glAttachShader(plProgramHandle, plVertHandle);
    glAttachShader(plProgramHandle, plFragHandle);

    if (auto result = glx::linkShaders(plProgramHandle); result)
    {
        throw OpenGLError(*result);
    }

    setupUniformVariables();
}

void Pointlight::loadDataToGPU(std::array<float, 324> const& vertices)
{
    // create buffer to hold vertex data
    glCreateBuffers(1, &plVbo);
    // allocate and initialize buffer to vertex data
    glNamedBufferStorage(
        plVbo, glx::size<float>(vertices.size()), vertices.data(), 0);

    // create holder for all buffers
    glCreateVertexArrays(1, &plVao);
    // bind vertex buffer to the vertex array
    glVertexArrayVertexBuffer(plVao, 0, plVbo, 0, glx::stride<float>(9));
    // enable attributes for the two components of a vertex
    glEnableVertexArrayAttrib(plVao, 0);
    // specify to OpenGL how the vertices and colors are laid out in the buffer
    glVertexArrayAttribFormat(
        plVao, 0, 3, GL_FLOAT, GL_FALSE, glx::relativeOffset<float>(0));
    // associate the vertex attributes (coordinates and color) to the vertex
    // attribute
    glVertexArrayAttribBinding(plVao, 0, 0);
}

void Pointlight::reloadShaders()
{
    if (glx::shouldShaderBeReloaded(vertexSource))
    {
        glx::reloadShader(
            plProgramHandle, plVertHandle, vertexSource, IncludeDir);
    }

    if (glx::shouldShaderBeReloaded(fragmentSource))
    {
        glx::reloadShader(
            plProgramHandle, plFragHandle, fragmentSource, IncludeDir);
    }
}

void Pointlight::render(
    [[maybe_unused]] int width,
    [[maybe_unused]] int height)
{
    reloadShaders();

    auto projMat{ glm::perspective(glm::radians(60.0f),static_cast<float>(width) / height, nearVal, farVal) }; // define projection matrix
    auto viewMat{ glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp) }; // define view matrix
    auto modelMat{ glm::translate(math::Matrix4{1.0f}, math::Vector(pointLightPositions[0])) * glm::scale(math::Matrix4{1.0f}, glm::vec3(0.1f)) };
    //auto modelMat{ glm::scale(math::Matrix4{1.0f}, glm::vec3(0.2f))};

    // tell OpenGL which program object to use to render the Pointlight
    glUseProgram(plProgramHandle);

    glUniformMatrix4fv(
        mUniformProjectionLoc, 1, GL_FALSE, glm::value_ptr(projMat)
    );
    glUniformMatrix4fv(
        mUniformViewLoc, 1, GL_FALSE, glm::value_ptr(viewMat)
    );
    glUniformMatrix4fv(
        mUniformModelLoc, 1, GL_FALSE, glm::value_ptr(modelMat)
    );
    glUniform3f(mUniformlightColor, 1.0f, 1.0f, 1.0f);
    
    // tell OpenGL which vertex array object to use to render
    glBindVertexArray(plVao);
    // actually render
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // draw second light cube
    modelMat = { glm::translate(math::Matrix4{1.0f}, math::Vector(pointLightPositions[1])) * glm::scale(math::Matrix4{1.0f}, glm::vec3(0.1f)) };
    glUseProgram(plProgramHandle);
    glUniformMatrix4fv(
        mUniformModelLoc, 1, GL_FALSE, glm::value_ptr(modelMat)
    );
    glBindVertexArray(plVao);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // draw third light cube
    modelMat = { glm::translate(math::Matrix4{1.0f}, math::Vector(pointLightPositions[2])) * glm::scale(math::Matrix4{1.0f}, glm::vec3(0.1f)) };
    glUseProgram(plProgramHandle);
    glUniformMatrix4fv(
        mUniformModelLoc, 1, GL_FALSE, glm::value_ptr(modelMat)
    );
    glBindVertexArray(plVao);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // draw direction light cube
    modelMat = { glm::translate(math::Matrix4{1.0f}, math::Vector(-dirDirect.x,dirDirect.y,dirDirect.z)) * glm::scale(math::Matrix4{1.0f}, glm::vec3(0.001f,0.6f,0.6f)) };
    glUseProgram(plProgramHandle);
    glUniformMatrix4fv(
        mUniformModelLoc, 1, GL_FALSE, glm::value_ptr(modelMat)
    );
    glUniform3f(mUniformlightColor, 0.0f, 1.0f, 0.0f);
    glBindVertexArray(plVao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void Pointlight::freeGPUData()
{
    // unwind all the allocations made
    glDeleteVertexArrays(1, &plVao);
    glDeleteBuffers(1, &plVbo);
    glDeleteShader(plFragHandle);
    glDeleteShader(plVertHandle);
    glDeleteProgram(plProgramHandle);
}

void Pointlight::setupUniformVariables()
{
    mUniformModelLoc = glGetUniformLocation(plProgramHandle, "model");
    mUniformViewLoc = glGetUniformLocation(plProgramHandle, "view");
    mUniformProjectionLoc = glGetUniformLocation(plProgramHandle, "proj");
    mUniformlightColor = glGetUniformLocation(plProgramHandle, "lightColor");
    
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

    // communicate to OpenGL the shaders used to render 
    glAttachShader(mProgramHandle, mVertHandle);
    glAttachShader(mProgramHandle, mFragHandle);

    if (auto result = glx::linkShaders(mProgramHandle); result)
    {
        throw OpenGLError(*result);
    }

    setupUniformVariables();
}

void Cube::loadDataToGPU(std::array<float, 324> const& vertices)
{
    // create buffer to hold  vertex data
    glCreateBuffers(1, &mVbo);
    // allocate and initialize buffer to vertex data
    glNamedBufferStorage(
        mVbo, glx::size<float>(vertices.size()), vertices.data(), 0);

    // create holder for all buffers
    glCreateVertexArrays(1, &mVao);
    // bind vertex buffer to the vertex array
    glVertexArrayVertexBuffer(mVao, 0, mVbo, 0, glx::stride<float>(9));

    // enable attributes for the two components of a vertex
    glEnableVertexArrayAttrib(mVao, 0);
    glEnableVertexArrayAttrib(mVao, 1);
    glEnableVertexArrayAttrib(mVao, 2);

    // specify to OpenGL how the vertices and colors are laid out in the buffer
    glVertexArrayAttribFormat(
        mVao, 0, 3, GL_FLOAT, GL_FALSE, glx::relativeOffset<float>(0));
    glVertexArrayAttribFormat(
        mVao, 1, 3, GL_FLOAT, GL_FALSE, glx::relativeOffset<float>(3));
    glVertexArrayAttribFormat(
        mVao, 2, 3, GL_FLOAT, GL_FALSE, glx::relativeOffset<float>(6));

    // associate the vertex attributes (coordinates and color) to the vertex
    // attribute
    glVertexArrayAttribBinding(mVao, 0, 0);
    glVertexArrayAttribBinding(mVao, 1, 0);
    glVertexArrayAttribBinding(mVao, 2, 0);
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

//    auto modelMat{ math::Matrix4{1.0f} }; // define model matrix
    // MVP = P*V*M
    // gl_position = MVP * vec4(position, 1.0);  // add 1 row for 3x3 position matrix


    // tell OpenGL which program object to use to render the Triangle
    glUseProgram(mProgramHandle);
    glUniform3f(matDiffuseLoc, matdiffuse.x, matdiffuse.y, matdiffuse.z);
    glUniform1f(matShininessLoc, Shininess);
    glUniform3f(matSpecularLoc, matSpecular.x, matSpecular.y, matSpecular.z);
    glUniform3f(dirDiffuseLoc, dirDiffuse.x, dirDiffuse.y, dirDiffuse.z);
    glUniform3f(dirDirectLoc, dirDirect.x, dirDirect.y, dirDirect.z);
    glUniform3f(dirSpecLoc, dirSpec.x, dirSpec.y, dirSpec.z);
    glUniform3f(plightDiffuseLoc, pointLightDiffuses[0].x, pointLightDiffuses[0].y, pointLightDiffuses[0].z);
    glUniform3f(plSpecularLoc, pointLightSpecs[0].x, pointLightSpecs[0].y, pointLightSpecs[0].z);
    glUniform3f(lightPosLoc, pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
    glUniform3f(ambientStrengthLoc, ambientStrength.x, ambientStrength.y, ambientStrength.z);
    glUniform1f(ConstantAttenuationLoc, ConstantAttenuation);
    glUniform1f(LinearAttenuationLoc, LinearAttenuation);
    glUniform1f(QuadraticAttenuationLoc, QuadraticAttenuation);
    glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);

    glUniform3f(glGetUniformLocation(mProgramHandle, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
    glUniform3f(glGetUniformLocation(mProgramHandle, "pointLights[1].diffuse"), pointLightDiffuses[1].x, pointLightDiffuses[1].y, pointLightDiffuses[1].z);
    glUniform3f(glGetUniformLocation(mProgramHandle, "pointLights[1].specular"), pointLightSpecs[1].x, pointLightSpecs[1].y, pointLightSpecs[1].z);
    
    glUniform3f(glGetUniformLocation(mProgramHandle, "pointLights[2].position"), pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
    glUniform3f(glGetUniformLocation(mProgramHandle, "pointLights[2].diffuse"), pointLightDiffuses[2].x, pointLightDiffuses[2].y, pointLightDiffuses[2].z);
    glUniform3f(glGetUniformLocation(mProgramHandle, "pointLights[2].specular"), pointLightSpecs[2].x, pointLightSpecs[2].y, pointLightSpecs[2].z);

    glUniformMatrix4fv(
        mUniformProjectionLoc, 1, GL_FALSE, glm::value_ptr(projMat)
    );
    glUniformMatrix4fv(
        mUniformViewLoc, 1, GL_FALSE, glm::value_ptr(viewMat)
    );
    glUniformMatrix4fv( 
        mUniformModelLoc, 1, GL_FALSE, glm::value_ptr(modelMat)
    );
    // tell OpenGL which vertex array object to use to render 
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
    matDiffuseLoc = glGetUniformLocation(mProgramHandle, "material.diffuse");
    matSpecularLoc = glGetUniformLocation(mProgramHandle, "material.specular");
    matShininessLoc = glGetUniformLocation(mProgramHandle, "material.Shininess");
    dirDiffuseLoc = glGetUniformLocation(mProgramHandle, "dirLight.diffuse");
    dirDirectLoc = glGetUniformLocation(mProgramHandle, "dirLight.direction");
    dirSpecLoc = glGetUniformLocation(mProgramHandle, "dirLight.specular");

    lightColourLoc = glGetUniformLocation(mProgramHandle, "lightColour");
    lightPosLoc = glGetUniformLocation(mProgramHandle, "pointLights[0].position");
    plightDiffuseLoc = glGetUniformLocation(mProgramHandle, "pointLights[0].diffuse");
    plSpecularLoc = glGetUniformLocation(mProgramHandle, "pointLights[0].specular");

    objectColorLoc = glGetUniformLocation(mProgramHandle, "objectColor");
    ambientStrengthLoc = glGetUniformLocation(mProgramHandle, "ambientStrength");
    viewPosLoc = glGetUniformLocation(mProgramHandle, "viewPos");

    ConstantAttenuationLoc = glGetUniformLocation(mProgramHandle, "ConstantAttenuation");
    LinearAttenuationLoc = glGetUniformLocation(mProgramHandle, "LinearAttenuation");
    QuadraticAttenuationLoc = glGetUniformLocation(mProgramHandle, "QuadraticAttenuation");
    ShininessLoc = glGetUniformLocation(mProgramHandle, "Shininess");
   
}


// ===------------LOADING&RENDERING MESH-------------===
Mesh::Mesh()
{
    // allocate the memory to hold the program and shader data
    mProgramHandle = glCreateProgram();
    mVertHandle = glCreateShader(GL_VERTEX_SHADER);
    mFragHandle = glCreateShader(GL_FRAGMENT_SHADER);
}

void Mesh::loadShaders()
{
    std::string shaderRoot{ ShaderPath };
    vertexSource =
        glx::readShaderSource(shaderRoot + "Mesh.vert", IncludeDir);
    fragmentSource =
        glx::readShaderSource(shaderRoot + "Mesh.frag", IncludeDir);

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

void Mesh::loadDataToGPU()
{
    // create buffer to hold triangle vertex data
    printf("vertices size: %zd \n", MeshVertices.size());
    printf("Triangles drawed: %d \n", GLint(this->MeshVertices.size() / 3));
    glCreateBuffers(1, &mVbo);
    // allocate and initialize buffer to vertex data
    glNamedBufferStorage(
        mVbo, glx::size<glm::vec3>(MeshVertices.size()), MeshVertices.data(), 0);

    glCreateBuffers(1, &mVbo2);
    // allocate and initialize buffer to vertex data
    glNamedBufferStorage(
        mVbo2, glx::size<glm::vec3>(normals.size()), normals.data(), 0);

    // create holder for all buffers
    glCreateVertexArrays(1, &mVao);
    // bind vertex buffer to the vertex array
    glVertexArrayVertexBuffer(mVao, 0, mVbo, 0, sizeof(glm::vec3));
    glVertexArrayVertexBuffer(mVao, 2, mVbo2, 0, sizeof(glm::vec3));
    // enable attributes for the two components of a vertex
    glEnableVertexArrayAttrib(mVao, 0); //position
    glEnableVertexArrayAttrib(mVao, 2); //normal

    // specify to OpenGL how the vertices and colors are laid out in the buffer
    glVertexArrayAttribFormat(
        mVao, 0, 3, GL_FLOAT, GL_FALSE, glx::relativeOffset<float>(0));
    glVertexArrayAttribFormat(
        mVao, 2, 3, GL_FLOAT, GL_FALSE, glx::relativeOffset<float>(0));

    // associate the vertex attributes (coordinates and color) to the vertex
    // attribute
    glVertexArrayAttribBinding(mVao, 0, 0);
}

void Mesh::reloadShaders()
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

void Mesh::render([[maybe_unused]] bool paused,
    [[maybe_unused]] int width,
    [[maybe_unused]] int height)
{
    reloadShaders();

    auto projMat{ glm::perspective(glm::radians(60.0f),static_cast<float>(width) / height, nearVal, farVal) }; // define projection matrix
    auto viewMat{ glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp) }; // define view matrix

    if (!paused) {
        position = static_cast<float>(glfwGetTime()) * 64.0f;  // something different than it was
    }

    auto modelMat{ glm::translate(math::Matrix4{1.0f}, math::Vector{0.0, 1.6, 0.0}) * glm::rotate(math::Matrix4{1.0f},
                    glm::radians(position),
                    math::Vector{0.0f, 1.0f, 0.0f}) }; // define model matrix

//    auto modelMat{ math::Matrix4{1.0f} }; // define model matrix
    // MVP = P*V*M
    // gl_position = MVP * vec4(position, 1.0);  // add 1 row for 3x3 position matrix


    // tell OpenGL which program object to use to render the Triangle
    glUseProgram(mProgramHandle);
    glUniform3f(matDiffuseLoc, matdiffuse.x, matdiffuse.y, matdiffuse.z);
    glUniform1f(matShininessLoc, Shininess);
    glUniform3f(matSpecularLoc, matSpecular.x, matSpecular.y, matSpecular.z);
    glUniform3f(dirDiffuseLoc, dirDiffuse.x, dirDiffuse.y, dirDiffuse.z);
    glUniform3f(dirDirectLoc, dirDirect.x, dirDirect.y, dirDirect.z);
    glUniform3f(dirSpecLoc, dirSpec.x, dirSpec.y, dirSpec.z);
    glUniform3f(plightDiffuseLoc, pointLightDiffuses[0].x, pointLightDiffuses[0].y, pointLightDiffuses[0].z);
    glUniform3f(plSpecularLoc, pointLightSpecs[0].x, pointLightSpecs[0].y, pointLightSpecs[0].z);
    glUniform3f(lightPosLoc, pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
    glUniform3f(ambientStrengthLoc, ambientStrength.x, ambientStrength.y, ambientStrength.z);
    glUniform1f(ConstantAttenuationLoc, ConstantAttenuation);
    glUniform1f(LinearAttenuationLoc, LinearAttenuation);
    glUniform1f(QuadraticAttenuationLoc, QuadraticAttenuation);
    glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);

    glUniform3f(glGetUniformLocation(mProgramHandle, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
    glUniform3f(glGetUniformLocation(mProgramHandle, "pointLights[1].diffuse"), pointLightDiffuses[1].x, pointLightDiffuses[1].y, pointLightDiffuses[1].z);
    glUniform3f(glGetUniformLocation(mProgramHandle, "pointLights[1].specular"), pointLightSpecs[1].x, pointLightSpecs[1].y, pointLightSpecs[1].z);

    glUniform3f(glGetUniformLocation(mProgramHandle, "pointLights[2].position"), pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
    glUniform3f(glGetUniformLocation(mProgramHandle, "pointLights[2].diffuse"), pointLightDiffuses[2].x, pointLightDiffuses[2].y, pointLightDiffuses[2].z);
    glUniform3f(glGetUniformLocation(mProgramHandle, "pointLights[2].specular"), pointLightSpecs[2].x, pointLightSpecs[2].y, pointLightSpecs[2].z);

    glUniformMatrix4fv(
        mUniformProjectionLoc, 1, GL_FALSE, glm::value_ptr(projMat)
    );
    glUniformMatrix4fv(
        mUniformViewLoc, 1, GL_FALSE, glm::value_ptr(viewMat)
    );
    glUniformMatrix4fv(
        mUniformModelLoc, 1, GL_FALSE, glm::value_ptr(modelMat)
    );
    // tell OpenGL which vertex array object to use to render the Monkey
    glBindVertexArray(mVao);
    // actually render the Monkey
    glDrawArrays(GL_TRIANGLES, 0, GLint(this->MeshVertices.size()));
    
}

void Mesh::freeGPUData()
{
    // unwind all the allocations made
    glDeleteVertexArrays(1, &mVao);
    glDeleteBuffers(1, &mVbo);
    glDeleteShader(mFragHandle);
    glDeleteShader(mVertHandle);
    glDeleteProgram(mProgramHandle);
}

void Mesh::setupUniformVariables()
{
    mUniformModelLoc = glGetUniformLocation(mProgramHandle, "model");
    // do 2 more times for other matrices
    mUniformViewLoc = glGetUniformLocation(mProgramHandle, "view");
    mUniformProjectionLoc = glGetUniformLocation(mProgramHandle, "proj");
    matDiffuseLoc = glGetUniformLocation(mProgramHandle, "material.diffuse");
    matSpecularLoc = glGetUniformLocation(mProgramHandle, "material.specular");
    matShininessLoc = glGetUniformLocation(mProgramHandle, "material.Shininess");
    dirDiffuseLoc = glGetUniformLocation(mProgramHandle, "dirLight.diffuse");
    dirDirectLoc = glGetUniformLocation(mProgramHandle, "dirLight.direction");
    dirSpecLoc = glGetUniformLocation(mProgramHandle, "dirLight.specular");

    lightColourLoc = glGetUniformLocation(mProgramHandle, "lightColour");
    lightPosLoc = glGetUniformLocation(mProgramHandle, "pointLights[0].position");
    plightDiffuseLoc = glGetUniformLocation(mProgramHandle, "pointLights[0].diffuse");
    plSpecularLoc = glGetUniformLocation(mProgramHandle, "pointLights[0].specular");

    objectColorLoc = glGetUniformLocation(mProgramHandle, "objectColor");
    ambientStrengthLoc = glGetUniformLocation(mProgramHandle, "ambientStrength");
    viewPosLoc = glGetUniformLocation(mProgramHandle, "viewPos");

    ConstantAttenuationLoc = glGetUniformLocation(mProgramHandle, "ConstantAttenuation");
    LinearAttenuationLoc = glGetUniformLocation(mProgramHandle, "LinearAttenuation");
    QuadraticAttenuationLoc = glGetUniformLocation(mProgramHandle, "QuadraticAttenuation");
    ShininessLoc = glGetUniformLocation(mProgramHandle, "Shininess");

}
bool Mesh::loadOBJ(
    const char* path
) {
    
    std::vector< glm::vec3 > temp_vertices;
    std::vector< glm::vec2 > temp_uvs;
    std::vector< glm::vec3 > temp_normals;

    FILE* file = fopen(path, "r");
    if (file == NULL) {
        printf("Cannot open the file !\n");
        return false;
    }
    while (1) {

        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.

        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
            temp_uvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        }
        
        else if (strcmp(lineHeader, "f") == 0) {
            std::string vertex1, vertex2, vertex3;
            GLint vertexIndex[3], normalIndex[3];
            int matches = fscanf_s(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
            if (matches != 6) {
                printf("f: num of parameters doesnt match. 6 values expected in f\n");
                return false;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
            
        }

    }
    //indexing
    // For each vertex of each triangle  
    for (unsigned int i = 0; i < vertexIndices.size(); i++) {
        // Get the indices of its attributes  
        unsigned int vertexIndexn = vertexIndices[i];
        unsigned int normalIndexn = normalIndices[i];

        // Get the attributes thanks to the index  
        glm::vec3 vertex = temp_vertices[vertexIndexn - 1];
        glm::vec3 normal = temp_normals[normalIndexn - 1];

        // Put the attributes in buffers  
        MeshVertices.push_back(vertex);
        normals.push_back(normal);
    }

 
    return true;
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
        if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {		
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
        GLfloat yoffset = (GLfloat)(lastY - ypos);		
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

void Program::run(Cube& cub, Pointlight& point)
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
        point.render( width, height);

        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }
}

void Program::runMesh(Mesh& mesh, Cube& cube, Pointlight& point)
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
        cube.render(paused, width, height);
        mesh.render(paused, width, height);
        point.render(width, height);

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
        std::array<float, 324> vertices
        {
            // Vertices          Colours                    Normal
           -0.3f, -0.3f, -0.3f,  1.0f, 0.5f, 0.31f, 0.0f, 0.0f, -1.0f,//front
            0.3f, -0.3f, -0.3f,  1.0f, 0.5f, 0.31f, 0.0f, 0.0f, -1.0f,
            0.3f,  0.3f, -0.3f,  1.0f, 0.5f, 0.31f, 0.0f, 0.0f, -1.0f,
            0.3f,  0.3f, -0.3f,  1.0f, 0.5f, 0.31f, 0.0f, 0.0f, -1.0f,
           -0.3f,  0.3f, -0.3f,  1.0f, 0.5f, 0.31f, 0.0f, 0.0f, -1.0f,
           -0.3f, -0.3f, -0.3f,   1.0f, 0.5f, 0.31f, 0.0f, 0.0f, -1.0f,

           - 0.3f, -0.3f,  0.3f, 1.0f, 0.5f, 0.31f,  0.0f, 0.0f, 1.0f,//back
           0.3f, -0.3f,  0.3f,   1.0f, 0.5f, 0.31f,  0.0f, 0.0f, 1.0f,
            0.3f,  0.3f,  0.3f,  1.0f, 0.5f, 0.31f, 0.0f, 0.0f, 1.0f,
            0.3f,  0.3f,  0.3f,  1.0f, 0.5f, 0.31f, 0.0f, 0.0f, 1.0f,
           -0.3f,  0.3f,  0.3f,  1.0f, 0.5f, 0.31f, 0.0f, 0.0f, 1.0f,
           -0.3f, -0.3f,  0.3f,   1.0f, 0.5f, 0.31f, 0.0f, 0.0f, 1.0f,

           - 0.3f,  0.3f,  0.3f,  1.0f, 0.5f, 0.31f, -1.0f, 0.0f, 0.0f,//left
           -0.3f,  0.3f, -0.3f,   1.0f, 0.5f, 0.31f, -1.0f, 0.0f, 0.0f,
             -0.3f, -0.3f, -0.3f,  1.0f, 0.5f, 0.31f, -1.0f, 0.0f, 0.0f,
           -0.3f, -0.3f, -0.3f,   1.0f, 0.5f, 0.31f, -1.0f, 0.0f, 0.0f,
           -0.3f, -0.3f,  0.3f,   1.0f, 0.5f, 0.31f, -1.0f, 0.0f, 0.0f,
           -0.3f,  0.3f,  0.3f,   1.0f, 0.5f, 0.31f, -1.0f, 0.0f, 0.0f,

           0.3f,  0.3f,  0.3f,   1.0f, 0.5f, 0.31f, 1.0f, 0.0f, 0.0f,//right
           0.3f,  0.3f, -0.3f,   1.0f, 0.5f, 0.31f, 1.0f, 0.0f, 0.0f,
            0.3f, -0.3f, -0.3f,   1.0f, 0.5f, 0.31f,1.0f, 0.0f, 0.0f,
           0.3f, -0.3f, -0.3f,   1.0f, 0.5f, 0.31f,1.0f, 0.0f, 0.0f,
           0.3f, -0.3f,  0.3f,  1.0f, 0.5f, 0.31f,1.0f, 0.0f, 0.0f,
           0.3f,  0.3f,  0.3f,   1.0f, 0.5f, 0.31f,1.0f, 0.0f, 0.0f,

           -0.3f, -0.3f, -0.3f,   1.0f, 0.5f, 0.31f, 0.0f, -1.0f, 0.0f, //bottom
            0.3f, -0.3f, -0.3f,   1.0f, 0.5f, 0.31f,0.0f, -1.0f, 0.0f,
            0.3f, -0.3f,  0.3f,   1.0f, 0.5f, 0.31f,0.0f, -1.0f, 0.0f,
            0.3f, -0.3f,  0.3f,   1.0f, 0.5f, 0.31f,0.0f, -1.0f, 0.0f,
           -0.3f, -0.3f,  0.3f,   1.0f, 0.5f, 0.31f,0.0f, -1.0f, 0.0f,
           -0.3f, -0.3f, -0.3f,   1.0f, 0.5f, 0.31f,0.0f, -1.0f, 0.0f,

           -0.3f,  0.3f, -0.3f,   1.0f, 0.5f, 0.31f, 0.0f, 1.0f, 0.0f,//top
            0.3f,  0.3f, -0.3f,   1.0f, 0.5f, 0.31f,0.0f, 1.0f, 0.0f,
            0.3f,  0.3f,  0.3f,   1.0f, 0.5f, 0.31f,0.0f, 1.0f, 0.0f,
           0.3f,  0.3f,  0.3f,   1.0f, 0.5f, 0.31f,0.0f, 1.0f, 0.0f,
          -0.3f,  0.3f,  0.3f,   1.0f, 0.5f, 0.31f,0.0f, 1.0f, 0.0f,
          -0.3f,  0.3f, -0.3f,   1.0f, 0.5f, 0.31f,0.0f, 1.0f, 0.0f

        };
        // clang-format on

        Program prog{ WIDTH, HEIGHT, "CSC305_Ass3 Tong_Zhang" };
        Cube cub{};
        
        cub.loadShaders();
        cub.loadDataToGPU(vertices);

        Pointlight point{};
        point.loadShaders();
        point.loadDataToGPU(vertices);

        Mesh mesh1{};

        std::string shaderRoot{ ShaderPath };
        std::string meshpath = shaderRoot + "suzanne.obj";
        char* char_path = new char [meshpath.length()+1];
        strcpy(char_path, meshpath.c_str());
        printf("read from mesh_path: %s\n", char_path);
        bool res = mesh1.loadOBJ(char_path);
        if (res) {
            mesh1.loadShaders();
            mesh1.loadDataToGPU();
            prog.runMesh(mesh1,cub, point);
        }
        else
            prog.run(cub, point);

        prog.freeGPUData();
        cub.freeGPUData();
        point.freeGPUData();
        mesh1.freeGPUData();
    }
    catch (OpenGLError & err)
    {
        fmt::print("OpenGL Error:\n\t{}\n", err.what());
    }

    return 0;
}
