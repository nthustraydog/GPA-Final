#include "../Externals/Include/Include.h"
#include "../Externals/Include/assimp/Importer.hpp"  
#include "../Externals/Include/assimp/scene.h"      
#include "../Externals/Include/assimp/postprocess.h"

#include "../VC14/Model.h"
#include "../VC14/Texture.h"
#include "../VC14/Camera.h"
#include "../VC14/Skybox.h"
#include <vector>

#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2
#define MENU_EXIT 3
#define SHADOW_MAP_SIZE 4096

GLubyte timer_cnt = 0;
bool timer_enabled = true;
unsigned int timer_speed = 16;
float perspect_rad = 0.0f;

using namespace glm;
using namespace std;

mat4 view;
mat4 projection;
mat4 model;

GLuint shadow_tex;
GLint light_mvp;
GLint shadow_matrix;
GLint light_matrix;

GLint um4p;
GLint um4mv;
GLint color;

Model objModel;
Model objMainModel;
GLuint program;
GLuint depthProg;

const int WINDOW_WIDTH = 600, WINDOW_HEIGHT = 600;
GLfloat lastX = WINDOW_WIDTH / 2.0f, lastY = WINDOW_HEIGHT / 2.0f;
GLfloat pressed_X = WINDOW_WIDTH / 2.0f, pressed_Y = WINDOW_HEIGHT / 2.0f; // record press coordinate
GLfloat deltaTime = 16.0f;
GLfloat rotateAngle = -90.0f;
bool MouseLeftPressed = false;
bool firstMouseMove = true;
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
Skybox* skybox;

struct
{
	GLuint fbo;
	GLuint depthMap;
} shadowBuffer;

const char *depth_vs[] =
{
	"#version 410 core                         \n"
	"                                          \n"
	"uniform mat4 mvp;                         \n"
	"                                          \n"
	"layout (location = 0) in vec3 iv3vertex;   \n"
	"                                          \n"
	"void main(void)                           \n"
	"{                                         \n"
	"    gl_Position = mvp * vec4(iv3vertex, 1.0);         \n"
	"}                                         \n"
};

const char *depth_fs[] =
{
	"#version 410 core                                \n"
	"                                                 \n"
	"out vec4 fragColor;                              \n"
	"                                                 \n"
	"void main()                                      \n"
	"{                                                \n"
	"    fragColor = vec4(vec3(gl_FragCoord.z), 1.0); \n"
	"}                                                \n"
};


char** loadShaderSource(const char* file)
{
    FILE* fp = fopen(file, "rb");
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *src = new char[sz + 1];
    fread(src, sizeof(char), sz, fp);
    src[sz] = '\0';
    char **srcp = new char*[1];
    srcp[0] = src;
    return srcp;
}

void freeShaderSource(char** srcp)
{
    delete[] srcp[0];
    delete[] srcp;
}


// define a simple data structure for storing texture image raw data
/*typedef struct _TextureData
{
    _TextureData(void) :
        width(0),
        height(0),
        data(0)
    {
    }

    int width;
    int height;
    unsigned char* data;
} TextureData;*/

// load a png image and return a TextureData structure with raw data
// not limited to png format. works with any image format that is RGBA-32bit
/*TextureData loadPNG(const char* const pngFilepath)
{
    TextureData texture;
    int components;

    // load the texture with stb image, force RGBA (4 components required)
    stbi_uc *data = stbi_load(pngFilepath, &texture.width, &texture.height, &components, 4);

    // is the image successfully loaded?
    if (data != NULL)
    {
        // copy the raw data
        size_t dataSize = texture.width * texture.height * 4 * sizeof(unsigned char);
        texture.data = new unsigned char[dataSize];
        memcpy(texture.data, data, dataSize);

        // mirror the image vertically to comply with OpenGL convention
        for (size_t i = 0; i < texture.width; ++i)
        {
            for (size_t j = 0; j < texture.height / 2; ++j)
            {
                for (size_t k = 0; k < 4; ++k)
                {
                    size_t coord1 = (j * texture.width + i) * 4 + k;
                    size_t coord2 = ((texture.height - j - 1) * texture.width + i) * 4 + k;
                    std::swap(texture.data[coord1], texture.data[coord2]);
                }
            }
        }

        // release the loaded image
        stbi_image_free(data);
    }

    return texture;
}*/
void LoadModel_custom(const string &objFilePath, Model *model) {
	if (!model->loadModel(objFilePath)) {
		printf("Fail to load model\n");
		return;
	}
}
void LoadModel(const string &objFilePath) 
{
	if (!objModel.loadModel(objFilePath)) {
		printf("Fail to load model\n");
		return;
	}
}

void My_Init()
{
	string objMainPath = "./Arabic+City.obj";
	string objFilePath = "./humvee.obj";

    glClearColor(0.0f, 0.6f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glDepthFunc(GL_LEQUAL);

	// ----- Begin Initialize Depth Shader Program -----
	GLuint shadow_vs;
	GLuint shadow_fs;
	// shadow vertex shader
	shadow_vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shadow_vs, 1, depth_vs, 0);
	glCompileShader(shadow_vs);
	// shadow fragment shader
	shadow_fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shadow_fs, 1, depth_fs, 0);
	glCompileShader(shadow_fs);
	// create depth program
	depthProg = glCreateProgram();
	glAttachShader(depthProg, shadow_vs);
	glAttachShader(depthProg, shadow_fs);
	glLinkProgram(depthProg);
	// get location of light mvp
	light_mvp = glGetUniformLocation(depthProg, "mvp");
	// ----- End Initialize Depth Shader Program -----

	// ----- Begin Initialize Blinn-Phong Shader Program -----
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	char** vertexShaderSource = loadShaderSource("vertex.vs.glsl");
	char** fragmentShaderSource = loadShaderSource("fragment.fs.glsl");
	
	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);

	freeShaderSource(vertexShaderSource);
	freeShaderSource(fragmentShaderSource);

	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	glUseProgram(program);
	um4p = glGetUniformLocation(program, "um4p");
	um4mv = glGetUniformLocation(program, "um4mv");
	shadow_matrix = glGetUniformLocation(program, "shadow_matrix");
	shadow_tex = glGetUniformLocation(program, "shadow_tex");
	// ----- End Initialize Blinn-Phong Shader Program -----

	// ----- Begin Initialize Main Model -----
	LoadModel_custom(objFilePath, &objModel);
	LoadModel_custom(objMainPath, &objMainModel);
	//LoadModel(objFilePath);
	//LoadModel(objMain);

	skybox = new Skybox(std::vector<std::string>{"right.jpg",
		"left.jpg",
		"top.jpg",
		"bottom.jpg",
		"back.jpg",
		"front.jpg"}, camera.position, view, projection);
	// ----- End Initialize Main Model -----

	

	// ----- Begin Initialize Shadow Framebuffer Object -----
	glGenFramebuffers(1, &shadowBuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer.fbo);

	glGenTextures(1, &shadowBuffer.depthMap);
	glBindTexture(GL_TEXTURE_2D, shadowBuffer.depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowBuffer.depthMap, 0);
	// ----- End Initialize Shadow Framebuffer Object -----
}

void My_Display()
{
	// ----- Begin Shadow Map Pass -----
	const float shadow_range = 5.0f;
	mat4 scale_bias_matrix =
		translate(mat4(), vec3(0.5f, 0.5f, 0.5f)) *
		scale(mat4(), vec3(0.5f, 0.5f, 0.5f));
	mat4 light_proj_matrix = ortho(-shadow_range, shadow_range, -shadow_range, shadow_range, 0.0f, 5000.0f);
	mat4 light_view_matrix = lookAt(vec3(0.0f, 1000.0f, -1000.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 light_vp_matrix = light_proj_matrix * light_view_matrix;
	mat4 shadow_sbpv_matrix = scale_bias_matrix * light_vp_matrix;
	mat4 r = rotate(mat4(), radians(rotateAngle), vec3(1.0, 0.0, 0.0));

	// model matrix
	model = mat4();

	glUseProgram(depthProg);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer.fbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 4.0f);
	glUniformMatrix4fv(light_mvp, 1, GL_FALSE, value_ptr(light_vp_matrix * model));
	objMainModel.ShadowDraw();

	glDisable(GL_POLYGON_OFFSET_FILL);
	// ----- End Shadow Map Pass -----


	// ----- Begin Blinn-Phong Shading Pass -----
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glUseProgram(program);

	projection = glm::perspective(camera.mouse_zoom,
		(GLfloat)(WINDOW_WIDTH) / WINDOW_HEIGHT, 1.0f, 10000.0f);
	view = camera.getViewMatrix();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowBuffer.depthMap);
	glUniform1i(shadow_tex, 0);

	mat4 mat4_shadow_matrix = shadow_sbpv_matrix * model;
	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));
	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniformMatrix4fv(shadow_matrix, 1, GL_FALSE, value_ptr(mat4_shadow_matrix));

	objMainModel.Draw(program);
	// ----- End Blinn-Phong Shading Pass -----
	

	//glUseProgram(program);
	//glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model * r));
	//glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	//objModel.Draw(program);
	//objMainModel.Draw(program);

	skybox->draw();
    glutSwapBuffers();
}

void My_Reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	float viewportAspect = (float)width / (float)height;
	projection = perspective(radians(60.0f), viewportAspect, 0.1f, 1000.0f);
	view = lookAt(vec3(perspect_rad, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
}

void My_Timer(int val)
{
	glutPostRedisplay();
	glutTimerFunc(timer_speed, My_Timer, val);
}

void My_Mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
		//Record the Scrolling of Mouse to Move the Scene
		pressed_X = (float)x;
		pressed_Y = (float)y;
	}
	else if (state == GLUT_UP)
	{
		printf("Mouse %d is released at (%d, %d)\n", button, x, y);
	}
	
	/*if (button == 3 || button == 4)
	{
		if (state == GLUT_UP) {
			printf("Scroll (pressed) %s At (%d, %d)\n", (button == 3)? "Up" : "Down", x, y);
		}
		else {
			printf("Scroll (released) %s At (%d, %d)\n", (button == 3) ? "Up" : "Down", x, y);
		}
	}
	else 
	{
		if (button == GLUT_LEFT_BUTTON && !MouseLeftPressed && state == GLUT_DOWN) {
			pressed_X = x;
			pressed_Y = y;
			MouseLeftPressed = true;
			printf("Left Mouse Button is pressed at (%d, %d)\n", x, y);
		}
		else if (button == GLUT_LEFT_BUTTON && MouseLeftPressed && state == GLUT_UP) {
			GLfloat xoffset = x - pressed_X;
			GLfloat yoffset = pressed_Y - y;

			camera.handleMouseHoldMove(xoffset, yoffset);
			MouseLeftPressed = false;
			printf("Left Mouse Button is released at (%d, %d)\n", x, y);
		}
	}*/
}

//The Function that controls with the Mouse
//First Introduced in "Computer Graphics" Color Model HW1
//The Motion Function is Registered Below
void onMouseMotion(int x, int y) {
	GLfloat xoffset = x - pressed_X;
	GLfloat yoffset = y - pressed_Y;
	pressed_X = x;
	pressed_Y = y;


	/*Camera_Setting();
	MV = lookAt(Position_View, Position_View + Front_View, Up_View);*/

	camera.handleMouseMove(xoffset, -yoffset);
}


/*void My_Move(int x, int y) {
	printf("Mouse moving... starting at (%d, %d)\n", x, y);
	if (firstMouseMove)
	{
		lastX = x;
		lastY = y;
		firstMouseMove = false;
	}

	GLfloat xoffset = x - lastX;
	GLfloat yoffset = lastY - y;

	lastX = x;
	lastY = y;

	camera.handleMouseMove(xoffset, yoffset);
}*/

void My_Keyboard(unsigned char key, int x, int y)
{
	if (key == 'w') {
		camera.handleKeyPress(FORWARD, deltaTime);
		printf("Button %c is pressed at (%d, %d)\n", key, x, y);
	}
	else if (key == 's') {
		camera.handleKeyPress(BACKWARD, deltaTime);
		printf("Button %c is pressed at (%d, %d)\n", key, x, y);
	}
	else if (key == 'a') {
		camera.handleKeyPress(LEFT, deltaTime);
		printf("Button %c is pressed at (%d, %d)\n", key, x, y);
	}
	else if (key == 'd') {
		camera.handleKeyPress(RIGHT, deltaTime);
		printf("Button %c is pressed at (%d, %d)\n", key, x, y);
	}
	else if (key == 'z') {
		camera.handleKeyPress(UP, deltaTime);
		printf("Button %c is pressed at (%d, %d)\n", key, x, y);
	}
	else if (key == 'x') {
		camera.handleKeyPress(DOWN, deltaTime);
		printf("Button %c is pressed at (%d, %d)\n", key, x, y);
	}
	else if (key == 'r') {
		camera.handleKeyPress(RESET, deltaTime);
		printf("Button %c is pressed at (%d, %d)\n", key, x, y);
	}
	else if (key == 'v') {
		objModel.Move(100, 0, 0);
		printf("Button %c is pressed at (%d, %d)\n", key, x, y);
	}
}


/*TRY: FIX THE MOUSE HOVERING ISSUE*/


void My_SpecialKeys(int key, int x, int y)
{
	switch(key)
	{
	case GLUT_KEY_F1:
		printf("F1 is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_UP:
		printf("Page up is pressed at (%d, %d)\n", x, y);
		perspect_rad += 1.0f;
		My_Reshape(600, 600);
		break;
	case GLUT_KEY_DOWN:
		printf("Left arrow is pressed at (%d, %d)\n", x, y);
		perspect_rad -= 1.0f;
		My_Reshape(600, 600);
		break;
	default:
		printf("Other special key is pressed at (%d, %d)\n", x, y);
		break;
	}
}

void My_Menu(int id)
{
	switch(id)
	{
	case MENU_TIMER_START:
		if(!timer_enabled)
		{
			timer_enabled = true;
			glutTimerFunc(timer_speed, My_Timer, 0);
		}
		break;
	case MENU_TIMER_STOP:
		timer_enabled = false;
		break;
	case MENU_EXIT:
		exit(0);
		break;
	default:
		break;
	}
}

int main(int argc, char *argv[])
{
#ifdef __APPLE__
    // Change working directory to source code path
    chdir(__FILEPATH__("/../Assets/"));
#endif
	// Initialize GLUT and GLEW, then create a window.
	////////////////////
	glutInit(&argc, argv);
#ifdef _MSC_VER
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(600, 600);
	glutCreateWindow("AS2_Framework"); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
	glewInit();
#endif
    glPrintContextInfo();
	My_Init();

	// Create a menu and bind it to mouse right button.
	int menu_main = glutCreateMenu(My_Menu);
	int menu_timer = glutCreateMenu(My_Menu);

	glutSetMenu(menu_main);
	glutAddSubMenu("Timer", menu_timer);
	glutAddMenuEntry("Exit", MENU_EXIT);

	glutSetMenu(menu_timer);
	glutAddMenuEntry("Start", MENU_TIMER_START);
	glutAddMenuEntry("Stop", MENU_TIMER_STOP);

	glutSetMenu(menu_main);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// Register GLUT callback functions.
	glutDisplayFunc(My_Display);
	glutReshapeFunc(My_Reshape);

	glutMouseFunc(My_Mouse);
	//glutMotionFunc(My_Move);
	//glutPassiveMotionFunc(My_Move);

	//Dedcated To "Computer Graphics" on Moving Mouse Hover
	glutMotionFunc(onMouseMotion);

	glutKeyboardFunc(My_Keyboard);
	glutSpecialFunc(My_SpecialKeys);
	glutTimerFunc(timer_speed, My_Timer, 0); 

	// Enter main event loop.
	glutMainLoop();

	delete skybox;

	return 0;
}
