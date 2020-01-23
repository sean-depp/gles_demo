//
// Created by sean on 2020/1/20.
//

#include "GLESUtils.h"

#define DYNAMICGLES_NO_NAMESPACE
#define DYNAMICEGL_NO_NAMESPACE

#include <DynamicGles.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <memory>
#include <FreeImage.h>
#include <fstream>
#include <iostream>

/*!*********************************************************************************************************************
\param[in]			functionLastCalled          Function which triggered the error
\return		True if no EGL error was detected
\brief	Tests for an EGL error and prints it.
***********************************************************************************************************************/
bool GLESUtils::testEGLError(const char *functionLastCalled) {
    //	eglGetError returns the last error that occurred using EGL, not necessarily the status of the last called function. The user has to
    //	check after every single EGL call or at least once every frame. Usually this would be for debugging only, but for this example
    //	it is enabled always.
    EGLint lastError = eglGetError();
    if (lastError != EGL_SUCCESS) {
        printf("%s failed (%x).\n", functionLastCalled, lastError);
        return false;
    }
    return true;
}

/*!*********************************************************************************************************************
\param[in]			functionLastCalled          Function which triggered the error
\return		True if no GL error was detected
\brief	Tests for an GL error and prints it in a message box.
***********************************************************************************************************************/
bool GLESUtils::testGLError(const char *functionLastCalled) {
    //	glGetError returns the last error that occurred using OpenGL ES, not necessarily the status of the last called function. The user
    //	has to check after every single OpenGL ES call or at least once every frame. Usually this would be for debugging only, but for this
    //	example it is enabled always
    GLenum lastError = glGetError();
    if (lastError != GL_NO_ERROR) {
        printf("%s failed (%x).\n", functionLastCalled, lastError);
        return false;
    }
    return true;
}

/*!*********************************************************************************************************************
\param[out]		nativeDisplay				Native display to create
\return		Whether the function succeeded or not.
\brief	Creates a native isplay for the application to render into.
***********************************************************************************************************************/
bool GLESUtils::createNativeDisplay() {
    Display **nativeDisplay = &_nativeDisplay;
    // Check for a valid display
    if (!nativeDisplay) { return false; }

    // Open the display
    *nativeDisplay = XOpenDisplay(0);
    if (!*nativeDisplay) {
        printf("Error: Unable to open X display\n");
        return false;
    }
    return true;
}

/*!*********************************************************************************************************************
\param[in]			nativeDisplay				Native display used by the application
\param[out]		nativeWindow			    Native window type to create
\return		Whether the function succeeded or not.
\brief	Creates a native window for the application to render into.
***********************************************************************************************************************/
bool GLESUtils::createNativeWindow() {
    Window *nativeWindow = &_nativeWindow;
    // Get the default screen for the display
    int defaultScreen = XDefaultScreen(_nativeDisplay);

    // Get the default depth of the display
    int defaultDepth = DefaultDepth(_nativeDisplay, defaultScreen);

    // Select a visual info
    std::unique_ptr<XVisualInfo> visualInfo(new XVisualInfo);
    XMatchVisualInfo(_nativeDisplay, defaultScreen, defaultDepth, TrueColor, visualInfo.get());
    if (!visualInfo.get()) {
        printf("Error: Unable to acquire visual\n");
        return false;
    }

    // Get the root window for the display and default screen
    Window rootWindow = RootWindow(_nativeDisplay, defaultScreen);

    // Create a color map from the display, root window and visual info
    Colormap colorMap = XCreateColormap(_nativeDisplay, rootWindow, visualInfo->visual, AllocNone);

    // Now setup the final window by specifying some attributes
    XSetWindowAttributes windowAttributes;

    // Set the color map that was just created
    windowAttributes.colormap = colorMap;

    // Set events that will be handled by the app, add to these for other events.
    windowAttributes.event_mask = StructureNotifyMask | ExposureMask | ButtonPressMask;

    // Create the window
    *nativeWindow = XCreateWindow(_nativeDisplay,              // The display used to create the window
                                  rootWindow,                   // The parent (root) window - the desktop
                                  0,                            // The horizontal (x) origin of the window
                                  0,                            // The vertical (y) origin of the window
                                  _winWidth,                 // The width of the window
                                  _winHeight,                // The height of the window
                                  0,                            // Border size - set it to zero
                                  visualInfo->depth,            // Depth from the visual info
                                  InputOutput,                  // Window type - this specifies InputOutput.
                                  visualInfo->visual,           // Visual to use
                                  CWEventMask |
                                  CWColormap,     // Mask specifying these have been defined in the window attributes
                                  &windowAttributes);           // Pointer to the window attribute structure

    // Make the window viewable by mapping it to the display
    XMapWindow(_nativeDisplay, *nativeWindow);

    // Set the window title
    XStoreName(_nativeDisplay, *nativeWindow, _appName);

    // Setup the window manager protocols to handle window deletion events
    Atom windowManagerDelete = XInternAtom(_nativeDisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(_nativeDisplay, *nativeWindow, &windowManagerDelete, 1);

    return true;
}


/*!*********************************************************************************************************************
\param[in]		nativeDisplay               The native display used by the application
\param[out]		eglDisplay				    EGLDisplay created from _nativeDisplay
\return		Whether the function succeeded or not.
\brief	Creates an EGLDisplay from a native native display, and initializes it.
***********************************************************************************************************************/
bool GLESUtils::createEGLDisplay() {
    //	Get an EGL display.
    //	EGL uses the concept of a "display" which in most environments corresponds to a single physical screen. After creating a native
    //	display for a given windowing system, EGL can use this handle to get a corresponding EGLDisplay handle to it for use in rendering.
    //	Should this fail, EGL is usually able to provide access to a default display.
    _eglDisplay = eglGetDisplay((EGLNativeDisplayType) _nativeDisplay);
    // If a display couldn't be obtained, return an error.
    if (_eglDisplay == EGL_NO_DISPLAY) {
        printf("Failed to get an EGLDisplay");
        return false;
    }

    //	Initialize EGL.
    //	EGL has to be initialized with the display obtained in the previous step. All EGL functions other than eglGetDisplay
    //	and eglGetError need an initialized EGLDisplay.
    //	If an application is not interested in the EGL version number it can just pass NULL for the second and third parameters, but they
    //	are queried here for illustration purposes.
    EGLint eglMajorVersion = 0;
    EGLint eglMinorVersion = 0;
    if (!eglInitialize(_eglDisplay, &eglMajorVersion, &eglMinorVersion)) {
        printf("Failed to initialize the EGLDisplay");
        return false;
    }

    // Bind the correct API
    int result = EGL_FALSE;

    result = eglBindAPI(EGL_OPENGL_ES_API);

    if (result != EGL_TRUE) {
        return false;
    }

    return true;
}

/*!*********************************************************************************************************************
\param[in]			eglDisplay                  The EGLDisplay used by the application
\param[out]		eglConfig                   The EGLConfig chosen by the function
\return		Whether the function succeeded or not.
\brief	Chooses an appropriate EGLConfig and return it.
***********************************************************************************************************************/
bool GLESUtils::chooseEGLConfig() {
    //	Specify the required configuration attributes.
    //	An EGL "configuration" describes the capabilities an application requires and the type of surfaces that can be used for drawing.
    //	Each implementation exposes a number of different configurations, and an application needs to describe to EGL what capabilities it
    //	requires so that an appropriate one can be chosen. The first step in doing this is to create an attribute list, which is an array
    //	of key/value pairs which describe particular capabilities requested. In this application nothing special is required so we can query
    //	the minimum of needing it to render to a window, and being OpenGL ES 2.0 capable.
    const EGLint configurationAttributes[] =
            {
                    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                    EGL_NONE
            };

    //	Find a suitable EGLConfig
    //	eglChooseConfig is provided by EGL to provide an easy way to select an appropriate configuration. It takes in the capabilities
    //	specified in the attribute list, and returns a list of available configurations that match or exceed the capabilities requested.
    //	Details of all the possible attributes and how they are selected for by this function are available in the EGL reference pages here:
    //	http://www.khronos.org/registry/egl/sdk/docs/man/xhtml/eglChooseConfig.html
    //	It is also possible to simply get the entire list of configurations and use a custom algorithm to choose a suitable one, as many
    //	advanced applications choose to do. For this application however, taking the first EGLConfig that the function returns suits
    //	its needs perfectly, so we limit it to returning a single EGLConfig.
    EGLint configsReturned;
    if (!eglChooseConfig(_eglDisplay, configurationAttributes, &_eglConfig, 1, &configsReturned) ||
        (configsReturned != 1)) {
        printf("Failed to choose a suitable config.");
        return false;
    }
    return true;
}

/*!*********************************************************************************************************************
\param[in]			nativeWindow                A native window that's been created
\param[in]			eglDisplay                  The EGLDisplay used by the application
\param[in]			eglConfig                   An EGLConfig chosen by the application
\param[out]		eglSurface					The EGLSurface created from the native window.
\return		Whether the function succeeds or not.
\brief	Creates an EGLSurface from a native window
***********************************************************************************************************************/
bool
GLESUtils::createEGLSurface() {
    //	Create an EGLSurface for rendering.
    //	Using a native window created earlier and a suitable _eglConfig, a surface is created that can be used to render OpenGL ES calls to.
    //	There are three main surface types in EGL, which can all be used in the same way once created but work slightly differently:
    //	 - Window Surfaces  - These are created from a native window and are drawn to the screen.
    //	 - Pixmap Surfaces  - These are created from a native windowing system as well, but are offscreen and are not displayed to the user.
    //	 - PBuffer Surfaces - These are created directly within EGL, and like Pixmap Surfaces are offscreen and thus not displayed.
    //	The offscreen surfaces are useful for non-rendering contexts and in certain other scenarios, but for most applications the main
    //	surface used will be a window surface as performed below.
    _eglSurface = eglCreateWindowSurface(_eglDisplay, _eglConfig, (EGLNativeWindowType) _nativeWindow, NULL);
    if (!testEGLError("eglCreateWindowSurface")) { return false; }
    return true;
}

/*!*********************************************************************************************************************
\param[in]			eglDisplay                  The EGLDisplay used by the application
\param[in]			eglConfig                   An EGLConfig chosen by the application
\param[in]			eglSurface					The EGLSurface created from the native window.
\param[out]		context                  The EGLContext created by this function
\param[in]			nativeWindow                A native window, used to display error messages
\return		Whether the function succeeds or not.
\brief	Sets up the EGLContext, creating it and then installing it to the current thread.
***********************************************************************************************************************/
bool
GLESUtils::setupEGLContext() {
    //	Make OpenGL ES the current API.
    // EGL needs a way to know that any subsequent EGL calls are going to be affecting OpenGL ES,
    // rather than any other API (such as OpenVG).
    eglBindAPI(EGL_OPENGL_ES_API);
    if (!testEGLError("eglBindAPI")) { return false; }

    //	Create a _context.
    //	EGL has to create what is known as a _context for OpenGL ES. The concept of a _context is OpenGL ES's way of encapsulating any
    //	resources and state. What appear to be "global" functions in OpenGL actually only operate on the current _context. A _context
    //	is required for any operations in OpenGL ES.
    //	Similar to an EGLConfig, a _context takes in a list of attributes specifying some of its capabilities. However in most cases this
    //	is limited to just requiring the version of the OpenGL ES _context required - In this case, OpenGL ES 2.0.
    EGLint contextAttributes[] =
            {
                    EGL_CONTEXT_CLIENT_VERSION, 2,
                    EGL_NONE
            };

    // Create the _context with the _context attributes supplied
    _context = eglCreateContext(_eglDisplay, _eglConfig, NULL, contextAttributes);
    if (!testEGLError("eglCreateContext")) { return false; }

    //	Bind the _context to the current thread.
    //	Due to the way OpenGL uses global functions, contexts need to be made current so that any function call can operate on the correct
    //	_context. Specifically, make current will bind the _context to the current rendering thread it's called from. If the calling thread already
    //  has a current rendering _context then that _context is flushed and marked as no longer current. It is not valid to call eglMakeCurrent with a _context
    //  which is current on another thread.
    //  To use multiple contexts at the same time, users should use multiple threads and synchronise between them.eglMakeCurrent(_eglDisplay, _eglSurface, _eglSurface, _context);
    eglMakeCurrent(_eglDisplay, _eglSurface, _eglSurface, _context);

    if (!testEGLError("eglMakeCurrent")) { return false; }
    return true;
}

/*!*********************************************************************************************************************
\param[in]			eglDisplay                   The EGLDisplay used by the application
\brief	Releases all resources allocated by EGL
***********************************************************************************************************************/
void GLESUtils::releaseEGLState(EGLDisplay eglDisplay) {
    if (eglDisplay != NULL) {
        // To release the resources in the _context, first the _context has to be released from its binding with the current thread.
        eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        // Terminate the display, and any resources associated with it (including the EGLContext)
        eglTerminate(eglDisplay);
    }
}

/*!*********************************************************************************************************************
\param[in]			nativeDisplay               The native display to release
\param[in]			nativeWindow                The native window to destroy
\brief	Releases all resources allocated by the windowing system
***********************************************************************************************************************/
void GLESUtils::releaseNativeResources(Display *nativeDisplay, Window nativeWindow) {
    // Destroy the window
    if (nativeWindow) { XDestroyWindow(nativeDisplay, nativeWindow); }

    // Release the display.
    if (nativeDisplay) { XCloseDisplay(nativeDisplay); }
}

/*!*********************************************************************************************************************
\param[in]			fragmentShader              Handle to a fragment shader
\param[in]			vertexShader                Handle to a vertex shader
\param[in]			shaderProgram               Handle to a shader program containing the fragment and vertex shader
\param[in]			vertexBuffer                Handle to a vertex buffer object
\brief	Releases the resources created by "InitializeGLState"
***********************************************************************************************************************/
void GLESUtils::deInitGLState() {
    // Frees the OpenGL handles for the program and the 2 shaders
    glDeleteShader(_fragmentShader);
    glDeleteShader(_vertexShader);

    // Delete texture object
    glDeleteTextures(1, &_textureID);

    glDeleteProgram(_shaderProgram);
}

Display *GLESUtils::getNativeDisplay() {
    return _nativeDisplay;
}

EGLDisplay &GLESUtils::getEglDisplay() {
    return _eglDisplay;
}

EGLConfig &GLESUtils::getEglConfig() {
    return _eglConfig;
}

Window GLESUtils::getNativeWindow() {
    return _nativeWindow;
}

GLuint GLESUtils::loadTexture(std::string fileName) {
    //unsigned textureId = 0;
    GLuint textureId = 0;
    //1 获取图片格式
    FREE_IMAGE_FORMAT fifmt = FreeImage_GetFileType(fileName.c_str(), 0);
    //2 加载图片
    FIBITMAP *dib = FreeImage_Load(fifmt, fileName.c_str(), 0);
    //3 转换为rgb24色
    dib = FreeImage_ConvertTo24Bits(dib);

    //4 获取数据指针
    BYTE *pixels = (BYTE *) FreeImage_GetBits(dib);

    int width = FreeImage_GetWidth(dib);
    int height = FreeImage_GetHeight(dib);

    //window存储颜色的格式是BGR,opengl的格式是RGB  所以需要翻转
    int i = 0;
    for (i = 0; i < width * height * 3; i += 3) {
        BYTE temp = pixels[i];
        pixels[i] = pixels[i + 2];
        pixels[i + 2] = temp;
    }

    // Use tightly packed data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Generate a texture object
    glGenTextures(1, &textureId);

    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, textureId);

    // Load the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // Set the filtering mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //释放内存
    FreeImage_Unload(dib);
    return textureId;
}

std::vector<GLuint> GLESUtils::loadMoreTexture(std::vector<std::string> fileNames) {
    std::vector<GLuint> vectorTextureId(fileNames.size());
    for (int i = 0; i < fileNames.size(); ++i) {
//        std::cout << fileNames[i] << std::endl;
        vectorTextureId[i] = loadTexture(fileNames[i]);
    }

    return vectorTextureId;
}

std::string GLESUtils::readShader(std::string path) {
    std::ifstream in(path);
    std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return contents;
}

void GLESUtils::cleanProc() {
    // Release the EGL State
    releaseEGLState(_eglDisplay);

    // Release the windowing system resources
    releaseNativeResources(_nativeDisplay, _nativeWindow);
}


EGLSurface GLESUtils::getEglSurface() {
    return _eglSurface;
}

EGLContext GLESUtils::getContext() {
    return _context;
}

GLint GLESUtils::getSamplerLoc() {
    return _samplerLoc;
}

void GLESUtils::setSamplerLoc(GLint sl) {
    _samplerLoc = sl;
}

GLuint GLESUtils::getTextureID() {
    return _textureID;
}

void GLESUtils::setTextureID(GLuint tid) {
    _textureID = tid;
}

std::vector<GLuint> GLESUtils::getVectorTextureID(const int size) {
    if (size > 0)
        _vectorTextureID.resize(size);
    return _vectorTextureID;
}

void GLESUtils::setVectorTextureID(std::vector<GLuint> vectorTid) {
    _vectorTextureID = vectorTid;
}

void GLESUtils::setWindowWH(unsigned int width, unsigned int height) {
    _winWidth = width;
    _winHeight = height;
}

unsigned int GLESUtils::getWindowWidth() {
    return _winWidth;
}

unsigned int GLESUtils::getWindowHeight() {
    return _winHeight;
}

void GLESUtils::setAppName(std::string appName) {
    _appName = const_cast<char *>(appName.c_str());
}

void GLESUtils::setTextureSize(int size) {
    _textureSize = size;
}

int GLESUtils::getTextureSize() {
    return _textureSize;
}

bool GLESUtils::createShader(std::string shaderPath, int type) {
    // Create a shader object
    GLuint shader = 0;
    shader = glCreateShader(type);

    // Load the source code into it
    glShaderSource(shader, 1, (const char **) &shaderPath, NULL);

    // Compile the source code
    glCompileShader(shader);

    // Check that the shader compiled
    GLint isShaderCompiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isShaderCompiled);
    if (!isShaderCompiled) {
        // If an error happened, first retrieve the length of the log message
        int infoLogLength, charactersWritten;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        // Allocate enough space for the message and retrieve it
        std::vector<char> infoLog;
        infoLog.resize(infoLogLength);
        glGetShaderInfoLog(shader, infoLogLength, &charactersWritten, infoLog.data());

        // Display the error in a dialog box
        if (type == GL_FRAGMENT_SHADER) {
            infoLogLength > 1 ? printf("%s", infoLog.data()) : printf("Failed to compile fragment shader.");
        } else {
            infoLogLength > 1 ? printf("%s", infoLog.data()) : printf("Failed to compile vertex shader.");
        }

        return false;
    }
    if (type == GL_FRAGMENT_SHADER) {
        setFragmentShader(shader);
    } else {
        setVectorShader(shader);
    }
    return true;
}

GLuint &GLESUtils::getFragmentShader() {
    return _fragmentShader;
}

GLuint &GLESUtils::getVertexShader() {
    return _vertexShader;
}

GLuint GLESUtils::getShaderProgram() {
    return _shaderProgram;
}

void GLESUtils::setFragmentShader(GLuint shader) {
    _fragmentShader = shader;
}

void GLESUtils::setVectorShader(GLuint shader) {
    _vertexShader = shader;
}

void GLESUtils::initNativeAndEGL() {
    // Get access to a native display
    if (!createNativeDisplay()) { cleanProc(); }

    // Setup the windowing system, create a window
    if (!createNativeWindow()) { cleanProc(); }

    // Create and Initialize an EGLDisplay from the native display
    if (!createEGLDisplay()) { cleanProc(); }

    // Choose an EGLConfig for the application, used when setting up the rendering surface and EGLContext
    if (!chooseEGLConfig()) { cleanProc(); }

    // Create an EGLSurface for rendering from the native window
    if (!createEGLSurface()) { cleanProc(); }

    // Setup the EGL Context from the other EGL constructs created so far, so that the application is ready to submit OpenGL ES commands
    if (!setupEGLContext()) { cleanProc(); }

}


