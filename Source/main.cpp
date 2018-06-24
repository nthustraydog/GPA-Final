#include "../Externals/Include/Include.h"
#include "../Externals/Include/assimp/Importer.hpp"  
#include "../Externals/Include/assimp/scene.h"      
#include "../Externals/Include/assimp/postprocess.h"

#include "../VC14/Model.h"
#include "../VC14/Texture.h"
#include "../VC14/OrbitCamera.h"
#include "../VC14/Skybox.h"
#include "../VC14/common.h"

#include <vector>

#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2
#define MENU_LIGHTEFFECT 3
#define MENU_FOGEFFECT 4
#define MENU_NORMALMAP 5
#define MENU_SHADOWEFFECT 6
#define MENU_EXIT 7
#define MENU_NAVIGATION 8
#define MENU_NAVIGATIONRUN 9

GLubyte timer_cnt = 0;
bool timer_enabled = true;
unsigned int timer_speed = 16;
float perspect_rad = 0.0f;

using namespace glm;
using namespace std;

mat4 view;
mat4 projection;
mat4 model;

GLint um4p;
GLint um4mv;
GLint color;

Model objModel;
Model objCar;
GLuint program;

const int WINDOW_WIDTH = 1066, WINDOW_HEIGHT = 600;
GLfloat lastX = WINDOW_WIDTH / 2.0f, lastY = WINDOW_HEIGHT / 2.0f;
GLfloat pressed_X = WINDOW_WIDTH / 2.0f, pressed_Y = WINDOW_HEIGHT / 2.0f; // record press coordinate
GLfloat deltaTime = 16.0f;
bool MouseLeftPressed = false;
bool firstMouseMove = true;
OrbitCamera camera;

Skybox* skybox;
const GLuint SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
GLuint depthProgram;
GLuint lightSpaceMatrixLocation;
GLuint modelLocation;
GLuint depthMapFBO;
GLuint depthMap;
GLuint depthMapLocation;

int lightEffect = 1;
int fogEffect = 0;
int normalMapEffect = 1;
int shadowMapEffect = 1;
GLuint lightEffect_switch;
GLuint fogEffect_switch;
GLuint normalMap_switch;
GLuint shadowMap_switch;

int navigationSwitch = 0;
int navigationRun = 0;
void My_Navigation();
float navRadius = 1500.0f;
float navHeight = 500.0f;
float navSpeed = 1;

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

void My_Navigation()
{
	static float previosTime = 0.0f;
	static float currentTime = 0.0f;
	static float navigationTime = 0.0f;

	currentTime = glutGet(GLUT_ELAPSED_TIME);
	float deltaTime = (currentTime - previosTime) / 1000.0f * navSpeed;
	if (navigationRun == 1)
	{
		navigationTime = navigationTime + deltaTime;
	}
	float navigationX = sin(navigationTime) * navRadius + -47.9799f;
	float navigationZ = cos(navigationTime) * navRadius + -999.456f;
	float navigationY = navHeight;
	camera.setPosition(vec3(navigationX, navigationY, navigationZ));
	camera.setTarget(vec3(-47.9799f, 76.9309f, -999.456f));
	//cout << navigationTime << " " << navigationX << " " << navigationY << " " << navigationZ << endl;
	previosTime = currentTime;
}

void LoadModel_Custom(const string &objFilePath, Model *model)
{
	if (!model->loadModel(objFilePath)) {
		printf("Fail to load model: %s\n", objFilePath.c_str());
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

void shadowMapSetup()
{
	// Program
	depthProgram = glCreateProgram();
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	char** vertexShaderSource = loadShaderSource("depth.vs.glsl");
	char** fragmentShaderSource = loadShaderSource("depth.fs.glsl");
	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
	freeShaderSource(vertexShaderSource);
	freeShaderSource(fragmentShaderSource);
	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);
	shaderLog(vertexShader);
	shaderLog(fragmentShader);
	glAttachShader(depthProgram, vertexShader);
	glAttachShader(depthProgram, fragmentShader);
	glLinkProgram(depthProgram);
	lightSpaceMatrixLocation = glGetUniformLocation(depthProgram, "lightSpaceMatrix");
	modelLocation = glGetUniformLocation(depthProgram, "model");
	// Gen FBO
	glGenFramebuffers(1, &depthMapFBO);
	// Get texture
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Attach to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void My_Init()
{
    glClearColor(0.0f, 0.6f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glDepthFunc(GL_LEQUAL);

	program = glCreateProgram();
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	char** vertexShaderSource = loadShaderSource("vertex.vs.glsl");
	char** fragmentShaderSource = loadShaderSource("fragment.fs.glsl");
	string objFilePath = "./Arabic+City.obj";
	string objCarPath = "./humvee.obj";

	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);

	freeShaderSource(vertexShaderSource);
	freeShaderSource(fragmentShaderSource);

	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	shaderLog(vertexShader);
	shaderLog(fragmentShader);

	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	glLinkProgram(program);
	um4p = glGetUniformLocation(program, "um4p");
	um4mv = glGetUniformLocation(program, "um4mv");

	lightEffect_switch = glGetUniformLocation(program, "lightEffect_switch");
	fogEffect_switch = glGetUniformLocation(program, "fogEffect_switch");
	normalMap_switch = glGetUniformLocation(program, "normalMap_switch");
	shadowMap_switch = glGetUniformLocation(program, "shadowMap_switch");
	depthMapLocation = glGetUniformLocation(program, "depthMap");

	glUseProgram(program);

	LoadModel_Custom(objFilePath, &objModel);
	LoadModel_Custom(objCarPath, &objCar);

	skybox = new Skybox(std::vector<std::string>{"right.jpg",
		"left.jpg",
		"top.jpg",
		"bottom.jpg",
		"back.jpg",
		"front.jpg"}, 
		camera.getPosition(), camera.getViewingMatrix(), camera.getPerspectiveMatrix());
	shadowMapSetup();
}

void My_Display()
{
	// Shadow map
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(depthProgram);
	GLfloat rotateAngle = -90.0f;
	glm::mat4 lightViewing = glm::lookAt(glm::vec3(1000.0f, 2000.0f, -1000.0f), glm::vec3(1000.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightProj = glm::ortho(-3500.0f, 3500.0f, -3500.0f, 3500.0f, 800.0f, 4000.0f);
	glm::mat4 lightSpace = lightProj * lightViewing;

	// for car position
	glm::mat4 r = rotate(mat4(), radians(rotateAngle), vec3(1.0, 0.0, 0.0));
	glm::mat4 t = translate(mat4(), vec3(-1600.0f, -200.0f, -10.0f));
	glm::mat4 s = scale(mat4(), vec3(0.5f, 0.5f, 0.5f));

	glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, value_ptr(lightSpace));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, value_ptr(mat4(1.0f)));
	glCullFace(GL_FRONT);
	objModel.Draw(depthProgram);

	// draw depth model matrix for car, need rotate, translate and scale
	glUseProgram(depthProgram);
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, value_ptr(mat4(1.0f) * r * t * s));
	objCar.Draw(depthProgram);

	glCullFace(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	// end

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	model = mat4();

	glUseProgram(program);

	camera.update(timer_speed / 1000.0f);
	if (navigationSwitch)
	{
		My_Navigation();
	}
	projection = camera.getPerspectiveMatrix();
	view = camera.getViewingMatrix(); 

	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));
	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
	glUniform1i(lightEffect_switch, lightEffect);
	glUniform1i(fogEffect_switch, fogEffect);
	glUniform1i(normalMap_switch, normalMapEffect);
	glUniform1i(shadowMap_switch, shadowMapEffect);
	glUniformMatrix4fv(glGetUniformLocation(program, "lightSpaceMatrix") , 1, GL_FALSE, value_ptr(lightSpace));
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(depthMapLocation, 9);
	objModel.Draw(program);

	glUseProgram(program);
	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model * r * t * s));
	objCar.Draw(program);

	skybox->draw();

	vec3 position = camera.getPosition();

	printf("camera: x = %f, y = %f, z = %f\n", position.x, position.y, position.z);

    glutSwapBuffers();
}

void My_Reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	float viewportAspect = (float)width / (float)height;
	projection = perspective(radians(60.0f), viewportAspect, 0.1f, 1000.0f);
	view = lookAt(vec3(perspect_rad, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

	camera.setAspect(viewportAspect);
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

	camera.onMouse(button, state, x, y);
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

	camera.onMotion(x, y);
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
	camera.onKeyboard(key, x, y);
	switch (key)
	{
	case 'f':
	case 'F':
		if (navigationSwitch == 1)
		{
			navRadius = navRadius - 100.0f;
			printf("\nThe New Radius is %f", navRadius);
		}
		break;
	case 'h':
	case 'H':
		if (navigationSwitch == 1)
		{
			navRadius = navRadius + 100.0f;
			printf("\nThe New Radius is %f", navRadius);
		}
		break;
	case 'g':
	case 'G':
		if (navigationSwitch == 1)
		{
			navHeight = navHeight - 100.0f;
			printf("\nThe New Height is %f", navHeight);
		}
		break;
	case 't':
	case 'T':
		if (navigationSwitch == 1)
		{
			navHeight = navHeight + 100.0f;
			printf("\nThe New Height is %f", navHeight);
		}
		break;
	case 'r':
	case 'R':
		if (navigationSwitch == 1)
		{
			navSpeed = navSpeed / 2;
			printf("\nThe New Speed is %f", navSpeed);
		}
		break;
	case 'y':
	case 'Y':
		if (navigationSwitch == 1)
		{
			navSpeed = navSpeed * 2;
			printf("\nThe New Speed is %f", navSpeed);
		}
		break;
	}
}

void My_Keyboard_Up(unsigned char key, int x, int y)
{
	camera.onKeyboardUp(key, x, y);
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
	case MENU_LIGHTEFFECT:
		lightEffect = (lightEffect + 1) % 2;
		printf("Light Effect: %s\n", (lightEffect == 1) ? "ON" : "OFF");
		break;
	case MENU_FOGEFFECT:
		if (fogEffect == 1)
		{
			fogEffect = 0;
			printf("Fog Effect: OFF\n");
		}
		else if (fogEffect == 0)
		{
			fogEffect = 1;
			printf("Fog Effect: ON\n");
		}
		break;
	case MENU_NORMALMAP:
		normalMapEffect = (normalMapEffect + 1) % 2;
		printf("Normal Mapping Effect: %s\n", (normalMapEffect == 1)? "ON" : "OFF");
		break;
	case MENU_SHADOWEFFECT:
		shadowMapEffect = (shadowMapEffect + 1) % 2;
		printf("Shadow Mapping Effect: %s\n", (shadowMapEffect == 1) ? "ON" : "OFF");
		break;
	case MENU_NAVIGATION:
		if (navigationSwitch == 1)
		{
			navigationSwitch = 0;
			navigationRun = 0;
			printf("Auto Navigation: OFF\n");
		}
		else if (navigationSwitch == 0)
		{
			navigationSwitch = 1;
			navigationRun = 1;
			printf("Auto Navigation: ON\n");
		}
		break;
	case MENU_NAVIGATIONRUN:
		if (navigationRun == 0)
		{
			navigationRun = 1;
			printf("Continue Navigation\n");
		}
		else if (navigationRun == 1)
		{
			navigationRun = 0;
			printf("Pause Navigation\n");
		}
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
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("AS2_Framework"); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
	glewInit();
#endif
    glPrintContextInfo();
	My_Init();

	// Create a menu and bind it to mouse right button.
	int menu_main = glutCreateMenu(My_Menu);
	int menu_timer = glutCreateMenu(My_Menu);
	int menu_effect = glutCreateMenu(My_Menu);
	int menu_navigation = glutCreateMenu(My_Menu);

	glutSetMenu(menu_main);
	glutAddSubMenu("Timer", menu_timer);
	glutAddSubMenu("Effect", menu_effect);
	glutAddSubMenu("Navigation", menu_navigation);
	glutAddMenuEntry("Exit", MENU_EXIT);

	glutSetMenu(menu_timer);
	glutAddMenuEntry("Start", MENU_TIMER_START);
	glutAddMenuEntry("Stop", MENU_TIMER_STOP);

	glutSetMenu(menu_effect);
	glutAddMenuEntry("Light Effect", MENU_LIGHTEFFECT);
	glutAddMenuEntry("Fog Effect", MENU_FOGEFFECT);
	glutAddMenuEntry("Normal Mapping Effect", MENU_NORMALMAP);
	glutAddMenuEntry("Shadow Mapping Effect", MENU_SHADOWEFFECT);

	glutSetMenu(menu_navigation);
	glutAddMenuEntry("On/Off", MENU_NAVIGATION);
	glutAddMenuEntry("Start/Pause", MENU_NAVIGATIONRUN);

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
	glutKeyboardUpFunc(My_Keyboard_Up);
	glutSpecialFunc(My_SpecialKeys);
	glutTimerFunc(timer_speed, My_Timer, 0); 

	// Enter main event loop.
	glutMainLoop();

	delete skybox;

	return 0;
}
