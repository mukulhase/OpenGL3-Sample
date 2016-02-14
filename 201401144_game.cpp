#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include "string"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
GLFWwindow* window;
bool clicked;
struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;
    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;
VAO *triangle, *square, *cube;
VAO* createCube(float x, float y, float z, float[2][3]);

class GameObject
{
public:
    float posx;
    float posy;
    float posz;
    float velx;
    float vely;
    float velz;
    float l;
    float b;
    float h;
    float anglex;
    float angley;
    float anglez;
    float velax;
    float velay;
    float velaz;
    float accy;
    float radius;
    VAO* vao;
};
class Player {
public:
    int state[6];
    float speed;
    float maxspeed;
    float minspeed;
    float jumpspeed;
    float lookangle;
    GameObject* object;
    int id;
    Player(){
        for(int i=0; i<4;i++){
            state[i]=0;
        }
        maxspeed = 1;
        minspeed = 0;
        jumpspeed = 0.25;
        lookangle = 0;
    }
    void jump(){
        if(abs(object->vely)<0.001)
            object->vely+=jumpspeed;
    }
    void updateSpeed(int s){
        if(s==1){
            speed+=0.005;
        }else if(s==0){
            speed-=0.005;
        }
        if(speed>maxspeed){
            speed=maxspeed;
        }
        if(speed<minspeed){
            speed=minspeed;
        }
    }
    void update(){
        GameObject* temp = object;
        
        float angle = temp -> angley;
        
        if (state[0]) {
            temp -> angley += 5.0;
        }
        if (state[1]) {
            temp -> angley -= 5.0;
        }
        if(state[2]){
            updateSpeed(1);
        }
        if(state[3]){
            updateSpeed(0);
        }
        if (state[4]) {
            if (lookangle<90) {
                lookangle++;
            }
        }
        if (state[5]) {
            if (lookangle>-90) {
                lookangle--;
            }
        }
        temp->velz=speed*cos(angle*M_PI/180.0f);
        temp->velx=speed*sin(angle*M_PI/180.0f);
    }
    void reset(){
        speed=0;
        object->velx=0;
        object->vely=0;
        object->velz=0;
        object->posx=-10;
        object->posy=2;
        object->posz=-10;
    }
};
class GameState
{
public:
    string message;
    int cameramode; //1=keyboard 0=mouse
    bool paused;
    int width;
    int height;
    bool ended;
    int lives;
    float gravity;
    float helicopterx;
    float helicoptery;
    float helicopterheight;
    VAO* skybox;
    int map[10][10];
    GameObject objects[100000];
    vector <int> oscillatables;
    Player player;
    int objectindex;
    int spawnObject(float posx,float posy, float posz, float velx, float vely, float velz, float accy, VAO* vao){
        objects[objectindex].vao = vao;
        objects[objectindex].posx = posx;
        objects[objectindex].posy = posy;
        objects[objectindex].posz = posz;
        objects[objectindex].velx = velx;
        objects[objectindex].vely = vely;
        objects[objectindex].velz = velz;
        objects[objectindex].anglex = 0;
        objects[objectindex].angley = 0;
        objects[objectindex].anglez = 0;
        objects[objectindex].accy = accy;
        objectindex++;
        return objectindex-1;
    }
    int spawnCube(float posx,float posy, float posz, VAO* vao, float l, float h, float b){
        int index = spawnObject(posx,posy,posz,0,0,0,0,vao);
        objects[index].radius=pow(pow(l/2,2.0)+pow(b/2,2.0)+pow(h/2,2.0),0.5);
        objects[index].l=l;
        objects[index].b=b;
        objects[index].h=h;
        return index;
    }
    void oscillate(){
        for ( auto i = oscillatables.begin(); i != oscillatables.end(); i++ ) {
            objects[*i].accy = -0.05*objects[*i].posy;
        }
    }
    void update(){
        
        player.update();
        if(player.object->posy < -10){
            player.reset();
        }
        /*if(detectfall()){
            //endgame();
        };*/
        oscillate();
        updatePositions();
        checkCollision();
        detectWin();
        
    }
    void updatePositions(){
        int i;
        for (i=0; i<objectindex; i++) {
            objects[i].vely+=objects[i].accy;
            objects[i].posx+=objects[i].velx;
            objects[i].posy+=objects[i].vely;
            objects[i].posz+=objects[i].velz;
            objects[i].anglex+=objects[i].velax;
            objects[i].angley+=objects[i].velay;
            objects[i].anglez+=objects[i].velaz;
        }
    }
    void preset(){
        helicopterheight=30;
        paused=false;
        width=1280;
        height=720;
        gravity=-0.0125;
    }
    void setup(){
        paused=true;
        ended = false;
        objectindex=0;
        lives = 10;
    }
    int detectWin(){
        float x = player.object->posx;
        float y = player.object->posy;
        float z = player.object->posz;
        int cuberow = floor((10 + x +1)/2);
        int cubecol = floor((10 + z +1)/2);
        if (map[cuberow][cubecol] == 'w') {
            endgame();
            return 1;
        }else if(map[cuberow][cubecol] != '1'){
            
        }
        return 0;
    }
    void checkCollision(){
        int i;
        GameObject *temp = player.object;
        GameObject temp2;
        //oscillator collisions
        
        for ( auto i = oscillatables.begin(); i != oscillatables.end(); i++ ) {
            temp2 = objects[*i];
            float xdiff = temp->posx-temp2.posx;
            float ydiff = temp->posy-temp2.posy;
            float zdiff = temp->posz-temp2.posz;
            bool hint = abs(ydiff) < abs(temp2.h + temp->h)/2;
            bool lint = abs(xdiff) < abs(temp2.l + temp->l)/2;
            bool bint = abs(zdiff) < abs(temp2.b + temp->b)/2;
            if (hint && lint && bint) {
                bool hintp = abs(ydiff-temp->vely) < abs(temp2.h + temp->h)/2;
                bool lintp = abs(xdiff-temp->velx) < abs(temp2.l + temp->l)/2;
                bool bintp = abs(zdiff-temp->velz) < abs(temp2.b + temp->b)/2;
                if (hint && lintp && bintp) {
                    player.object->posy += temp2.vely;
                }
            }
        }
        //player collisions
        for(i=0; i<objectindex; i++){
            if (i==player.id) {
                continue;
            }
            temp2 = objects[i];
            float xdiff = temp->posx-temp2.posx;
            float ydiff = temp->posy-temp2.posy;
            float zdiff = temp->posz-temp2.posz;
            bool hint = abs(ydiff) < abs(temp2.h + temp->h)/2;
            bool lint = abs(xdiff) < abs(temp2.l + temp->l)/2;
            bool bint = abs(zdiff) < abs(temp2.b + temp->b)/2;
            if (hint && lint && bint) {
                bool hintp = abs(ydiff-temp->vely) < abs(temp2.h + temp->h)/2;
                bool lintp = abs(xdiff-temp->velx) < abs(temp2.l + temp->l)/2;
                bool bintp = abs(zdiff-temp->velz) < abs(temp2.b + temp->b)/2;
                if (hint && lintp && bintp) {
                    temp->posy -= temp->vely;
                    temp->vely = 0;
                }
                if (lint && hintp && bintp) {
                    temp->posx -= temp->velx;
                    temp->velx = 0;
                }
                if (bint && lintp && hintp) {
                    temp->posz -= temp->velz;
                    temp->velz = 0;
                }
                if (lintp && hintp && bintp) {
                    player.reset();
                }
            }
        }
        
    }
    void init(){
        float skycolor[2][3]= {
            {
                0,255,255
            },
            {
                0,255,255
            }
        };
        skybox = createCube(100,100,100,skycolor);
        float color[2][3]= {
            {
                255,255,125
            },
            {
                255,0,0
            }
        };
        float wincolor[2][3]= {
            {
                125,255,255
            },
            {
                125,125,255
            }
        };
        float nojumpcolor[2][3]= {
            {
                125,255,125
            },
            {
                125,255,125
            }
        };
        player.object = &objects[player.id=spawnCube(-10.0, 2.0, -10.0, createCube(0.01,0.01,0.01,color), 1, 2,1)];
        player.object->accy = gravity;

        int i,j;
        
        for (i=0; i<10; i++) {
            for (j=0; j<10; j++) {
                int k;
                switch (map[i][j]) {
                        /*
                         0 -nothing
                         1 normal
                         2 win
                         3 -1height
                         4 -2height
                         */
                    case '0':
                        break;
                    case '2':
                        for (k=0; k<5; k++) {
                            spawnCube(-10.0 + 2.0*i, -2.0 -2.0*k, -10.0 + 2.0*j, cube, 2,2,2);
                        }
                        break;
                    case '3':
                        for (k=0; k<5; k++) {
                            spawnCube(-10.0 + 2.0*i, -4.0 -2.0*k, -10.0 + 2.0*j, cube, 2,2,2);
                        }
                        break;
                    case '1':
                        for (k=0; k<5; k++) {
                            spawnCube(-10.0 + 2.0*i, -2.0*k, -10.0 + 2.0*j, cube,2,2,2);
                        }
                        break;
                    case 'w':
                        for (k=0; k<5; k++) {
                            spawnCube(-10.0 + 2.0*i, -2.0*k, -10.0 + 2.0*j, createCube(2,2,2,wincolor),2,2,2);
                        }
                        break;
                    case '4':
                        for (k=0; k<5; k++) {
                            spawnCube(-10.0 + 2.0*i, 2.0 -2.0*k, -10.0 + 2.0*j, cube, 2,2,2);
                        }
                        break;
                    case '5':
                        for (k=0; k<5; k++) {
                            spawnCube(-10.0 + 2.0*i, 4.0 -2.0*k, -10.0 + 2.0*j, cube, 2,2,2);
                        }
                        break;
                    case '6':
                        for (k=0; k<5; k++) {
                            spawnCube(-10.0 + 2.0*i, 6.0 -2.0*k, -10.0 + 2.0*j, cube, 2,2,2);
                        }
                        break;
                    case '7':
                        for (k=0; k<5; k++) {
                            spawnCube(-10.0 + 2.0*i, 8.0 -2.0*k, -10.0 + 2.0*j, cube, 2,2,2);
                        }
                        break;
                    case 'o':
                        oscillatables.push_back(spawnCube(-10.0 + 2.0*i, -2.0, -10.0 + 2.0*j, cube, 2,2,2));
                        break;
                    case 'n':
                        spawnCube(-10.0 + 2.0*i, 6.0 -2.0*k, -10.0 + 2.0*j, , createCube(2,2,2,nojumpcolor), 2,2,2);
                    default:
                        break;
                }
            }
        }
    }
    void endgame(){
        ended = true;
    }
};
GameState game;

struct GLMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {
    
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
    
    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }
    
    GLint Result = GL_FALSE;
    int InfoLogLength;
    
    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);
    
    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);
    
    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);
    
    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);
    
    // Link the program
    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);
    
    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
    
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
    
    return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;
    
    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors
    
    glBindVertexArray (vao->VertexArrayID); // Bind the VAO
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }
    
    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);
    
    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);
    
    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);
    
    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);
    
    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Function is called first on GLFW_PRESS.
    
    if (action == GLFW_RELEASE) {
        
        switch (key) {
            case GLFW_KEY_1:
                game.cameramode=0;
                break;
            case GLFW_KEY_2:
                game.cameramode=1;
                break;
            case GLFW_KEY_3:
                game.cameramode=2;
                break;
            case GLFW_KEY_4:
                game.cameramode=3;
                break;
            case GLFW_KEY_5:
                game.cameramode=4;
                break;
            case GLFW_KEY_SPACE:
                game.player.jump();
                break;
            case GLFW_KEY_A:
                break;
            case GLFW_KEY_R:
                game.player.reset();
                break;
            case GLFW_KEY_LEFT:
                game.player.state[0] = 0;
                break;
            case GLFW_KEY_RIGHT:
                game.player.state[1] = 0;
                break;
            case GLFW_KEY_UP:
                game.player.state[4] = 0;
                break;
            case GLFW_KEY_DOWN:
                game.player.state[5] = 0;
                break;
            case GLFW_KEY_F:
                game.player.state[2] = 0;
                break;
            case GLFW_KEY_S:
                game.player.state[3] = 0;
                break;
            case GLFW_KEY_K:
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        if (game.paused) {
            game.paused=false;
        }
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_LEFT:
                game.player.state[0] = 1;
                break;
            case GLFW_KEY_RIGHT:
                game.player.state[1] = 1;
                break;
            case GLFW_KEY_UP:
                game.player.state[4] = 1;
                break;
            case GLFW_KEY_DOWN:
                game.player.state[5] = 1;
                break;
            case GLFW_KEY_F:
                game.player.state[2] = 1;
                break;
            case GLFW_KEY_S:
                game.player.state[3] = 1;
                break;
            case GLFW_KEY_A:
                break;
            case GLFW_KEY_B:
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key) {
        case 'Q':
        case 'q':
            quit(window);
            break;
        default:
            break;
    }
}
float startx;
float starty;
/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE){
                if (game.paused) {
                    game.paused=false;
                }else{
                }
                clicked=false;
            }
            if (action == GLFW_PRESS){
                double mos_x,mos_y;
                glfwGetCursorPos(window, &mos_x, &mos_y);
                startx = mos_x;
                starty = mos_y;
                clicked=true;
            }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_PRESS){
            }
            if (action == GLFW_RELEASE) {
            }
            break;
        default:
            break;
    }
}

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);
    
    GLfloat fov = 1.0f;
    
    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);
    
    // set the projection matrix as perspective
    /* glMatrixMode (GL_PROJECTION);
     glLoadIdentity ();
     gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 1.0f, 500.0f);
    
    // Ortho projection for 2D views
    // Matrices.projection = glm::ortho(-game.leftmax, game.rightmax, -game.downmax, game.upmax, 0.1f, 500.0f);
}



float camera_rotation_angle = 80;
float rectangle_rotation = 0;
float triangle_rotation = 0;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    game.helicopterheight+=yoffset;
}

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void processobjects(){}
glm::mat4 VP;
glm::mat4 MVP;
VAO* createBall (float scale)
{
    int i;
    GLfloat vertex_buffer_data[540], color_buffer_data[540];
    for (i=0; i+2<540; i+=3) {
        vertex_buffer_data[i] = scale*cos(i*M_PI/60);
        vertex_buffer_data[i+1] = scale*sin(i*M_PI/60);
        vertex_buffer_data[i+2] = 1;
        color_buffer_data[i] = 1;
        color_buffer_data[i+1] = 1;
        color_buffer_data[i+2] = 0;
    }
    color_buffer_data[0]=0;
    return create3DObject(GL_TRIANGLE_FAN, 180, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void draw ()
{
    
    float eyex;
    float eyey;
    float eyez;
    float tarx;
    float tary;
    float tarz;
    float firstpersonheight = tan(game.player.lookangle*M_PI/180);
    static float xdiff = 0;
    static float ydiff = M_PI/4;
    // clear the color and depth in the frame buffer
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // use the loaded shader program
    // Don't change unless you know what you are doing
    glUseProgram (programID);

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // For each model you render, since the MVP will be different (at least the M part)
    //  Don't change unless you are sure!!
    
    if(game.paused){
        eyex=0;
        eyey=0;
        eyez=5;
        // Target - Where is the camera looking at.  Don't change unless you are sure!!
        tarx=0;
        tary=0;
        tarz=0;

        glm::vec3 eye (eyex, eyey, eyez);
        // Target - Where is the camera looking at.  Don't change unless you are sure!!
        glm::vec3 target (tarx,tary,tarz);
        // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
        glm::vec3 up (0, 1, 0);
        // Compute Camera matrix (view)
        Matrices.view = glm::lookAt( eye, target, up );
        VP = Matrices.projection * Matrices.view;

        Matrices.model = glm::mat4(1.0f);

        glm::mat4 translateTriangle = glm::translate (glm::vec3(0, 4.0 * (0.5 + 0.25*sin(glfwGetTime()*2)), 0.0f)); // glTranslatef
        // rotate about vector (1,0,0)
        Matrices.model *= translateTriangle;
        MVP = VP * Matrices.model; // MVP = p * V * M
        
        //  Don't change unless you are sure!!
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        
        // draw3DObject draws the VAO given to it using current MVP matrix
        draw3DObject(triangle);

    }else{
        
        switch (game.cameramode) {
            case 0:
                // cout << "TOWER VIEW" << endl;
                // Eye - Location of camera. Don't change unless you are sure!!
                eyex=15*cos(camera_rotation_angle*M_PI/180.0f);
                eyey=12;
                eyez=15*sin(camera_rotation_angle*M_PI/180.0f);
                // Target - Where is the camera looking at.  Don't change unless you are sure!!
                tarx=0;
                tary=0;
                tarz=0;
                // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
                break;
            case 1:
                // cout << "TOP VIEW" << endl;
                // Eye - Location of camera. Don't change unless you are sure!!
                eyex=0.01;
                eyey=30;
                eyez=0.01;            // Target - Where is the camera looking at.  Don't change unless you are sure!!
                tarx=0;
                tary=0;
                tarz=0;
                // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
                break;
            case 2:
                eyex=game.player.object->posx;
                eyey=game.player.object->posy;
                eyez=game.player.object->posz;
                tarx=eyex + 1*sin(game.player.object->angley*M_PI/180.0f);
                tarz=eyez + 1*cos(game.player.object->angley*M_PI/180.0f);
                tary=eyey + firstpersonheight;
                break;
            case 3:
                tarx=game.player.object->posx;
                tary=game.player.object->posy;
                tarz=game.player.object->posz;
                eyex=tarx - 3*sin(game.player.object->angley*M_PI/180.0f);
                eyez=tarz - 3*cos(game.player.object->angley*M_PI/180.0f);
                eyey=tary + firstpersonheight;
                break;
            case 4:
                double mos_x,mos_y;
                glfwGetCursorPos(window, &mos_x, &mos_y);
                if (clicked) {
                    xdiff += mos_x -  startx;
                    ydiff += mos_y - starty;
                }
                tarx=0;
                tary=0;
                tarz=0;
                eyex=game.helicopterheight*sin(ydiff/game.height)*sin(xdiff/game.width);
                eyez=game.helicopterheight*sin(ydiff/game.height)*cos(xdiff/game.width);
                eyey=game.helicopterheight*cos(ydiff/game.height);
                break;

        }
        
        // Eye - Location of camera. Don't change unless you are sure!!
        glm::vec3 eye (eyex, eyey, eyez);
        // Target - Where is the camera looking at.  Don't change unless you are sure!!
        glm::vec3 target (tarx,tary,tarz);
        // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
        glm::vec3 up (0, 1, 0);
        // Compute Camera matrix (view)
        Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
        
        VP = Matrices.projection * Matrices.view;
        Matrices.model = glm::mat4(1.0f);
        
        glm::mat4 translateTriangle = glm::translate (glm::vec3(0, 0, 0.0f)); // glTranslatef
        // rotate about vector (1,0,0)
        Matrices.model *= translateTriangle;
        MVP = VP * Matrices.model; // MVP = p * V * M
        
        //  Don't change unless you are sure!!
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        
        // draw3DObject draws the VAO given to it using current MVP matrix
        draw3DObject(game.skybox);
        int i;
        for (i=0; i<game.objectindex; i++) {
            Matrices.model = glm::mat4(1.0f);
            /* Render your scene */
            
            glm::mat4 translateTriangle = glm::translate (glm::vec3(game.objects[i].posx, game.objects[i].posy, game.objects[i].posz)); // glTranslatef
            glm::mat4 rotateTriangle = glm::rotate((float)(game.objects[i].anglez*M_PI/180.0f), glm::vec3(0,0,1));
            rotateTriangle *= glm::rotate((float)(game.objects[i].anglex*M_PI/180.0f), glm::vec3(1,0,0));
            rotateTriangle *= glm::rotate((float)(game.objects[i].angley*M_PI/180.0f), glm::vec3(0,1,0));// rotate about vector (1,0,0)
            glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
            Matrices.model *= triangleTransform;
            MVP = VP * Matrices.model; // MVP = p * V * M
            
            //  Don't change unless you are sure!!
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            
            // draw3DObject draws the VAO given to it using current MVP matrix
            draw3DObject(game.objects[i].vao);
            
        }
        Matrices.model = glm::mat4(1.0f);
        float color[2][3]= {
            {
                255,255,125
            },
            {
                255,0,0
            }
        };
        /* Render your scene */
        float body[8][7] =
        {
            {
                0,0.75,0,0.5,0.5,0.2,0
            },
            {
                0,0,0,0.5,1,0.5,0
            },
            {
                0.25,0.5,0,0.5,0.2,0.2,0
            },
            {
                -0.25,0.5,0,0.5,0.2,0.2,0
            },
            {
                0.5,0.25,0,0.2,0.5,0.2,0
            },
            {
                -0.5,0.25,0,0.2,0.5,0.2,0
            },
            {
                0.25,-0.75,0,0.2,0.5,0.2,0
            },
            {
                -0.25,-0.75,0,0.2,0.5,0.2,0
            }
        };
        for(i=0;i<8;i++){
            Matrices.model = glm::mat4(1.0f);
            glm::mat4 translateTriangle = glm::translate (glm::vec3( game.player.object -> posx, game.player.object -> posy, game.player.object -> posz)); // glTranslatef
            glm::mat4 rotateTriangle = glm::rotate((float)(game.player.object -> anglez*M_PI/180.0f), glm::vec3(0,0,1));
            rotateTriangle *= glm::rotate((float)(game.player.object -> anglex*M_PI/180.0f), glm::vec3(1,0,0));
            rotateTriangle *= glm::rotate((float)(game.player.object -> angley*M_PI/180.0f), glm::vec3(0,1,0));// rotate about vector (1,0,0)
            glm::mat4 translateAfterTriangle= glm::translate (glm::vec3(body[i][0], body[i][1], body[i][2]));

            glm::mat4 triangleTransform = translateTriangle * rotateTriangle *translateAfterTriangle;
            Matrices.model *= triangleTransform;
            MVP = VP * Matrices.model;
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            draw3DObject(createCube(body[i][3], body[i][4], body[i][5], color));
        }
    }
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle
    
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow(width, height, "Angry Birds -  By Mukul Hase", NULL, NULL);
    
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );
    
    /* --- register callbacks with GLFW --- */
    
    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);
    
    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);
    
    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling
    glfwSetScrollCallback(window, scroll_callback);

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    
    return window;
}

void createSquare()
{
    static const GLfloat vertex_buffer_data [] = {
        -0.04f,0.04f,0, // vertex 1
        0.04f,0.04f,0, // vertex 2
        0.04f, -0.04f,0, // vertex 3
        
        0.04f, -0.04f,0, // vertex 3
        -0.04f, -0.04f,0, // vertex 4
        -0.04f,0.04f,0  // vertex 1
    };
    
    static const GLfloat color_buffer_data [] = {
        0.3,0.3,1, // color 1
        0.3,0.3,1,
        0.3,0.3,1,
        0.3,0.3,1,
        0.3,0.3,1,
        0.3,0.3,1
    };
    
    // create3DObject creates and returns a handle to a VAO that can be used later
    square = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createTriangle (float scale)
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */
    
    /* Define vertex array as used in glBegin (GL_TRIANGLES) */
    static const GLfloat vertex_buffer_data [] = {
        0, scale,0, // vertex 0
        -scale*cos(M_PI/6),-scale*sin(M_PI/6),0, // vertex 1
        scale*cos(M_PI/6),-scale*sin(M_PI/6),0, // vertex 2
    };
    
    static const GLfloat color_buffer_data [] = {
        0,1,1, // color 0
        0,1,1, // color 1
        0,1,1, // color 2
    };
    
    // create3DObject creates and returns a handle to a VAO that can be used later
    triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}
VAO* createCube (float l, float h, float b, float color[2][3])
{
    // GL3 accepts only Triangles. Quads are not supported
    float x=l/2, y=h/2, z=b/2;
    GLfloat vertex_buffer_data [] = {
        -x,-y,-z, // triangle 1 : begin
        -x,-y, z,
        -x, y, z,
        -x,-y,-z,//
        -x, y, z,
        -x, y,-z,
        
        x, y, z,//
        x,-y,-z,
        x, y,-z,
        x,-y,-z,//
        x, y, z,
        x,-y, z,
        
        x, y,-z,//
        x,-y,-z,
        -x,-y,-z,
        x, y,-z, // triangle 2 : begin
        -x,-y,-z,
        -x, y,-z,
        
        -x, y, z,//
        -x,-y, z,
        x,-y, z,
        x, y, z,//
        -x, y, z,
        x,-y, z,
        
        x,-y, z,//
        -x,-y,-z,
        x,-y,-z,
        x,-y, z,//
        -x,-y, z,
        -x,-y,-z,
        
        x, y, z,//
        x, y,-z,
        -x, y,-z,
        x, y, z,//
        -x, y,-z,
        -x, y, z,
    };
    GLfloat color_buffer_data [] = {
        
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 1
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 2
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 3
        
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 3
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 4
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 1
        
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 1
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 2
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 3
        
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 3
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 4
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 1
        
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 1
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 2
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 3
        
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 3
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 4
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 1
        
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 1
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 2
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 3
        
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 3
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 4
        color[0][0]/255.0f,color[0][1]/255.0f,color[0][2]/255.0f, // color 1
        
        color[1][0]/300.0f,color[1][1]/300.0f,color[1][2]/300.0f, // color 1
        color[1][0]/255.0f,color[1][1]/255.0f,color[1][2]/255.0f, // color 2
        color[1][0]/300.0f,color[1][1]/300.0f,color[1][2]/300.0f, // color 3
        
        color[1][0]/300.0f,color[1][1]/300.0f,color[1][2]/300.0f, // color 3
        color[1][0]/255.0f,color[1][1]/255.0f,color[1][2]/255.0f, // color 4
        color[1][0]/300.0f,color[1][1]/300.0f,color[1][2]/300.0f, // color 1
        
        color[1][0]/300.0f,color[1][1]/300.0f,color[1][2]/300.0f, // color 1
        color[1][0]/255.0f,color[1][1]/255.0f,color[1][2]/255.0f, // color 2
        color[1][0]/300.0f,color[1][1]/300.0f,color[1][2]/300.0f, // color 3
        
        color[1][0]/300.0f,color[1][1]/300.0f,color[1][2]/300.0f, // color 3
        color[1][0]/255.0f,color[1][1]/255.0f,color[1][2]/255.0f, // color 4
        color[1][0]/300.0f,color[1][1]/300.0f,color[1][2]/300.0f, // color 1
        
    };
    /*static const GLfloat color_buffer_data [] = {
        
        0.5,0.5,0.5, // color 1
        0.5,0.5,0.5, // color 2
        0.5,0.5,0.5, // color 3
        0.5,0.5,0.5, // color 1
        0.5,0.5,0.5, // color 2
        0.5,0.5,0.5, // color 3
        
        0.5,0.5,0.5, // color 1
        0.5,0.5,0.5, // color 2
        0.5,0.5,0.5, // color 3
        0.5,0.5,0.5, // color 1
        0.5,0.5,0.5, // color 2
        0.5,0.5,0.5,
        
        0.5,0.5,0.5, // color 1
        0.5,0.5,0.5, // color 2
        0.5,0.5,0.5, // color 3
        0.5,0.5,0.5, // color 1
        0.5,0.5,0.5, // color 2
        0.5,0.5,0.5,
        
        0.5,0.5,0.5, // color 1
        0.5,0.5,0.5, // color 2
        0.5,0.5,0.5, // color 3
        0.5,0.5,0.5, // color 1
        0.5,0.5,0.5, // color 2
        0.5,0.5,0.5,
        
        1,0,0, // color 1
        0,0,1, // color 2
        0,1,0,
        1,0,0, // color 1
        0,0,1, // color 2
        0,1,0, // color 3
        
        1,0,0, // color 1
        0,0,1, // color 2
        0,1,0, // color 3
        1,0,0, // color 1
        0,0,1, // color 2
        0,1,0 // color 3
        
    };
    */
    // create3DObject creates and returns a handle to a VAO that can be used later
    return create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
    // Create the models
    createTriangle (0.2);
    createSquare();
    float color[2][3]= {
        {
            0,125,125
        },
        {
            125,125,0
        }
    };
    cube = createCube(2.0,2.0,2.0,color);
    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");
    
    
    reshapeWindow (window, width, height);
    
    // Background color of the scene
    glClearColor (0.0f, 0.0f, 0.0f, 0.0f); // R, G, B, A
    glClearDepth (1.0f);
    
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);
    
    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}


int main (int argc, char** argv)
{
    string option,name;
    game.preset();
    window = initGLFW(game.width, game.height);
    initGL (window, game.width, game.height);
    
    while (1) {
        while (game.paused) {
            draw();
            
            // Swap Frame Buffer in double buffering
            glfwSwapBuffers(window);
            
            // Poll for Keyboard and mouse events
            glfwPollEvents();
            
        }
        game.setup();
        cout << "Start Game?(Y/N)" << endl;
        cin >> option;
        if (strcmp(option.c_str(),"N")==0) {
            break;
        }
        cout << "Enter Name of map:-" <<endl;
        cin >> name;
        std::ifstream file(name.c_str());
        for(int i = 0; i < 10; i++)
        {
            std::string row;
            if (file >> row) {
                for (int j = 0; j != std::min<int>(10, row.length()) ; ++j)
                {
                    game.map[j][i] = row[j];
                }
            } else break;
        }
        double last_update_time = glfwGetTime(), current_time;
        double start_time = glfwGetTime();

        /* Draw in loop */
        game.init();

        while (!glfwWindowShouldClose(window)) {
            
            
            
            // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
            current_time = glfwGetTime(); // Time in seconds
            if ((current_time - last_update_time) >= 0.01) { // atleast 0.5s elapsed since last frame
                // do something every 0.5 seconds ..
                game.update();
                string mode="";
                switch (game.cameramode) {
                    case 0:
                        mode="Tower View ";
                        break;
                    case 1:
                        mode="Top View ";
                        break;
                    case 2:
                        mode="First Person View ";
                        break;
                    case 3:
                        mode="Third Person View ";
                        break;
                    case 4:
                        mode="Helicopter View ";
                        break;
                }
                glfwSetWindowTitle(window,(mode + "- " + to_string(current_time)).c_str());
                if (game.ended==true) {
                    glfwSetWindowTitle(window, ("Game Over, You Won! - Go to terminal"));
                    game.paused = true;
                    break;
                }
                last_update_time = current_time;
                // OpenGL Draw commands
                draw();
                
                // Swap Frame Buffer in double buffering
                glfwSwapBuffers(window);
                
                // Poll for Keyboard and mouse events
                glfwPollEvents();
                
            }
        }
    }
    cout << "Ended Game" << endl;
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
