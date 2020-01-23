#include <vector>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <ctime>
#include "X11/Xlib.h"
#include "GLESUtils.h"

#define DYNAMICGLES_NO_NAMESPACE
#define DYNAMICEGL_NO_NAMESPACE

#include <DynamicGles.h>

// 着色器路径
std::string vsh_path = "../../shader/test/vsh.vert";
std::string fsh_path = "../../shader/test/fsh.frag";

// 纹理路径
const int TEXTURE_SIZE = 3;
std::string image_file = "../../pic/1.jpg";
std::string image_file2 = "../../pic/2.jpg";
std::string image_file3 = "../../pic/3.jpg";
std::vector<std::string> image_files;

// 时间变量
clock_t start, finish;
// 速度控制
const float speed = 1.3;

/**
 * @MethodName: initShaders
 * @Return: 初始化是否成功
 * @Description: 初始化shaders
 */
bool GLESUtils::initShaders() {
    /* 读取shader文件 */
    std::string vshStr = readShader(vsh_path);
    std::string fshStr = readShader(fsh_path);

    bool fragResult = createShader(fshStr, GL_FRAGMENT_SHADER);
    bool vertResult = createShader(vshStr, GL_VERTEX_SHADER);

    if (!fragResult || !vertResult) {
        return false;
    }

    // Create the shader program
    _shaderProgram = glCreateProgram();

    // Attach the fragment and vertex shaders to it
    glAttachShader(_shaderProgram, _fragmentShader);
    glAttachShader(_shaderProgram, _vertexShader);

    // Link the program
    glLinkProgram(_shaderProgram);

    // Check if linking succeeded in the same way we checked for compilation success
    GLint isLinked;
    glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &isLinked);
    if (!isLinked) {
        // If an error happened, first retrieve the length of the log message
        int infoLogLength, charactersWritten;
        glGetProgramiv(_shaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength);

        // Allocate enough space for the message and retrieve it
        std::vector<char> infoLog;
        infoLog.resize(infoLogLength);
        glGetProgramInfoLog(_shaderProgram, infoLogLength, &charactersWritten, infoLog.data());

        // Display the error in a dialog box
        infoLogLength > 1 ? printf("%s", infoLog.data()) : printf("Failed to link shader program.");
        return false;
    }


    /* 加载贴图 */

    // 贴图个数设置
    setTextureSize(TEXTURE_SIZE);
    std::vector<GLuint> vectorTextureID = getVectorTextureID(getTextureSize());

    // vector转数组
    auto *textureIDs = new GLuint[vectorTextureID.size()];
    if (!vectorTextureID.empty()) {
        memcpy(textureIDs, &vectorTextureID[0], vectorTextureID.size() * sizeof(GLuint));
    }

    // 生成贴图
    glGenTextures(vectorTextureID.size(), textureIDs);

    image_files.reserve(vectorTextureID.size());
    image_files.push_back(image_file);
    image_files.push_back(image_file2);
    image_files.push_back(image_file3);

    setVectorTextureID(loadMoreTexture(image_files));

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    return true;
}

/**
 * @MethodName: renderScene
 * @Return: 绘制是否要结束
 * @Description: 绘制
 */
bool GLESUtils::renderScene() {
    GLfloat vVertices[] = {-1.0f, 1.0f, 0.0f,  // Position 0
                           0.0f, 1.0f,        // TexCoord 0
                           -1.0f, -1.0f, 0.0f,  // Position 1
                           0.0f, 0.0f,        // TexCoord 1
                           1.0f, -1.0f, 0.0f,  // Position 2
                           1.0f, 0.0f,        // TexCoord 2
                           1.0f, 1.0f, 0.0f,  // Position 3
                           1.0f, 1.0f         // TexCoord 3
    };
    GLushort indices[] = {0, 1, 2, 0, 2, 3};

    //	Clears the color buffer.
    //	glClear is used here with the Color Buffer to clear the color. It can also be used to clear the depth or stencil buffer using
    //	GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT, respectively.
    glClear(GL_COLOR_BUFFER_BIT);

    //	Use the Program
    //	Calling glUseProgram tells OpenGL ES that the application intends to use this program for rendering. Now that it's installed into
    //	the current state, any further glDraw* calls will use the shaders contained within it to process scene data. Only one program can
    //	be active at once, so in a multi-program application this function would be called in the render loop. Since this application only
    //	uses one program it can be installed in the current state and left there.
    glUseProgram(_shaderProgram);

    if (!testGLError("glUseProgram")) { return false; }

    // Load the vertex position
    glVertexAttribPointer(glGetAttribLocation(_shaderProgram, "a_position"), 3, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), vVertices);

    // Load the texture coordinate
    glVertexAttribPointer(glGetAttribLocation(_shaderProgram, "a_texCoord"), 2, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3]);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Bind the texture
    std::vector<GLuint> vectorTextureID = getVectorTextureID(0);
    for (int i = 0; i < vectorTextureID.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, vectorTextureID[i]);
    }

    // Get the sampler location
    // 设置值
    std::vector<GLint> vValue(vectorTextureID.size());
    for (int i = 0; i < vValue.size(); ++i) {
        vValue[i] = i;
    }

    // vector转换成数组
    auto *values = new GLint[vValue.size()];
    if (!vectorTextureID.empty()) {
        memcpy(values, &vValue[0], vValue.size() * sizeof(GLint));
    }

    /* 传参 */
    // 进度控制
    finish = clock();
    GLint progressLoc = glGetUniformLocation(_shaderProgram, "progress");
    double progress = (double) (finish - start) / CLOCKS_PER_SEC;
    glUniform1f(progressLoc, (GLfloat) progress);

    std::cout << progress * speed << std::endl;
    glUniform1f(glGetUniformLocation(_shaderProgram, "speed"), (GLfloat) speed);

    // 设置采样器变量
    setSamplerLoc(glGetUniformLocation(_shaderProgram, "s_texture"));
    glUniform1iv(getSamplerLoc(), vectorTextureID.size(), values);

    //	Draw the triangle
    //	glDrawArrays is a draw call, and executes the shader program using the vertices and other state set by the user. Draw calls are the
    //	functions which tell OpenGL ES when to actually draw something to the framebuffer given the current state.
    //	glDrawArrays causes the vertices to be submitted sequentially from the position given by the "first" argument until it has processed
    //	"count" vertices. Other draw calls exist, notably glDrawElements which also accepts index data to allow the user to specify that
    //	some vertices are accessed multiple times, without copying the vertex multiple times.
    //	Others include versions of the above that allow the user to draw the same object multiple times with slightly different data, and
    //	a version of glDrawElements which allows a user to restrict the actual indices accessed.

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    if (!testGLError("glDrawElements")) { return false; }

    // Invalidate the contents of the specified buffers for the framebuffer to allow the implementation further optimization opportunities.
    // The following is taken from https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_discard_framebuffer.txt
    // Some OpenGL ES implementations cache framebuffer images in a small pool of fast memory.  Before rendering, these implementations must load the
    // existing contents of one or more of the logical buffers (color, depth, stencil, etc.) into this memory.  After rendering, some or all of these
    // buffers are likewise stored back to external memory so their contents can be used again in the future.  In many applications, some or all of the
    // logical buffers  are cleared at the start of rendering.  If so, the effort to load or store those buffers is wasted.

    // Even without this extension, if a frame of rendering begins with a full-screen Clear, an OpenGL ES implementation may optimize away the loading
    // of framebuffer contents prior to rendering the frame.  With this extension, an application can use DiscardFramebufferEXT to signal that framebuffer
    // contents will no longer be needed.  In this case an OpenGL ES implementation may also optimize away the storing back of framebuffer contents after rendering the frame.
    if (isGlExtensionSupported("GL_EXT_discard_framebuffer")) {
        GLenum invalidateAttachments[2];
        invalidateAttachments[0] = GL_DEPTH_EXT;
        invalidateAttachments[1] = GL_STENCIL_EXT;

        glDiscardFramebufferEXT(GL_FRAMEBUFFER, 2, &invalidateAttachments[0]);
        if (!testGLError("glDiscardFramebufferEXT")) { return false; }
    }

    //	Present the display data to the screen.
    //	When rendering to a Window surface, OpenGL ES is double buffered. This means that OpenGL ES renders directly to one frame buffer,
    //	known as the back buffer, whilst the display reads from another - the front buffer. eglSwapBuffers signals to the windowing system
    //	that OpenGL ES 2.0 has finished rendering a scene, and that the display should now draw to the screen from the new data. At the same
    //	time, the front buffer is made available for OpenGL ES 2.0 to start rendering to. In effect, this call swaps the front and back
    //	buffers.
    if (!eglSwapBuffers(_eglDisplay, _eglSurface)) {
        testEGLError("eglSwapBuffers");
        return false;
    }

    // Check for messages from the windowing system.
    int numberOfMessages = XPending(_nativeDisplay);
    for (int i = 0; i < numberOfMessages; i++) {
        XEvent event;
        XNextEvent(_nativeDisplay, &event);

        switch (event.type) {
            // Exit on window close
            case ClientMessage:
                // Exit on mouse click
            case ButtonPress:
            case DestroyNotify:
                return false;
            default:
                break;
        }
    }
    return true;
}

/**
 * 主函数
 */
int main(int /*argc*/, char ** /*argv*/) {
    start = clock();

    // opengl_es工具类实例
    GLESUtils glesUtils;

    // 设置窗口大小和名称
    glesUtils.setWindowWH(1600, 900);
    std::string appName = std::string("GLES Demo");
    glesUtils.setAppName(appName);

    // 初始化本地和EGL相关
    glesUtils.initNativeAndEGL();

    // 初始化shader
    if (!glesUtils.initShaders()) { glesUtils.cleanProc(); }

    // 绘图, 循环次数为帧数
    for (int i = 0; i < 80000; ++i) {
        if (!glesUtils.renderScene()) {
            break;
        }
    }

    // 释放资源
    glesUtils.deInitGLState();

    return 0;
}
