#include <iostream>
#include "GL/glew.h"
#include "GL/3dgl.h"
#include "GL/glut.h"
#include "GL/freeglut_ext.h"

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

# define PI           3.14159265358979323846  /* pi */
#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

C3dglTerrain water;


// Screen Size
GLuint ScreenWidth = 800;
GLuint ScreenHeight = 600;



// 3D models
C3dglModel table;
C3dglModel vase;
C3dglModel chicken;
C3dglModel lamp;
C3dglModel room;
C3dglModel mirror;
C3dglModel ceilingLamp;
unsigned nPyramidBuf = 0;

// texture ids
GLuint idTexWood;
GLuint idTexCloth;
GLuint idTexNone;
GLuint idTexCube;
GLuint idTexShadowMap;
GLuint idTexWater;
GLuint idFBO;


// GLSL Program
C3dglProgram Program;
C3dglProgram ProgramWater;
// camera position (for first person type camera navigation)
mat4 matrixView;			// The View Matrix
float angleTilt = 15.f;		// Tilt Angle
float angleRot = 0.1f;		// Camera orbiting angle
vec3 cam(0);				// Camera movement values

// light switches
int nAmbient = 1, nDir = 1, nPoint1 = 1, nPoint2 = 1, nPoint3 = 1;

// mirror angle
float angleFrame = 45, angleMirror = -10, deltaFrame = 0, deltaMirror = 0;


bool init()
{
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	// Initialise Shaders
	C3dglShader VertexShader;
	C3dglShader FragmentShader;

	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/basic.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/basic.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!Program.Create()) return false;
	if (!Program.Attach(VertexShader)) return false;
	if (!Program.Attach(FragmentShader)) return false;
	if (!Program.Link()) return false;
	if (!Program.Use(true)) return false;


	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/water.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/water.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramWater.Create()) return false;
	if (!ProgramWater.Attach(VertexShader)) return false;
	if (!ProgramWater.Attach(FragmentShader)) return false;
	if (!ProgramWater.Link()) return false;
	if (!ProgramWater.Use(true)) return false;

	// glut additional setup
	glutSetVertexAttribCoord3(Program.GetAttribLocation("aVertex"));
	glutSetVertexAttribNormal(Program.GetAttribLocation("aNormal"));

	// load your 3D models here!
	if (!table.load("models\\table.obj")) return false;
	if (!vase.load("models\\vase.obj")) return false;
	if (!chicken.load("models\\chicken.obj")) return false;
	if (!lamp.load("models\\lamp.obj")) return false;
	if (!ceilingLamp.load("models\\ceilingLamp.3ds")) return false;
	if (!room.load("models\\LivingRoomObj\\LivingRoom.obj")) return false;
	room.loadMaterials("models\\LivingRoomObj\\");
	if (!water.loadHeightmap("models\\watermap.png", 10)) return false;
	if (!mirror.load("models\\mirror.obj")) return false;
	mirror.loadMaterials("models\\");

	// create pyramid
	float vermals[] = {
	  -4, 0,-4, 0, 4,-7, 4, 0,-4, 0, 4,-7, 0, 7, 0, 0, 4,-7,
	  -4, 0, 4, 0, 4, 7, 4, 0, 4, 0, 4, 7, 0, 7, 0, 0, 4, 7,
	  -4, 0,-4,-7, 4, 0,-4, 0, 4,-7, 4, 0, 0, 7, 0,-7, 4, 0,
	   4, 0,-4, 7, 4, 0, 4, 0, 4, 7, 4, 0, 0, 7, 0, 7, 4, 0,
	  -4, 0,-4, 0,-1, 0,-4, 0, 4, 0,-1, 0, 4, 0,-4, 0,-1, 0,
	   4, 0, 4, 0,-1, 0,-4, 0, 4, 0,-1, 0, 4, 0,-4, 0,-1, 0 };

	// Generate 1 buffer name
	glGenBuffers(1, &nPyramidBuf);
	// Bind (activate) the buffer
	glBindBuffer(GL_ARRAY_BUFFER, nPyramidBuf);
	// Send data to the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vermals), vermals, GL_STATIC_DRAW);

	// Setup Lights
	Program.SendUniform("lightAmbient.on", nAmbient);
	Program.SendUniform("lightAmbient.color", 0.025, 0.025, 0.025);
	Program.SendUniform("lightEmissive.on", 0);
	Program.SendUniform("lightEmissive.color", 1.0, 1.0, 1.0);
	Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
	Program.SendUniform("lightDir.on", nDir);
	Program.SendUniform("lightDir.direction", 1.0, 0.5, 1.0);
	Program.SendUniform("lightDir.diffuse", 0.3, 0.3, 0.3);	  // dimmed white light
	Program.SendUniform("lightPoint1.on", nPoint1);
	Program.SendUniform("lightPoint1.position", -2.95, 4.24, -1.0);
	Program.SendUniform("lightPoint1.diffuse", 0.5, 0.5, 0.5);
	Program.SendUniform("lightPoint1.specular", 1.0, 1.0, 1.0);
	Program.SendUniform("lightPoint2.on", nPoint2);
	Program.SendUniform("lightPoint2.position", 1.05, 4.24, 1.0);
	Program.SendUniform("lightPoint2.diffuse", 0.5, 0.5, 0.5);
	Program.SendUniform("lightPoint2.specular", 1.0, 1.0, 1.0);
	Program.SendUniform("lightPoint3.on", nPoint3);
	Program.SendUniform("lightPoint3.position", 0.0f, 7.0f, 0.0f);
	Program.SendUniform("lightPoint3.diffuse", 0.5, 0.5, 0.5);
	Program.SendUniform("lightPoint3.specular", 1.0, 1.0, 1.0);
	Program.SendUniform("materialSpecular", 0.0, 0.0, 0.0);
	Program.SendUniform("shininess", 3.0);

	// load Cube Map
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &idTexCube);
	glBindTexture(GL_TEXTURE_CUBE_MAP, idTexCube);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


	// create & load textures
	C3dglBitmap bm;

	glActiveTexture(GL_TEXTURE0);

	// cloth texture
	bm.Load("models/cloth.png", GL_RGBA);
	if (!bm.GetBits()) return false;
	glGenTextures(1, &idTexCloth);
	glBindTexture(GL_TEXTURE_2D, idTexCloth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// wood texture
	bm.Load("models/oak.png", GL_RGBA);
	if (!bm.GetBits()) return false;
	glGenTextures(1, &idTexWood);
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// none (simple-white) texture
	glGenTextures(1, &idTexNone);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	BYTE bytes[] = { 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);

	// Water texture
	bm.Load("models/water.png", GL_RGBA);
	glGenTextures(1, &idTexWater);
	glBindTexture(GL_TEXTURE_2D, idTexWater);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	Program.SendUniform("texture0", 0);
	ProgramWater.SendUniform("texture0", 0);

	// setup the water colours and level
	ProgramWater.SendUniform("waterColor", 0.2f, 0.6f, 1.f);
	ProgramWater.SendUniform("skyColor", 0.2f, 0.6f, 1.f);

	// Create shadow map texture
	glActiveTexture(GL_TEXTURE7);
	glGenTextures(1, &idTexShadowMap);
	glBindTexture(GL_TEXTURE_2D, idTexShadowMap);


	// Texture parameters - to get nice filtering & avoid artefact on the edges of the shadowmap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

	// This will associate the texture with the depth component in the Z-buffer
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, ScreenWidth * 2, ScreenHeight * 2, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	// Send the texture info to the shaders
	Program.SendUniform("shadowMap", 7);

	// revert to texture unit 0
	glActiveTexture(GL_TEXTURE0);

	// Create a framebuffer object (FBO)
	glGenFramebuffers(1, &idFBO);
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, idFBO);

	// Instruct openGL that we won't bind a color texture with the currently binded FBO
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// attach the texture to FBO depth attachment point
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, idTexShadowMap, 0);
	// switch back to window-system-provided framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	// Send the texture info to the shaders
	Program.SendUniform("textureCubeMap", 1);
	Program.SendUniform("shadowMap", 7);

	// switch on: transparency/blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));
	matrixView *= lookAt(
		vec3(0.0, 5.0, 6.0),
		vec3(0.0, 5.0, 0.0),
		vec3(0.0, 1.0, 0.0));


	// setup the screen background colour
	glClearColor(0.18f, 0.25f, 0.22f, 1.0f);   // deep grey background

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Shift+AD or arrow key to auto-orbit" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;
	cout << "  1 to switch the lamp #1 on/off" << endl;
	cout << "  2 to switch the lamp #2 on/off" << endl;
	cout << "  9 to switch directional light on/off" << endl;
	cout << "  0 to switch ambient light on/off" << endl;
	cout << endl;

	return true;
}

void done()
{
}

void renderVase(mat4 matrixView)
{
	// setup the View Matrix (camera)
	mat4 m = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));// switch tilt off
	m = translate(m, cam);												// animate camera motion (controlled by WASD keys)
	m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));			// switch tilt on
	m = m * matrixView;
	m = rotate(m, radians(angleRot), vec3(0.f, 1.f, 0.f));				// animate camera orbiting
	matrixView = m;

	glActiveTexture(GL_TEXTURE1);
	Program.SendUniform("reflectionPower", 1.0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, idTexCube);

	// vase
	Program.SendUniform("materialDiffuse", 0.2, 0.4, 0.8);
	Program.SendUniform("materialAmbient", 0.2, 0.4, 0.8);
	Program.SendUniform("materialSpecular", 1.0, 1.0, 1.0);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	m = matrixView;
	m = translate(m, vec3(0.f, 3.f, 0.f));
	m = scale(m, vec3(0.12f, 0.12f, 0.12f));
	vase.render(m);
}

void renderObjects(mat4 matrixView, float theta, bool vaseRender = true, bool mirrorRender = false)
{

	mat4 m;

	if (!mirrorRender)
	{
		// setup the View Matrix (camera)
		mat4 m = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));// switch tilt off
		m = translate(m, cam);												// animate camera motion (controlled by WASD keys)
		m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));			// switch tilt on
		m = m * matrixView;
		m = rotate(m, radians(angleRot), vec3(0.f, 1.f, 0.f));				// animate camera orbiting
		matrixView = m;
		// setup View Matrix
		Program.SendUniform("matrixView", matrixView);
	}
	
	// setup materials
	Program.SendUniform("materialSpecular", 0.0, 0.0, 0.0);

	glActiveTexture(GL_TEXTURE0);
	// room
	Program.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);
	Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);

	m = matrixView;
	m = scale(m, vec3(0.03f, 0.03f, 0.03f));
	room.render(m);

	// table & chairs
	Program.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);
	Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	m = matrixView;
	m = scale(m, vec3(0.004f, 0.004f, 0.004f));
	table.render(1, m);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexCloth);

	table.render(0, m);
	m = rotate(m, radians(180.f), vec3(0.f, 1.f, 0.f));
	table.render(0, m);
	m = translate(m, vec3(250, 0, 0));
	m = rotate(m, radians(90.f), vec3(0.f, 1.f, 0.f));
	table.render(0, m);
	m = translate(m, vec3(0, 0, -500));
	m = rotate(m, radians(180.f), vec3(0.f, 1.f, 0.f));
	table.render(0, m);
	
	// teapot
	Program.SendUniform("materialDiffuse", 0.1, 0.8, 0.3);
	Program.SendUniform("materialAmbient", 0.1, 0.8, 0.3);
	Program.SendUniform("materialSpecular", 1.0, 1.0, 1.0);
	m = matrixView;
	m = translate(m, vec3(1.8f, 3.4f, 0.0f));
	Program.SendUniform("matrixModelView", m);
	glutSolidTeapot(0.5);

	// pyramid
	Program.SendUniform("materialDiffuse", 1.0, 0.2, 0.2);
	Program.SendUniform("materialAmbient", 1.0, 0.2, 0.2);
	Program.SendUniform("materialSpecular", 0.0, 0.0, 0.0);
	m = matrixView;
	m = translate(m, vec3(-1.5f, 3.7f, 0.5f));
	m = rotate(m, radians(180.f), vec3(1, 0, 0));
	m = rotate(m, radians(-4 * theta), vec3(0, 1, 0));
	m = scale(m, vec3(0.1f, 0.1f, 0.1f));
	Program.SendUniform("matrixModelView", m);

	GLuint attribVertex = Program.GetAttribLocation("aVertex");
	GLuint attribNormal = Program.GetAttribLocation("aNormal");
	glBindBuffer(GL_ARRAY_BUFFER, nPyramidBuf);
	glEnableVertexAttribArray(attribVertex);
	glEnableVertexAttribArray(attribNormal);
	glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glVertexAttribPointer(attribNormal, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glDrawArrays(GL_TRIANGLES, 0, 18);
	glDisableVertexAttribArray(GL_VERTEX_ARRAY);
	glDisableVertexAttribArray(GL_NORMAL_ARRAY);

	// chicken
	Program.SendUniform("materialDiffuse", 0.8, 0.8, 0.2);
	Program.SendUniform("materialAmbient", 0.8, 0.8, 0.2);
	Program.SendUniform("materialSpecular", 0.6, 0.6, 1.0);
	m = translate(m, vec3(0, -5, 0));
	m = scale(m, vec3(0.2f, 0.2f, 0.2f));
	m = rotate(m, radians(180.f), vec3(1, 0, 0));
	chicken.render(m);

	// lamp 1
	m = matrixView;
	m = translate(m, vec3(-2.2f, 3.075f, -1.0f));
	m = scale(m, vec3(0.02f, 0.02f, 0.02f));
	lamp.render(m);

	// lamp 2
	m = matrixView;
	m = translate(m, vec3(1.8f, 3.075f, 1.0f));
	m = scale(m, vec3(0.02f, 0.02f, 0.02f));
	lamp.render(m);

	// light bulb 1
	Program.SendUniform("materialDiffuse", 0.8, 0.8, 0.8);
	Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
	Program.SendUniform("lightEmissive.on", nPoint1);
	m = matrixView;
	m = translate(m, vec3(-2.95f, 4.24f, -1.0f));
	m = scale(m, vec3(0.1f, 0.1f, 0.1f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);

	// light bulb 2
	Program.SendUniform("materialDiffuse", 0.8, 0.8, 0.8);
	Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
	Program.SendUniform("lightEmissive.on", nPoint2);
	m = matrixView;
	m = translate(m, vec3(1.05f, 4.24f, 1.0f));
	m = scale(m, vec3(0.1f, 0.1f, 0.1f));
	Program.SendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);
	Program.SendUniform("lightEmissive.on", 0);

	if (vaseRender) {

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, idTexCube);
		Program.SendUniform("reflectionPower", 1.0);

		// vase
		Program.SendUniform("materialDiffuse", 0.2, 0.4, 0.8);
		Program.SendUniform("materialAmbient", 0.2, 0.4, 0.8);
		Program.SendUniform("materialSpecular", 1.0, 1.0, 1.0);
		glBindTexture(GL_TEXTURE_2D, idTexNone);
		m = matrixView;
		m = translate(m, vec3(0.f, 3.f, 0.f));
		m = scale(m, vec3(0.12f, 0.12f, 0.12f));
		vase.render(m);
		Program.SendUniform("reflectionPower", 0.0);
	}
	glActiveTexture(GL_TEXTURE0);
	// Mirror - an example of matrix calculation using GLM

	// Mirror - the Base
	mat4 matMirror = matrixView;
	matMirror = translate(matMirror, vec3(-6.4f, 0.2f, -2.5f));
	matMirror = rotate(matMirror, radians(-90.f - angleFrame), vec3(0, 1, 0));
	matMirror = rotate(matMirror, radians(-90.f), vec3(1, 0, 0));
	matMirror = scale(matMirror, vec3(0.004f, 0.004f, 0.004f));
	for (int i = 0; i <= 7; i++)
		mirror.render(i, matMirror);

	// Mirror - the Swivel
	matMirror = translate(matMirror, vec3(0, 0, 3.67f / 0.004f));
	matMirror = rotate(matMirror, radians(angleMirror - 15), vec3(1, 0, 0));
	matMirror = translate(matMirror, vec3(0, 0, -3.67f / 0.004f));
	for (int i = 8; i <= 12; i++)
		if (i != 11)
			mirror.render(i, matMirror);

	

}

mat4 renderLamp(mat4 matrixView, bool bRender = true)
{
	mat4 m;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexNone);

	// time
	static float t = -1;
	if (t < 0) t = (float)glutGet(GLUT_ELAPSED_TIME);
	float dt = glutGet(GLUT_ELAPSED_TIME) - t;
	t += dt;

	// calculation of the harmonic motion of the swinging lamp
	const float k = 0.000002f;	// Hooke's coefficient
	static float theta = 0.0f;	// angular position
	static float omega = 0.015f;	// angular velocity
	float alpha = -k * theta;	// angular acceleration
	omega += alpha * dt;
	theta += omega * dt;

	// additional rotation
	static float beta = 0;
	beta += dt / 500.f;

	// render the lamp
	Program.SendUniform("materialDiffuse", 0.8, 0.8, 0.8);
	Program.SendUniform("materialAmbient", 0.2, 0.2, 0.2);
	m = matrixView;
	m = translate(m, vec3(0.f, 10.15f, 0.f));
	m = rotate(m, radians(beta), vec3(0.f, 1.f, 0.f));
	m = rotate(m, radians(theta), vec3(0.f, 0.f, 1.f));
	m = translate(m, vec3(0.f, -10.15f, 0.f));

	if (bRender)
	{
		mat4 m1 = m;
		m1 = translate(m1, vec3(0, 10.15f, 0));
		m1 = scale(m1, vec3(0.03f, 0.03f, 0.03f));
		ceilingLamp.render(m1);
	}


	// render the light bulb
	Program.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);
	Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);

	if (bRender) Program.SendUniform("lightEmissive.on", nPoint3);

	m = translate(m, vec3(0.0f, 7.1f, 0.0f));
	m = scale(m, vec3(0.1f, 0.1f, 0.1f));

	if (bRender)
	{
		Program.SendUniform("matrixModelView", m);
		glutSolidSphere(1, 32, 32);
	}

	if (bRender) Program.SendUniform("lightEmissive.on", 0);

	return m;
}

void renderMirror()
{
	// Draw only the mirror surface
	mat4 matMirror = matrixView;
	matMirror = translate(matMirror, vec3(-6.4f, 0.2f, -2.5f));
	matMirror = rotate(matMirror, radians(-90.f - angleFrame), vec3(0, 1, 0));
	matMirror = rotate(matMirror, radians(-90.f), vec3(1, 0, 0));
	matMirror = scale(matMirror, vec3(0.004f, 0.004f, 0.004f));
	matMirror = translate(matMirror, vec3(0, 0, 3.67f / 0.004f));
	matMirror = rotate(matMirror, radians(angleMirror - 15), vec3(1, 0, 0));
	matMirror = translate(matMirror, vec3(0, 0, -3.67f / 0.004f));
	mirror.render(11, matMirror);
}

// called before window opened or resized - to setup the Projection Matrix
void reshape(int w, int h)
{
	ScreenWidth = w;
	ScreenHeight = h;

	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	Program.SendUniform("matrixProjection", perspective(radians(60.f), ratio, 0.02f, 1000.f));
	ProgramWater.SendUniform("matrixProjection", perspective(radians(60.f), ratio, 0.02f, 1000.f));
	// added to allow for changes in the shadow map size
	glBindTexture(GL_TEXTURE_2D, idTexShadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w * 2, h * 2, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
}


void prepareCubeMap(float x, float y, float z, float theta)
{
	// Store the current viewport in a safe place
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	int w = viewport[2];
	int h = viewport[3];

	// setup the viewport to 256x256, 90 degrees FoV (Field of View)
	glViewport(0, 0, 256, 256);
	Program.SendUniform("matrixProjection", perspective(radians(90.f), 1.0f, 0.02f, 1000.0f));

	// render environment 6 times
	mat4 tmp = matrixView;	// store matrixView
	Program.SendUniform("reflectionPower", 0.0);
	for (int i = 0; i < 6; ++i)
	{
		// clear background
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// setup the camera
		const GLfloat ROTATION[6][6] =
		{	// at              up
			{ 1.0, 0.0, 0.0,   0.0, -1.0, 0.0 },  // pos x
			{ -1.0, 0.0, 0.0,  0.0, -1.0, 0.0 },  // neg x
			{ 0.0, 1.0, 0.0,   0.0, 0.0, 1.0 },   // pos y
			{ 0.0, -1.0, 0.0,  0.0, 0.0, -1.0 },  // neg y
			{ 0.0, 0.0, 1.0,   0.0, -1.0, 0.0 },  // poz z
			{ 0.0, 0.0, -1.0,  0.0, -1.0, 0.0 }   // neg z
		};
		matrixView = lookAt(
			vec3(x, y, z),
			vec3(x + ROTATION[i][0], y + ROTATION[i][1], z + ROTATION[i][2]),
			vec3(ROTATION[i][3], ROTATION[i][4], ROTATION[i][5]));

		// send the View Matrix
		Program.SendUniform("matrixView", matrixView);

		// render scene objects - all but the reflective one
		glActiveTexture(GL_TEXTURE0);
		renderObjects(matrixView, theta, false);


		// send the image to the cube texture
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, idTexCube);
		glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, 0, 0, 256, 256, 0);
	}

	// restore the matrixView, viewport and projection
	matrixView = tmp;
	reshape(w, h);
}

void newShadowMap(float theta)
{
	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// ambient material
	Program.SendUniform("materialAmbient", 1.0, 1.0, 1.0);

	// specular material (normally off)
	Program.SendUniform("materialSpecular", 0.0, 0.0, 0.0);
	Program.SendUniform("shininess", 3.0);

	/////////////////////////////////////
	// PHASE 1: CREATING THE SHADOW MAP

	// additional: disable shadows in the shader
	Program.SendUniform("shadowPower", 0.0);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);


	// Step 1: 
	// Set the viewport, view and projection matrices to those that are appropriate for the light POV

	// Setup the View Matrix
	mat4 matLamp = renderLamp(mat4(1), false);		// rertrieve the matrix of the source of light

	mat4 matLampView = lookAt(
		vec3(matLamp[3][0], matLamp[3][1], matLamp[3][2]),
		vec3(0, 0, 0),
		vec3(0.0, 0.0, 1.0));

	// send the new View Matrix to the Shaders
	Program.SendUniform("matrixView", matLampView);

	// setup the Projection Matrix
	mat4 matrixProjection = perspective(radians(120.f), (float)ScreenWidth / (float)ScreenHeight, 0.2f, 10.f);
	Program.SendUniform("matrixProjection", matrixProjection);

	// Setup the Viewport (usually higher than standard resolution to avoid problems with anti-aliasing)
	glViewport(0, 0, ScreenWidth * 2, ScreenHeight * 2);

	// Step 2:
	// Bind the framebuffer containing the shadow map (see next section for framebuffer preparation)

	// Bind the Framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, idFBO);
	// OFF-SCREEN RENDERING FROM NOW!

	// Step 3 and 4:
	// Clear the depth buffer and draw the scene – possibly with a very simple shader program (we just create the shadow map)

	// Clear previous frame values
	glClear(GL_DEPTH_BUFFER_BIT);

	// Disable color rendering, we only want to write to the Z-Buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

phase1render:

	// This is just to retrieve the light coordinates and setup the light for a "swinging lamp"
	matLamp = renderLamp(matLampView, false);
	Program.SendUniform("lightPoint.position", 0.0, 7.1, 0.0);
	Program.SendUniform("lightPoint.transform", matLamp);

	renderObjects(matLampView, theta, true);



	/////////////////////////////////////
	// EXTRA: CREATING THE SHADOW MATRIX
	// Shadow Matrix has to be calculated and sent to the Vertex Shader

	// This is matrix transform every coordinate x,y,z
	// x = x* 0.5 + 0.5 
	// y = y* 0.5 + 0.5 
	// z = z* 0.5 + 0.5 
	// Moving from unit cube [-1,1] to [0,1]  
	const mat4 bias = {
		{ 0.5, 0.0, 0.0, 0.0 },
		{ 0.0, 0.5, 0.0, 0.0 },
		{ 0.0, 0.0, 0.5, 0.0 },
		{ 0.5, 0.5, 0.5, 1.0 }
	};
	Program.SendUniform("matrixShadow", bias * matrixProjection * matLampView);

	////////////////////////////////////////////
	// PHASE 2: RENDERING THE SCENE WITH SHADOWS

phase_2:
	// additional: enable shadows in the shader
	Program.SendUniform("shadowPower", 1.0);

	// Step 0: Disable culling
	glDisable(GL_CULL_FACE);

	// Step 1:
	// Go back to the default framebuffer, undo any other changes is applicable

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// Step 2: Clear the buffers; Set the projection / viewport / view settings 
	// to those that are appropriate for the camera POV.

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// This will quickly restore the Projection and Viewport
	reshape(ScreenWidth, ScreenHeight);

	// Setup the Camera View Matrix
	mat4 m = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));// switch tilt off
	m = translate(m, cam);												// animate camera motion (controlled by WASD keys)
	m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));			// switch tilt on
	m = m * matrixView;
	m = rotate(m, radians(angleRot), vec3(0.f, 1.f, 0.f));				// animate camera orbiting
	matrixView = m;

	// setup View Matrix
	Program.SendUniform("matrixView", matrixView);

	// Step 3:
	// Bind the shadow map texture

	Program.SendUniform("shadowMap", 7);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, idTexShadowMap);

	// Step 4:
	// Draw the scene – with the shadow-enabled shader program

	matLamp = renderLamp(matrixView, false);
	Program.SendUniform("lightPoint.position", 0.0, 7.4, 0.0);
	Program.SendUniform("lightPoint.transform", matLamp);

	renderLamp(matrixView, true);
	renderObjects(matrixView, theta);


rendering_over:
	reshape(ScreenWidth, ScreenHeight);


}

void render()
{
	Program.Use();
	// this global variable controls the animation
	float theta = glutGet(GLUT_ELAPSED_TIME) * 0.01f;

	prepareCubeMap(0.0f, 4.2f, 0.0f, theta);

	newShadowMap(theta);

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// global variables controlling animation of the mirror
	angleMirror += deltaMirror;
	angleFrame += deltaFrame;

	// setup the View Matrix (camera)
	mat4 m = rotate(mat4(1.f), radians(angleTilt), vec3(1.f, 0.f, 0.f));// switch tilt off
	m = translate(m, cam);												// animate camera motion (controlled by WASD keys)
	m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));			// switch tilt on
	m = m * matrixView;
	m = rotate(m, radians(angleRot), vec3(0.f, 1.f, 0.f));				// animate camera orbiting
	matrixView = m;

	// Find the reflection surface (normal and pont)
	float radFrame = radians(angleFrame);
	float radMirror = radians(angleMirror);
	vec3 n(cos(radFrame) * cos(radMirror), sin(radMirror), sin(radFrame) * cos(radMirror));	// normal to the reflection plane
	vec3 p(-6.4f, 3.86f, -2.5f);

	// check which side of the mirror is visible; flip the normal if necessary
	mat4 camView = inverse(matrixView);
	vec3 camPos = vec3(camView[3][0], camView[3][1], camView[3][2]);
	if (dot(p - camPos, n) > 0)
		n = -n;

	// reflection matrix
	float a = n.x, b = n.y, c = n.z, d = -dot(p, n);	// parameters of the reflection plane: Ax + By + Cz + d = 0
	mat4 matrixReflection = mat4(						// reflection matrix
		1 - 2 * a*a, -2 * a*b, -2 * a*c, 0,
		-2 * a*b, 1 - 2 * b*b, -2 * b*c, 0,
		-2 * a*c, -2 * b*c, 1 - 2 * c*c, 0,
		-2 * a*d, -2 * b*d, -2 * c*d, 1);

	// Prepare the Stencil test
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xffffffff);
	glDisable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	// just render the mirror, only with stencil output
	renderMirror();

	// set-up stencil to the render the reflection
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glStencilFunc(GL_EQUAL, 1, 0xffffffff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// setup the view matrix for the reflected image
	matrixView *= matrixReflection;
	Program.SendUniform("matrixView", matrixView);

	// Setup the plane clip
	Program.SendUniform("planeClip", a, b, c, d);
	glEnable(GL_CLIP_PLANE0);

	// render the reflection
	renderObjects(matrixView, theta, true, true);

	// switch off the stencil and the clip plane
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_CLIP_PLANE0);

	// revert to normal view matrix
	matrixView *= matrixReflection;
	Program.SendUniform("matrixView", matrixView);

	// render the image
	// first render the mirror to satisfy the depth test
	// but block the colour output so that the reflection remains on screen
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	renderMirror();
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// render everything else as normally

	renderObjects(matrixView, theta);
	renderLamp(matrixView, true);

	// setup the water texture
	glBindTexture(GL_TEXTURE_2D, idTexWater);

	// render the water
	ProgramWater.Use();
	m = matrixView;
	m = translate(m, vec3(8.1f,2.5f, -11.5f));
	m = scale(m, vec3(0.035f, 1.0f, 0.13f));
	ProgramWater.SendUniform("matrixModelView", m);
	water.render(m);

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
		
}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': if ((glutGetModifiers() & GLUT_ACTIVE_ALT) == 0) cam.z = std::max(cam.z * 1.05f, 0.01f); else deltaMirror = 0.5; break;
	case 's': if ((glutGetModifiers() & GLUT_ACTIVE_ALT) == 0) cam.z = std::min(cam.z * 1.05f, -0.01f); else deltaMirror = -0.5; break;
	case 'a': if ((glutGetModifiers() & GLUT_ACTIVE_ALT) == 0) { cam.x = std::max(cam.x * 1.05f, 0.01f); angleRot = 0.1f; } else deltaFrame = 0.5; break;
	case 'd': if ((glutGetModifiers() & GLUT_ACTIVE_ALT) == 0) { cam.x = std::min(cam.x * 1.05f, -0.01f); angleRot = -0.1f; } else deltaFrame = -0.5; break;
	case 'e': cam.y = std::max(cam.y * 1.05f, 0.01f); break;
	case 'q': cam.y = std::min(cam.y * 1.05f, -0.01f); break;
	case '1': nPoint1 = 1 - nPoint1;	Program.SendUniform("lightPoint1.on", nPoint1); return;
	case '2': nPoint2 = 1 - nPoint2;	Program.SendUniform("lightPoint2.on", nPoint2); return;
	case '3': nPoint3 = 1 - nPoint3;	Program.SendUniform("lightPoint3.on", nPoint3); return;
	case '9': nDir = 1 - nDir;			Program.SendUniform("lightDir.on", nDir); return;
	case '0': nAmbient = 1 - nAmbient;	Program.SendUniform("lightAmbient.on", nAmbient); return;
	}
	// speed limit
	cam.x = std::max(-0.15f, std::min(0.15f, cam.x));
	cam.y = std::max(-0.15f, std::min(0.15f, cam.y));
	cam.z = std::max(-0.15f, std::min(0.15f, cam.z));
	// stop orbiting
	if ((glutGetModifiers() & GLUT_ACTIVE_SHIFT) == 0) angleRot = 0;
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': deltaMirror = cam.z = 0; break;
	case 'a':
	case 'd': deltaFrame = cam.x = 0; break;
	case 'q':
	case 'e': cam.y = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
bool bJustClicked = false;
void onMouse(int button, int state, int x, int y)
{
	bJustClicked = (state == GLUT_DOWN);
	glutSetCursor(bJustClicked ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
}

// handle mouse move
void onMotion(int x, int y)
{
	if (bJustClicked)
		bJustClicked = false;
	else
	{
		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// find delta (change to) pan & tilt
		float deltaPan = 0.25f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
		float deltaTilt = 0.25f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

		// View = Tilt * DeltaPan * Tilt^-1 * DeltaTilt * View;
		angleTilt += deltaTilt;
		mat4 m = mat4(1.f);
		m = rotate(m, radians(angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaPan), vec3(0.f, 1.f, 0.f));
		m = rotate(m, radians(-angleTilt), vec3(1.f, 0.f, 0.f));
		m = rotate(m, radians(deltaTilt), vec3(1.f, 0.f, 0.f));
		matrixView = m * matrixView;
	}
}

int main(int argc, char **argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("CI5520 3D Graphics Programming");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "GLEW Error: " << glewGetErrorString(err) << endl;
		return 0;
	}
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;

	// register callbacks
	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);

	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Version: " << glGetString(GL_VERSION) << endl;

	// init light and everything � not a GLUT or callback function!
	if (!init())
	{
		cerr << "Application failed to initialise" << endl;
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	done();

	return 1;
}

