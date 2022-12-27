/**
 * Code outline from https://www.youtube.com/playlist?list=PLvv0ScY6vfd9zlZkIIqGDeG5TUWswkMox 
 */ 


#include <glad/glad.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <string>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include "glm/glm.hpp"
#include <cstdlib>
#include "Camera.hpp"
#include "Transform.hpp"
#if defined(LINUX) || defined(MINGW)
    #include <SDL2/SDL.h>
#else // This works for Mac
    #include <SDL.h>
#endif

/////////////////// GLOBALS /////////////////////////

// Screen Dimensions
int gScreenHeight = 750;
int gScreenWidth = 1024;
SDL_Window* gGraphicsApplicationWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;
Transform gTransform;
std::vector<GLfloat> gOffsets;
int gNumberOfInstances;
int gNumberOfOffsets;
int gPPMWidth;
int gPPMHeight;
std::string gMagicNumber;
unsigned char* gPixelData;
// MainLoop flag
bool gQuit = false;

// VAO, encapsulates all items needed to render object
GLuint gVertexArrayObject = 0;
// VBO, store info relating to vertices (positions, normals, textures)
GLuint gVertexBufferObject = 0;
GLuint gInstanceVBO = 0;
GLuint gColorBuffer = 0;
GLuint gIndexBufferObject = 0;
GLuint gTextureID;
std::vector<GLint> gIndices;

//////////////////// GLOBALS /////////////////////////

// Shader
// stores the unique id for the graphics pipeline
// program object used for OpenGL draw calls
GLuint gGraphicsPipelineShaderProgram = 0;

std::string LoadShader(const std::string& fname) {
    std::string result;
    // 1.) Get every line of data
    std::string line;
    std::ifstream myFile(fname.c_str());

    if(myFile.is_open()){
        while(getline(myFile,line)){
            result += line + '\n';
        }
    } else {
        std::cout << ("LoadShader: file not found. Try an absolute file path to see if the file exists\n");
    }
    // Close file
    myFile.close();
    return result;
}


void createTranslations() {

    const int start = -15;
    const int end = 15;
    gNumberOfOffsets = 0;
    gNumberOfInstances = 0;
    for (int x = start; x <end; x++) {
        for (int y = start; y < end; y++) {
			for (int z = start; z < end; z++) {
				gOffsets.push_back((float)x * 5.0f);
				gOffsets.push_back((float)y * 5.0f);
				gOffsets.push_back((float)z * 5.0f);
				gNumberOfOffsets += 3;
				gNumberOfInstances++;
            }
        }
    }

    std::cout << "Number of instances: " << gNumberOfInstances << std::endl;
}

GLuint CompileShader(GLuint type, const std::string& source) {

    GLuint shaderObject;
    
    if (type == GL_VERTEX_SHADER) {
        shaderObject = glCreateShader(GL_VERTEX_SHADER);
    } else if (type == GL_FRAGMENT_SHADER) {
        shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    }

    const char* src = source.c_str();
    glShaderSource(shaderObject, 1, &src, nullptr);
    glCompileShader(shaderObject);

    int result;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
        char* errorMessages = new char[length];
        glGetShaderInfoLog(shaderObject, length, &length, errorMessages);

        if (type == GL_VERTEX_SHADER) {
            std::cout << "ERROR: GL_VERTEX_SHADER compilation failed." << std::endl << errorMessages << std::endl;

        } else if (type == GL_FRAGMENT_SHADER) {
            
            std::cout << "ERROR: GL_FRAGMENT_SHADER compilation failed." << std::endl << errorMessages << std::endl;

        }

        delete[] errorMessages;

        glDeleteShader(shaderObject);

        return 0;
    }

    return shaderObject;
}

GLuint CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource) {
   
    // Create a new prgram object 
    GLuint programObject = glCreateProgram();

    // Compile Shaders
    GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Link our 2 shaders together
    glAttachShader(programObject, myVertexShader);
    glAttachShader(programObject, myFragmentShader);
    glLinkProgram(programObject);

    // Validate program
    glValidateProgram(programObject);

    // glDetachShader, glDeleteShader
    
    glDetachShader(programObject, myVertexShader);
    glDetachShader(programObject, myFragmentShader);

    glDeleteShader(myVertexShader);
    glDeleteShader(myFragmentShader);
     
    return programObject;
}

void SetUniform2f(std::string name, const glm::vec2 &value) {

    GLint location = glGetUniformLocation(gGraphicsPipelineShaderProgram, name.c_str());
    glUniform2fv(location, 1, &value[0]);

}

void SetUniform3f(std::string name, const glm::vec3 &value) {
//void SetUniform3f(std::string name, float v0, float v1, float v2) {
    GLint location = glGetUniformLocation(gGraphicsPipelineShaderProgram, name.c_str());
    //glUniform3f(location, v0, v1, v2);
    glUniform3fv(location, 1, &value[0]);
}

void CreateGraphicsPipeline() {

    std::string vertexShaderSource = LoadShader("./shaders/vert.glsl");
    std::string fragmentShaderSource = LoadShader("./shaders/frag.glsl");
    gGraphicsPipelineShaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);

}

// Function to get OpenGL Version Information
void GetOpenGLVersionInfo(){
    SDL_Log("(Note: If you have two GPU's, make sure the correct one is selected)");
    SDL_Log("(Note: Let instructor know if you do not have at least OpenGL 4.3)");
    SDL_Log("Vendor: %s",(const char*)glGetString(GL_VENDOR));
    SDL_Log("Renderer: %s",(const char*)glGetString(GL_RENDERER));
    SDL_Log("Version: %s",(const char*)glGetString(GL_VERSION));
    SDL_Log("Shading language: %s",(const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
}


/**
 * Function adapted from class
 */
void LoadPPM(bool flip, const std::string filepath){
    // Open an input file stream for reading a file
    std::ifstream ppmFile(filepath.c_str());
    // If our file successfully opens, begin to process it.
    if (ppmFile.is_open()){
        // line will store one line of input
        std::string line;
        // Our loop invariant is to continue reading input until
        // we reach the end of the file and it reads in a NULL character
        std::cout << "Reading in ppm file: " << filepath << std::endl;
        unsigned int iteration = 0;
        unsigned int pos = 0;
        while ( getline (ppmFile, line) ){
            // Ignore comments in the file
            if (line[0] == '#'){
                continue;
            } else if (line[0] == 'P'){
                gMagicNumber = line;
            } else if (iteration == 1) {
                // Returns first token 
                char *token = strtok((char*)line.c_str(), " "); 
                gPPMWidth = atoi(token);
                token = strtok(NULL, " ");
                gPPMHeight = atoi(token);
                std::cout << "PPM width, height = " << gPPMWidth << ", " << gPPMHeight << "\n";	
                if(gPPMWidth > 0 && gPPMHeight > 0){
                    gPixelData = new unsigned char[gPPMWidth * gPPMHeight * 3];
                    if(gPixelData == NULL) {
                        std::cout << "Unable to allocate memory for ppm" << std::endl;
                        exit(1);
                    }
                } else {
                    std::cout << "PPM not parsed correctly, width and/or height dimensions are 0" << std::endl;
                    exit(1);
                }
            } else if (iteration == 2) {
			// max color range is stored here
            } else {
                gPixelData[pos] = (unsigned char)atoi(line.c_str());
                ++pos;
            }
        iteration++;
        }             
    ppmFile.close();
    } else {
        std::cout << "Unable to open ppm file:" << filepath << std::endl;
    } 

    // Flip all of the pixels
    if (flip) {
        // Copy all of the data to a temporary stack-allocated array
        unsigned char* copyData = new unsigned char[gPPMWidth * gPPMHeight * 3];
        for(int i = 0; i < gPPMWidth * gPPMHeight * 3; ++i){
            copyData[i] = gPixelData[i];
        }
        //memcpy(copyData,m_pixelData,(m_width*m_height*3)*sizeof(unsigned char));
        unsigned int pos = (gPPMWidth * gPPMHeight * 3) - 1;
        for(int i = 0; i < gPPMWidth * gPPMHeight * 3; i += 3){
            gPixelData[pos] = copyData[i + 2];
            gPixelData[pos - 1] = copyData[i + 1];
            gPixelData[pos - 2] = copyData[i];
            pos -= 3;
        }
        delete[] copyData;
    }
}

/**
 * Function adapted from class
 */
void LoadTexture(const std::string filepath){
	// Load our actual image data
	// This method loads .ppm files of pixel data
	LoadPPM(true, filepath);
    glEnable(GL_TEXTURE_2D); 
	// Generate a buffer for our texture
    glGenTextures(1, &gTextureID);
    // Similar to our vertex buffers, we now 'select'
    // a texture we want to bind to.
    // Note the type of data is 'GL_TEXTURE_2D'
    glBindTexture(GL_TEXTURE_2D, gTextureID);
	// Now we are going to setup some information about
	// our textures.
	// There are four parameters that must be set.
	// GL_TEXTURE_MIN_FILTER - How texture filters (linearly, etc.)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
	// Wrap mode describes what to do if we go outside the boundaries of
	// texture.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
	// At this point, we are now ready to load and send some data to OpenGL.
	glTexImage2D(GL_TEXTURE_2D,
							0,
						GL_RGB,
						gPPMWidth,
						gPPMHeight,
							0,
						GL_RGB,
						GL_UNSIGNED_BYTE,
						gPixelData); // Here is the raw pixel data
	// We are done with our texture data so we can unbind.
	glBindTexture(GL_TEXTURE_2D, 0);
}

void VertexSpecification() {

    const std::vector<GLfloat> vertexPosition {
        0.4, 0.4, -0.4, 0.49837, 0.997711, 
        -0.4, 0.4, 0.4, 0.364645, 0.74957, 
        0.4, 0.4, 0.4, 0.499217, 0.749885, 
        -0.4, -0.4, -0.4, 0.228806, 0.50372, 
        -0.4, -0.4, 0.4, 0.364645, 0.500571, 
        -0.4, 0.4, -0.4, 0.770971, 0.749295, 
        0.4, -0.4, -0.4, 0.635676, 0.500925, 
        -0.4, -0.4, -0.4, 0.771314, 0.50124, 
        0.4, -0.4, 0.4, 0.498631, 0.50132, 
        -0.4, -0.4, -0.4, 0.364645, 0.251714, 
        0.4, -0.4, -0.4, 0.499337, 0.250604, 
        0.4, 0.4, -0.4, 0.635676, 0.749925, 
        -0.4, 0.4, -0.4, 0.365873, 0.998711, 
        -0.4, 0.4, -0.4, 0.228977, 0.74831 
    };


    const std::vector<GLuint> indices {
        0, 1, 2, 
        1, 3, 4, 
        5, 6, 7, 
        8, 9, 10, 
        2, 4, 8, 
        11, 8, 6, 
        0, 12, 1, 
        1, 13, 3, 
        5, 11, 6, 
        8, 4, 9, 
        2, 1, 4, 
        11, 2, 8
    };

    for (GLuint i : indices) {
        gIndices.push_back(i);
    }
    
    // Color
    const std::vector<GLfloat> colorData {
        0.583f,  0.771f,  0.014f,
        0.609f,  0.115f,  0.436f,
        0.327f,  0.483f,  0.844f,
        0.822f,  0.569f,  0.201f,
        0.435f,  0.602f,  0.223f,
        0.310f,  0.747f,  0.185f,
        0.597f,  0.770f,  0.761f,
        0.559f,  0.436f,  0.730f,
        0.359f,  0.583f,  0.152f,
        0.483f,  0.596f,  0.789f,
        0.559f,  0.861f,  0.639f,
        0.195f,  0.548f,  0.859f,
        0.014f,  0.184f,  0.576f,
        0.771f,  0.328f,  0.970f,
        0.406f,  0.615f,  0.116f,
        0.676f,  0.977f,  0.133f,
        0.971f,  0.572f,  0.833f,
        0.140f,  0.616f,  0.489f,
        0.997f,  0.513f,  0.064f,
        0.945f,  0.719f,  0.592f,
        0.543f,  0.021f,  0.978f,
        0.279f,  0.317f,  0.505f,
        0.167f,  0.620f,  0.077f,
        0.347f,  0.857f,  0.137f,
        0.055f,  0.953f,  0.042f,
        0.714f,  0.505f,  0.345f,
        0.783f,  0.290f,  0.734f,
        0.722f,  0.645f,  0.174f,
        0.302f,  0.455f,  0.848f,
        0.225f,  0.587f,  0.040f,
        0.517f,  0.713f,  0.338f,
        0.053f,  0.959f,  0.120f,
        0.393f,  0.621f,  0.362f,
        0.673f,  0.211f,  0.457f,
        0.820f,  0.883f,  0.371f,
        0.982f,  0.099f,  0.879f
    };
  
    // Setup on GPU
    // VAO Setup
    // VAO is a wrapper around the VBOs in that it encapsulates all VBO state
    // Bind to select which VAO we want to work within
    glGenVertexArrays(1, &gVertexArrayObject);
    glBindVertexArray(gVertexArrayObject);
    //POSITION 
    // Generate VBO
    glGenBuffers(1, &gVertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, 
                 vertexPosition.size() * sizeof(GL_FLOAT), 
                 vertexPosition.data(),
                 GL_STATIC_DRAW);

    // For given VAO, we need to tell OpenGL how the info in buffer will be used
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, (void*) 0);

    // TEXTURE
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE,sizeof(GLfloat) * 5, (char*)(sizeof(float) * 3));
    LoadTexture("clouds.ppm");

    // INDEX
    glGenBuffers(1, &gIndexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GL_UNSIGNED_INT), indices.data(), GL_STATIC_DRAW); 

    createTranslations(); 
    // Instance VBO
    glGenBuffers(1, &gInstanceVBO);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, gInstanceVBO); // this attribute comes from a different vertex buffer
    glBufferData(GL_ARRAY_BUFFER, gNumberOfOffsets * sizeof(GLfloat), &gOffsets[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.
    // Unbind our currently bound VAP
    glBindVertexArray(0);
    // Disable attributes opened in vertex attribute array
    glDisableVertexAttribArray(0);
}

void InitializeProgram() {
    // Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO)< 0){
		std::cout << "SDL could not initialize!" << std::endl;
		exit(1);
	}
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// We want to request a double buffer for smooth updating.
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    gGraphicsApplicationWindow = SDL_CreateWindow("Final Project",
                                                  SDL_WINDOWPOS_UNDEFINED,
                                                  SDL_WINDOWPOS_UNDEFINED,
                                                  gScreenWidth,
                                                  gScreenHeight,
                                                  SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    // Check if window didn't create
    if (gGraphicsApplicationWindow == nullptr) {
		std::cout << "SDL_Window was not able to be created." << std::endl;
        exit(1);
    }
    // Create OpenGL Graphics Context
    gOpenGLContext = SDL_GL_CreateContext(gGraphicsApplicationWindow);
    if (gOpenGLContext == nullptr) {
		std::cout << "OpenGL context not available." << std::endl;
        exit(1);
    }
    // initialize Glad Library
    if(!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
        std::cout << "glad was not initialized" << std::endl;
        exit(1);
	}
   GetOpenGLVersionInfo();
}

void Input() { 
    SDL_Event e;
    SDL_StartTextInput();
    float cameraSpeed = 0.50f;
    while(SDL_PollEvent(&e) != 0) {
        if(e.type == SDL_QUIT) {
            std::cout << "Goodbye" << std::endl;
            gQuit = true;
        }
        if (e.type==SDL_MOUSEMOTION){
            // Handle mouse movements
            int mouseX = e.motion.x;
            int mouseY = e.motion.y;
            Camera::Instance().MouseLook(mouseX, mouseY);
        }

        switch(e.type) {
                // Handle keyboard presses
            case SDL_KEYDOWN:
                switch(e.key.keysym.sym) {

                    case SDLK_q:
                        gQuit = true;
                        std::cout << "Goodbye" << std::endl;
                        break;
                    case SDLK_LEFT:
                        Camera::Instance().MoveLeft(cameraSpeed);
                        break;
                    case SDLK_RIGHT:
                        Camera::Instance().MoveRight(cameraSpeed);
                        break;
                    case SDLK_UP:
                        Camera::Instance().MoveForward(cameraSpeed);
                        break;
                    case SDLK_DOWN:
                        Camera::Instance().MoveBackward(cameraSpeed);
                        break;
                    case SDLK_a:
                        Camera::Instance().MoveUp(cameraSpeed);
                        break;
                    case SDLK_z:
                        Camera::Instance().MoveDown(cameraSpeed);
                        break;
                }
                break;
        }
    }
}


void PreDraw() {
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    // Initialize clear color
    // This is the background of the screen.
    glViewport(0, 0, gScreenWidth, gScreenHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glUseProgram(gGraphicsPipelineShaderProgram);

    // MVP
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), ((float)gScreenWidth) / ((float) gScreenHeight), 0.1f, 1024.0f);

    GLint modelMatrixUniformLocation =  glGetUniformLocation(gGraphicsPipelineShaderProgram,"model");
    GLint viewMatrixUniformLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram,"view");
    GLint projectionMatrixUniformLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram,"projection");
    glUniformMatrix4fv(modelMatrixUniformLocation, 1, GL_FALSE, &gTransform.GetInternalMatrix()[0][0]);
    glUniformMatrix4fv(viewMatrixUniformLocation, 1, GL_FALSE, &Camera::Instance().GetWorldToViewmatrix()[0][0]);
    glUniformMatrix4fv(projectionMatrixUniformLocation, 1, GL_FALSE, &projection[0][0]);


    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureID);
    GLint textureLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_Texture");
    glUniform1i(textureLocation, 0);    

}

void Draw() {

    // Enable our attributes
    glBindVertexArray(gVertexArrayObject);
    // Select the vertex buffer object we want to enable
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
    // render data
    glDrawElementsInstanced(GL_TRIANGLES, gIndices.size(), GL_UNSIGNED_INT, nullptr, gNumberOfInstances);
    glBindVertexArray(0);
    // Stop using our current graphics pipeline
    glUseProgram(0);

}

void MainLoop() {
    while (!gQuit) {
        Input();
        PreDraw();
        Draw();
        // Update the screen
        SDL_GL_SwapWindow(gGraphicsApplicationWindow);
    }
}

void Cleanup() {
    SDL_DestroyWindow(gGraphicsApplicationWindow);
    // Delete OpenGL objects
    glDeleteBuffers(1, &gVertexBufferObject);
    glDeleteVertexArrays(1, &gVertexArrayObject);
    // Delete Graphics Pipeline
    glDeleteProgram(gGraphicsPipelineShaderProgram);
    // Quit SDL subsystems
    SDL_Quit();
}

int main(int argc, char* args[]) {
    // Set up graphics program
    InitializeProgram();
    // Setup geometry
    VertexSpecification();
    // Create graphics pipeline
    CreateGraphicsPipeline();
    // Main loop
    MainLoop();
    // call the cleanup fnc
    Cleanup();
    return 0;
}
