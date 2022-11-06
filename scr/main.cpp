#define _USE_MATH_DEFINES

#include "auxiliar.h"
#include "PLANE.h"

#include <gl/glew.h>
#define SOLVE_FGLUT_WARNING
#include <gl/freeglut.h> 

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <complex>

#include <iostream>

#include<chrono>


//Variables de la ventana
int WIDTH =1024;
int HEIGHT = 1024;

//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////

//Shaders y program
unsigned int vshader; 
unsigned int fshader;
unsigned int program;

unsigned int cshader;
unsigned int program2;

//Atributos (identificadores)
int inPos;
unsigned int ucolortex;
unsigned int ucolcom;

//VAO 
unsigned int vao;

//VBOs que forman parte del objeto 
unsigned int posVBO;

glm::vec4 coeff = glm::vec4(1.f, 0.f, 0.f, 0.f);
glm::vec2 a = glm::vec2(-1.f,0.f);
glm::mat4x2 solutions = glm::mat4x2(0.f);
unsigned int usize;
unsigned int ucoeff;
unsigned int ua;
unsigned int usol;
unsigned int ustart;
unsigned int uend;

glm::ivec2 wgs = glm::ivec2(16);
glm::ivec2 size= glm::ivec2(WIDTH, HEIGHT);
glm::vec2 sstart= glm::vec2(-1., -1.);
glm::vec2 send= glm::vec2(1.,1.);
glm::vec2 startaux(-1., -1.);
glm::vec2 endaux(1., 1.);

unsigned int utrans;
glm::mat4 trans = glm::mat4(1.f);
float angle = 0.f;

//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////

//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);
void mouseMotionFunc(int, int);

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShader(const char *vname, const char *fname);
void initShader2(const char* name);
void initObj();
void initObj2();
void destroy();

//Carga el shader indicado, devuele el ID del shader
//!Por implementar
GLuint loadShader(const char *fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL, 
//y devuelve el identificador de la textura 
unsigned int loadTex(const char *fileName);

glm::mat4x2 rootssolver();

int main(int argc, char** argv)
{
	//std::locale::global(std::locale("spanish"));// acentos ;)

	printf("Input polynimial coefficients (higher degree to lower degree)\n Example: 4 3 2 1 0 -> 4x^4+3x^3+2x^2+x+10:\n");
	scanf("%f %f %f %f %f", &coeff[0], &coeff[1], &coeff[2], &coeff[3], &a[0]);
	solutions=rootssolver();

	initContext(argc, argv);
	initOGL();
	initShader("../shaders/shader.v0.vert", "../shaders/shader.v0.frag");
	initShader2("../shaders/shader.v1.comp");

	initObj();
	initObj2();

	glutMainLoop();		// bucle de eventos

	destroy();

	return 0;
}
	
//////////////////////////////////////////
// Funciones auxiliares 
void initContext(int argc, char** argv){

	// Definimos el contexto
	glutInit(&argc, argv);							// Inicializa Glut
	glutInitContextVersion(4, 3);					// Indicamos la version de OpenGL
	glutInitContextProfile(GLUT_CORE_PROFILE);		// Queremos un contexto sin compabilidad hacia atras

	// Definimos el Frame Buffer y creamos la ventana
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);	// Frame Buffer por defecto
	glutInitWindowSize(WIDTH, HEIGHT);								// Tamaño de la ventana
	glutInitWindowPosition(0, 0);								// Posicion de la ventana
	glutCreateWindow("Fractal Creator");							// Crea la ventana

	// Inicializamos las extensiones de OpenGL
	glewExperimental = GL_TRUE;
	GLenum err = glewInit(); 
	if (GLEW_OK != err) 
	{ 
		std::cout << "Error: " << glewGetErrorString(err) << std::endl; 
		exit(-1); 
	} 

	// Comprobamos la version de OpenGL
	const GLubyte* oglVersion = glGetString(GL_VERSION);  
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

	// Le indicamos a Glut que funciones hay que ejecutar cuando se dan ciertos eventos
	glutReshapeFunc(resizeFunc);		// redimension de la ventana
	glutDisplayFunc(renderFunc);		// renderizado
	glutIdleFunc(idleFunc);				// cuando el procesador esta ocioso
	glutKeyboardFunc(keyboardFunc);		// evento de teclado
	glutMouseFunc(mouseFunc);			// evento de raton
	glutMotionFunc(mouseMotionFunc);	// control de la camara con el raton
}

void initOGL()
{
	glDisable(GL_DEPTH_TEST);					
	glClearColor(0.f, 0.f, 0.f, 1.0f);		

	glPolygonMode(GL_FRONT, GL_FILL);
	glDisable(GL_CULL_FACE);			

}

void destroy()
{
	// LIBERAMOS RECURSOS

	// buffers
	glDeleteBuffers(1, &posVBO);
	glDeleteVertexArrays(1, &vao);

	// shaders y programa 1
	glDetachShader(program, vshader); 
	glDetachShader(program, fshader);
	glDeleteShader(vshader); 
	glDeleteShader(fshader);
	glDeleteProgram(program);

	//shaders y program 2
	glDetachShader(program2, cshader);
	glDeleteProgram(program2);
}

void initShader(const char *vname, const char *fname)
{
	// Cargamos los shaders de vertices y fragmentos
	vshader = loadShader(vname, GL_VERTEX_SHADER); 
	fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	// Enlazamos los shaders en un programa
	program = glCreateProgram();			// crea el programa
	glAttachShader(program, vshader);		// asigna el shader de vertices
	glAttachShader(program, fshader);		// asigna el shader de fragmentos

	glLinkProgram(program);					// enlaza

	// Comprobamos si hay error al enlazar
	int linked; 
	glGetProgramiv(program, GL_LINK_STATUS, &linked);	// comprueba el status de linkado
	if (!linked) 
	{ 
		//Calculamos una cadena de error 
		GLint logLen; 
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
		char *logString = new char[logLen]; 
		glGetProgramInfoLog(program, logLen, NULL, logString); 
		std::cout << "Error: " << logString << std::endl; 
		delete[] logString; 
		exit (-1); 
	}

	// Creamos los idenficadores de los atributos
	inPos = glGetAttribLocation(program, "inPos"); 
	ucolortex = glGetUniformLocation(program, "color");
	utrans= glGetUniformLocation(program, "trans");
}

void initShader2(const char* name) {
	// Cargamos los shaders de vertices y fragmentos
	cshader = loadShader(name, GL_COMPUTE_SHADER);

	//Enlazamos los shaders en un programa
	program2 = glCreateProgram();
	glAttachShader(program2, cshader);
	glLinkProgram(program2);

	// Comprobamos si hay error al enlazar
	int linked;
	glGetProgramiv(program2, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(program2, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetProgramInfoLog(program2, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		exit(-1);
	}
	ucolcom = glGetUniformLocation(program2, "color");
	usize= glGetUniformLocation(program2, "size");
	ucoeff= glGetUniformLocation(program2, "coeff");
	ua= glGetUniformLocation(program2, "a");
	usol= glGetUniformLocation(program2, "solutions");
	ustart= glGetUniformLocation(program2, "start");
	uend= glGetUniformLocation(program2, "end");
}

void initObj()
{
	glGenVertexArrays(1, &vao);			// crea el VAO
	glBindVertexArray(vao);				// activa el VAO

	glGenBuffers(1, &posVBO);
	
	glBindBuffer(GL_ARRAY_BUFFER, posVBO);
	glBufferData(GL_ARRAY_BUFFER, planeNVertex * sizeof(float) * 2, planeVertexPos, GL_STATIC_DRAW);

	// Lo enlazamos a la variable inPos (location = 0)
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
}

void initObj2() {

	glGenTextures(1, &ucolcom);
	glBindTexture(GL_TEXTURE_2D, ucolcom);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindImageTexture(0, ucolcom, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
}

GLuint loadShader(const char *fileName, GLenum type)
{ 
	unsigned int fileLen; 
	char* source = loadStringFromFile(fileName, fileLen);		// carga el codigo del shader

	////////////////////////////////////////////// 
	//Creación y compilación del Shader 
	GLuint shader;												 
	shader = glCreateShader(type);								// crea un ID para el shader
	glShaderSource(shader, 1, 
		(const GLchar **)&source, (const GLint *)&fileLen);		// le asigna el codigo del shader
	glCompileShader(shader);									// compila el shader
	delete[] source;											// libera espacio

	//Comprobamos que se compiló bien 
	GLint compiled; 
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled); 
	if (!compiled) { 
		//Calculamos una cadena de error 
		GLint logLen; 
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen); 
		char *logString = new char[logLen]; 
		glGetShaderInfoLog(shader, logLen, NULL, logString); 
		std::cout << "Error: " << logString << std::endl; 
		delete[] logString; 
		exit (-1); 
	}

	return shader; 
}

unsigned int loadTex(const char *fileName)
{ 
	unsigned char* map; 
	unsigned int w, h; 

	// cargamos la textura en map
	map = loadTexture(fileName, w, h); 

	// comprobacion de que ha cargado
	if (!map) 
	{ 
		std::cout << "Error cargando el fichero: " << fileName << std::endl; 
		exit(-1); 
	}

	// creamos el id de la textura, la activamos y la subimos a la tarjeta grafica
	unsigned int texId; 
	glGenTextures(1, &texId);										// crea
	glBindTexture(GL_TEXTURE_2D, texId);							// activa	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA,		// sube a la grafica
		GL_UNSIGNED_BYTE, (GLvoid*)map);

	// generamos el resto de niveles
	glGenerateMipmap(GL_TEXTURE_2D);

	GLfloat fLargest;
	if (glewIsSupported("GL_EXT_texture_filter_anisotropic")) {
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
	}

	// configuramos el acceso
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // tengo menos texeles
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);				// tengo mas texeles
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);			// me paso de la coordenada X
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);			// me paso de la coordenada Y

	// liberamos memoria
	delete[] map;

	return texId;	// devolvemos el ID de la textura
}

void renderFunc()
{
	glClear(GL_COLOR_BUFFER_BIT);


	glUseProgram(program2);	
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(ucolcom, 0);
	if(usize!=-1)
		glUniform2iv(usize, 1,&size[0]);
	if (ucoeff != -1)
		glUniform4fv(ucoeff, 1, &coeff[0]);
	if (ua != -1)
		glUniform2fv(ua,1, &a[0]);
	if (usol != -1)
		glUniformMatrix4x2fv(usol, 1,false, &solutions[0][0]);
	if (ustart != -1)
		glUniform2fv(ustart, 1, &sstart[0]);
	if (uend != -1)
		glUniform2fv(uend, 1, &send[0]);

	auto start0 = std::chrono::system_clock::now();
	// Se lanzan los compute shaders
	glDispatchCompute((size.x - 1) / wgs.x + 1, (size.y - 1) / wgs.y + 1, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	
	auto end0 = std::chrono::system_clock::now();

	glUseProgram(program);
	if(ucolortex!=-1)
		glUniform1i(ucolortex, 0);
	if (utrans != -1)
		glUniformMatrix4fv(utrans, 1, false, &trans[0][0]);

	// Activamos el VAO
	glBindVertexArray(vao);

	auto start1 = std::chrono::system_clock::now();

	// Se renderiza el sistema de particulas
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	auto end1 = std::chrono::system_clock::now();

	glutSwapBuffers();

	std::cout << "Calculations done" << std::endl;

}

void resizeFunc(int width, int height)
{
	WIDTH = width;
	HEIGHT = height;

	size = glm::vec2(width, height);
	glBindTexture(GL_TEXTURE_2D, ucolcom);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindImageTexture(0, ucolcom, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	// transformamos coordenadas normalizadas a coordenadas de pixeles
	glViewport(0,0,width,height);	
	glutPostRedisplay();
}


void idleFunc()
{
	//glutPostRedisplay();
}

void keyboardFunc(unsigned char key, int x, int y){

	using namespace glm;

	static float c = M_PI/8.f;

	if (key == 'r' || key == 'R') {
		sstart = vec2(-1.f, -1.f);
		send = vec2(1.f, 1.f);
		angle = 0.f;
		trans = mat4(1.f);
		glutPostRedisplay();
	}
	else if (key == 's' || key == 'S') {
		saveimage(WIDTH, HEIGHT);
	}
	else if (key == 'q' || key == 'Q') {
		angle -= c;
		trans = rotate(mat4(1.f), angle, vec3(0.f, 0.f, 1.f));
		glutPostRedisplay();
	}
	else if (key == 'e' || key == 'e') {
		angle += c;
		trans = rotate(mat4(1.f), angle, vec3(0.f, 0.f, 1.f));
		glutPostRedisplay();
	}
}

void mouseFunc(int button, int state, int x, int y){

	using namespace glm;

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		startaux=vec2(x,size.y-y);
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		vec2 diff = (send - sstart) / vec2(size);
		endaux = vec2(x, size.y-y);
		vec2 change = endaux - startaux;
		sstart = sstart - change * diff;
		send = send - change * diff;
	}
	else if (button==3) {
		vec2 diff = (send - sstart) / vec2(size);
		sstart += diff * 10.f;
		send -= diff * 10.f;
		glutPostRedisplay();
	}
	else if(button==4) {
		vec2 diff = (send - sstart) / vec2(size);
		sstart -= diff * 10.f;
		send += diff * 10.f;
		glutPostRedisplay();
	}

}

void mouseMotionFunc(int x, int y)
{
	
}

glm::mat4x2 rootssolver() {

	glm::mat4x2 sol = glm::mat4x2(0.f);

	using namespace std;

	if (coeff.x != 0) {
		double p = (8. * coeff.x * coeff.z - 3. * coeff.y * coeff.y) / (8. * coeff.x * coeff.x);
		double q = ((double)coeff.y * coeff.y * coeff.y - 4. * coeff.x * coeff.y * coeff.z +
			8. * coeff.x * coeff.x * coeff.w) / (8. * coeff.x * coeff.x * coeff.x);
		complex<double> delta0 = (double)coeff.z * coeff.z - 3. * coeff.y * coeff.w + 12. * coeff.x * a.x;
		complex<double> delta1 = 2. * coeff.z * coeff.z * coeff.z - 9. * coeff.y * coeff.z * coeff.w +
			27. * coeff.y * coeff.y * a.x + 27. * coeff.x * coeff.w * coeff.w -
			72. * coeff.x * coeff.z * a.x;
		complex<double> Q = pow(.5*(delta1 + sqrt(delta1 * delta1 -4.* delta0 * delta0 * delta0)),1./3.);
		if (Q.imag() == 0 || Q.real()==0) {
			complex<double>rot(-.5, sqrt(3.) / 2.);
			Q *= rot;
		}

		complex<double> S = .5 * sqrt(-2. / 3. * p + (Q + delta0 / Q) / (3. * coeff.x));

		complex<double> disc0 = .5 * sqrt(-4. * S * S - 2 * p + q / S);
		complex<double> disc1 = .5 * sqrt(-4. * S * S - 2 * p - q / S);

		double aux = -coeff.y / (4. * coeff.x);

		complex<double>x1 = aux - S + disc0;
		complex<double>x2 = aux - S - disc0;
		complex<double>x3 = aux + S + disc1;
		complex<double>x4 = aux + S - disc1;


		sol[0].x = x1.real();
		sol[0].y = x1.imag();

		sol[1].x = x2.real();
		sol[1].y = x2.imag();

		sol[2].x = x3.real();
		sol[2].y = x3.imag();

		sol[3].x = x4.real();
		sol[3].y = x4.imag();
	}
	else if (coeff.y != 0) {
		double delta0 = (double)coeff.z * coeff.z - 3. * coeff.y * coeff.w;
		double delta1 = 2. * coeff.z * coeff.z * coeff.z - 9. * coeff.y * coeff.z * coeff.w +
			27. * coeff.y * coeff.y * a.x;
		complex<double> Q = cbrt(.5 * (delta1 + sqrt(delta1 * delta1 - 4 * delta0 * delta0 * delta0)));
		if (Q == complex<double>(0., 0.)) {
			Q = cbrt(.5 * (delta1 - sqrt(delta1 * delta1 - 4 * delta0 * delta0 * delta0)));
			if (Q == complex<double>(0., 0.)) {
				sol[1].x = -coeff.z / (3. * coeff.y);
				return sol;
			}
		}
		complex<double>rot(-.5, sqrt(3.) / 2.);
		complex<double>Q1 = Q * rot;
		complex<double>Q2 = Q1 * rot;

		double aux = -1. / (3. * coeff.y);
		complex<double>x1 = aux * ((double)coeff.z + Q + delta0 / Q);
		complex<double>x2 = aux * ((double)coeff.z + Q1 + delta0 / Q1);
		complex<double>x3 = aux * ((double)coeff.z + Q2 + delta0 / Q2);

		sol[0].x = x1.real();
		sol[0].y = x1.imag();

		sol[1].x = x2.real();
		sol[1].y = x2.imag();

		sol[2].x = x3.real();
		sol[2].y = x3.imag();
	}
	else if (coeff.z != 0) {

		complex<double>x1 = (-coeff.w + sqrt(coeff.w * coeff.w - 4 * coeff.z * a.x)) / (2 * coeff.z);
		complex<double>x2 = (-coeff.w - sqrt(coeff.w * coeff.w - 4 * coeff.z * a.x)) / (2 * coeff.z);

		sol[0].x = x1.real();
		sol[0].y = x1.imag();

		sol[1].x = x2.real();
		sol[1].y = x2.imag();
	}
	else if(coeff.w!=0){
		sol[0].x = -a.x;
	}
	else {
		printf("No correct polynomial");
		exit;
	}

	

	return sol;
}
