#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//C++ standard libraries
#include <iostream>
#include <fstream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include "SOIL2/SOIL2.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Texture.h"
#include "Animation.h"
#include "Animator.h"

// Function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();
void AnimationKeys();

// Window dimensions
const GLuint WIDTH = 1920, HEIGHT = 1000;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera  camera(glm::vec3(1.5f, 0.5f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), 270.0f);
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

//Luces
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
bool active;
bool luzNaranja = false;   // false = blanco, true = naranja (tecla L)

// Spotlights sobre Kaya (teclas R, V, B)
bool luzRoja = true, luzVerde = true, luzAzul = true;

// Positions of the point lights
glm::vec3 pointLightPositions[] = {
	glm::vec3(0.0f,2.0f, 0.0f)
};

#define MAX_FRAMES 20
typedef struct _frame {
	float rotGato;
	float rotGatoInc;
	float gatoPosX;
	float gatoPosY;
	float gatoPosZ;
	float incX;
	float incY;
	float incZ;
	float headGato;
	float headIncGato;
	float FLegsGato;
	float FLegsIncGato;
	float BLegsGato;
	float BLegsIncGato;
	float tailGato;
	float tailIncGato;
	float inclinacionGato;
	float inclinacionIncGato;
} FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 0;
bool play = false;
int playIndex = 0;

float vertices[] = {
	 -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

// === Animacion ciclo dia-noche ===
float sunAngle = 0.0f;
float sunSpeed = 0.2f;
float sunRadius = 10.0f;
bool dayNightCycle = true;
glm::vec3 sunPosition(0.0f);

// Intensidad luz puntual, se cambia con SPACE
glm::vec3 Light1 = glm::vec3(0);


// === Animaciones FBX (por esqueleto) ===
bool playFBX = false;
Animation* g_SteveAnim = nullptr;
Animator* g_SteveAnimator = nullptr;
float steveSpeed = 1.0f;
int steveEstado = 0;
float steveDir = 1.0f;
float stevePosX = 0.0f, stevePosY = 0.0f, stevePosZ = 0.0f;

// === Zorro ===
bool foxWalking = false;          // tecla Z: caminar (alterna)
bool foxJumping = false;          // tecla X: saltar (una vez)
float foxWalkTime = 0.0f;
float foxJumpTime = 0.0f;
const float foxJumpDur = 0.8f;
float foxZ = 0.0f;                // desplazamiento al caminar (unidades del modelo)
float foxDir = 1.0f;

// === Kaya (huesos, bucle continuo) ===
Animation* g_SoldAnim = nullptr;
Animator* g_SoldAnimator = nullptr;

// === Campesino (huesos, bucle continuo) ===
Animation* g_CampAnim = nullptr;
Animator* g_CampAnimator = nullptr;
// Kaya: la tecla C la pone a caminar (se mueve de su punto) o la pausa
bool kayaActiva = false;
float kayaDesp = 0.0f;            // cuanto se ha alejado de su punto (unidades)
float kayaDir = 1.0f;             // 1 = se aleja, -1 = regresa

int i_max_steps = 140;
int i_curr_steps = 0;


// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame


// Rota una matriz alrededor de un pivote (para articulaciones del zorro)
glm::mat4 RotarEnPivote(glm::mat4 base, glm::vec3 pivote, float grados, glm::vec3 eje)
{
	base = glm::translate(base, pivote);
	base = glm::rotate(base, glm::radians(grados), eje);
	base = glm::translate(base, -pivote);
	return base;
}


int main()
{
	// Inicializacion de GLFW y creacion de ventana
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Extraordinario CG", nullptr, nullptr);

	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Set the required callback functions
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	// Define the viewport dimensions
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// === Carga de shaders ===
	Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
	Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");
	Shader skyboxShader("Shader/SkyBox.vs", "Shader/SkyBox.frag");


	// === Carga de modelos ===

	//Model Aldea
	Model Aldea((char*)"ModelsExtra/Floor.obj");


	Model tree((char*)"ModelsExtra/Tree/Tree.obj");

	//Model Steve
	Model Steve((char*)"ModelsExtra/Steve/steveAnimated.fbx");
	g_SteveAnim = new Animation("ModelsExtra/Steve/steveAnimated.fbx", &Steve);
	g_SteveAnimator = new Animator(g_SteveAnim);
	g_SteveAnimator->Reset();

	// === Kaya (Mixamo) ===
	Model Kaya((char*)"ModelsExtra/Kaya.fbx");
	g_SoldAnim = new Animation("ModelsExtra/Kaya.fbx", &Kaya);
	g_SoldAnimator = new Animator(g_SoldAnim);
	g_SoldAnimator->Reset();

	// === Campesino ===
	Model Campesino((char*)"ModelsExtra/Campesino.fbx");
	g_CampAnim = new Animation("ModelsExtra/Campesino.fbx", &Campesino);
	g_CampAnimator = new Animator(g_CampAnim);
	g_CampAnimator->Reset();

	// === Caballo
	Model Caballo((char*)"ModelsExtra/Caballo.OBJ", false);   // false = color cafe fijo

	// === Carroza ===
	Model Carroza((char*)"ModelsExtra/Carroza/Carroza.obj");

	// === Bandera ===
	Model Bandera((char*)"ModelsExtra/Bandera/scene.gltf");

	// === Modelos estaticos del campamento ===
	Model Cactus((char*)"ModelsExtra/Cactus/BarrelCactus.obj");
	Model Caja((char*)"ModelsExtra/Caja/Caja.fbx");
	Model CajaMuniciones((char*)"ModelsExtra/CajaMuniciones/AssaultRifleAmmo_Box.obj");
	Model Machete((char*)"ModelsExtra/Machete/Machete.fbx");
	Model Rifle((char*)"ModelsExtra/Rifle/Rifle.fbx");
	Model Canon((char*)"ModelsExtra/Canon/Cannon.fbx");
	Model Tienda((char*)"ModelsExtra/Tienda/Tent.obj");
	Model Cactus2((char*)"ModelsExtra/Cactus2/PUSHILIN_prickly_pear_cactus.obj");
	Model Iglesia((char*)"ModelsExtra/Iglesia/Iglesia.obj");
	Model Barril((char*)"ModelsExtra/Barril.fbx");


	// === Zorro ===
	Model zCuerpo((char*)"ModelsExtra/Zorro/cuerpo.obj");
	Model zCabeza((char*)"ModelsExtra/Zorro/cabeza.obj");
	Model zCola((char*)"ModelsExtra/Zorro/cola.obj");
	Model zPataDD((char*)"ModelsExtra/Zorro/pataDerDelante.obj");
	Model zPataDT((char*)"ModelsExtra/Zorro/pataDerDetras.obj");
	Model zPataID((char*)"ModelsExtra/Zorro/pataIzqDelante.obj");
	Model zPataIT((char*)"ModelsExtra/Zorro/pataIzqDetras.obj");


	// Vertices skybox
	GLfloat skyboxVertices[] = {
		// Positions
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	GLuint indices[] =
	{  // Note that we start from 0!
		0,1,2,3,
		4,5,6,7,
		8,9,10,11,
		12,13,14,15,
		16,17,18,19,
		20,21,22,23,
		24,25,26,27,
		28,29,30,31,
		32,33,34,35
	};

	// First, set the container's VAO (and VBO)
	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);


	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Set texture units
	lightingShader.Use();
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.difuse"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.specular"), 1);

	//Skybox
	GLuint skyboxVBO, skyboxVAO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	// Load textures
	vector < const GLchar*> faces;
	faces.push_back("SkyBox/right.jpg");
	faces.push_back("SkyBox/left.jpg");
	faces.push_back("SkyBox/top.jpg");
	faces.push_back("SkyBox/bottom.jpg");
	faces.push_back("SkyBox/back.jpg");
	faces.push_back("SkyBox/front.jpg");

	GLuint cubemapTexture = TextureLoading::LoadCubemap(faces);


	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);

	// === Loop para renderizar la escena ===
	while (!glfwWindowShouldClose(window))
	{

		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		sunPosition = glm::vec3(0.0f, 10.0f, 0.0f);

		// Direccion de la luz hacia abajo
		glm::vec3 sunDir = glm::vec3(0.0f, -1.0f, 0.0f);

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		DoMovement();
		AnimationKeys();


		// Clear the colorbuffer

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// OpenGL options
		glEnable(GL_DEPTH_TEST);


		glm::mat4 modelTemp = glm::mat4(1.0f); //Temp


		// Use corresponding shader when setting uniforms/drawing objects
		lightingShader.Use();


		glUniform1i(glGetUniformLocation(lightingShader.Program, "diffuse"), 0);
		//glUniform1i(glGetUniformLocation(lightingShader.Program, "specular"),1);

		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

		// === Directional light: blanco tenue / naranja, intensidad 0.5 ===
		float intensidad = 0.5f;
		glm::vec3 colorLuz = luzNaranja ? glm::vec3(1.0f, 0.5f, 0.0f)   // naranja
			: glm::vec3(1.0f, 1.0f, 1.0f);  // blanco

		glm::vec3 ambient = colorLuz * intensidad * 0.5f;
		glm::vec3 diffuse = colorLuz * intensidad;
		glm::vec3 specular = colorLuz * intensidad;

		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), sunDir.x, sunDir.y, sunDir.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), ambient.x, ambient.y, ambient.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), diffuse.x, diffuse.y, diffuse.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), specular.x, specular.y, specular.z);


		// === Spotlights R, V, B sobre Kaya ===
		glm::vec3 objetivoSpot(1.0f, 0.35f, 1.0f);                 // Kaya
		glm::vec3 posSpot[3] = { glm::vec3(-0.5f, 1.5f, 2.5f),     // roja
								 glm::vec3(1.0f, 1.7f, 2.7f),      // verde
								 glm::vec3(2.5f, 1.5f, 2.5f) };    // azul
		glm::vec3 colSpot[3] = { glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1) };
		bool encendida[3] = { luzRoja, luzVerde, luzAzul };

		for (int i = 0; i < 3; i++)
		{
			string base = "spotLights[" + to_string(i) + "]";
			glm::vec3 dir = glm::normalize(objetivoSpot - posSpot[i]);
			glm::vec3 col = encendida[i] ? colSpot[i] : glm::vec3(0.0f);   // apagada = color negro
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".position").c_str()), posSpot[i].x, posSpot[i].y, posSpot[i].z);
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".direction").c_str()), dir.x, dir.y, dir.z);
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".ambient").c_str()), 0.0f, 0.0f, 0.0f);
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".diffuse").c_str()), col.x * 2.0f, col.y * 2.0f, col.z * 2.0f);
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + ".specular").c_str()), col.x, col.y, col.z);
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".cutOff").c_str()), glm::cos(glm::radians(12.5f)));
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".outerCutOff").c_str()), glm::cos(glm::radians(17.5f)));
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".constant").c_str()), 1.0f);
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".linear").c_str()), 0.09f);
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + ".quadratic").c_str()), 0.032f);
		}

		// Set material properties
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 5.0f);

		// Create camera transformations
		glm::mat4 view;
		view = camera.GetViewMatrix();

		// Get the uniform locations
		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


		glm::mat4 model(1);
		glUniform1i(glGetUniformLocation(lightingShader.Program, "useBones"), 0);

		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Aldea.Draw(lightingShader);

		// === Arboles (x, y, z): la Y es la altura del piso en ese punto ===
		glm::vec3 posArboles[9] = {
			glm::vec3(0.0f, 0.45f,   8.0f),   // 5 originales, actualizados
			glm::vec3(8.0f, 0.02f,   0.0f),
			glm::vec3(-8.0f, 0.30f,   0.0f),
			glm::vec3(0.0f, 0.05f, -10.0f),
			glm::vec3(3.0f, 0.05f, -15.0f),
			glm::vec3(5.0f, 0.02f, -25.0f),   // 4 nuevos
			glm::vec3(3.0f, 0.04f, -33.0f),
			glm::vec3(9.0f, 0.03f,   8.0f),
			glm::vec3(-9.0f, 0.05f,  -6.0f)
		};
		for (int a = 0; a < 9; a++)
		{
			model = glm::mat4(1);
			model = glm::translate(model, posArboles[a]);
			model = glm::scale(model, glm::vec3(2.0f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			tree.Draw(lightingShader);
		}


		// === Animacion de Steve caminando ===
		model = modelTemp;
		if (playFBX)
		{
			// Maquina estados
			if (steveEstado == 0) // Estado 0: Caminando hacia adelante
			{
				g_SteveAnimator->UpdateAnimation(deltaTime);
				stevePosZ += steveDir * steveSpeed * deltaTime;

				if (stevePosZ >= 2.8f)
				{
					stevePosZ = 2.8f;
					steveDir = -1.0f;
					steveEstado = 1;
				}
			}
			else if (steveEstado == 1) // Estado 1: Regresando al origen
			{
				g_SteveAnimator->UpdateAnimation(deltaTime);
				stevePosZ += steveDir * steveSpeed * deltaTime;

				if (stevePosZ <= 0.0f)
				{
					stevePosZ = 0.0f;
					steveEstado = 2;
					playFBX = false;
				}
			}

			glUniform1i(glGetUniformLocation(lightingShader.Program, "useBones"), 1);
			model = glm::translate(model, glm::vec3(2.0f + stevePosX, 0.1f + stevePosY, 0.0f + stevePosZ));
			if (steveEstado == 1)
				model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.05f));
			const auto& transforms = g_SteveAnimator->GetFinalBoneMatrices();
			for (int i = 0; i < (int)transforms.size(); i++)
			{
				string uniformName = "finalBonesMatrices[" + to_string(i) + "]";
				glUniformMatrix4fv(
					glGetUniformLocation(lightingShader.Program, uniformName.c_str()),
					1, GL_FALSE, glm::value_ptr(transforms[i])
				);
			}
		}
		else
		{
			glUniform1i(glGetUniformLocation(lightingShader.Program, "useBones"), 0);
			model = glm::translate(model, glm::vec3(2.0f + stevePosX, 0.1f + stevePosY, 0.0f + stevePosZ));
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.05f));
		}
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Steve.Draw(lightingShader);

		glUniform1i(glGetUniformLocation(lightingShader.Program, "useBones"), 0);


		// === Kaya (huesos): la tecla C la pone a caminar de ida y vuelta / la pausa ===
		if (kayaActiva)
		{
			g_SoldAnimator->UpdateAnimation(deltaTime);
			kayaDesp += kayaDir * 0.6f * deltaTime;                       // 0.6 = velocidad
			if (kayaDesp >= 3.0f) { kayaDesp = 3.0f; kayaDir = -1.0f; }   // 3.0 = hasta donde llega
			if (kayaDesp <= 0.0f) { kayaDesp = 0.0f; kayaDir = 1.0f; }
		}
		glUniform1i(glGetUniformLocation(lightingShader.Program, "useBones"), 1);
		const auto& soldBones = g_SoldAnimator->GetFinalBoneMatrices();
		for (int i = 0; i < (int)soldBones.size(); i++)
		{
			string uniformName = "finalBonesMatrices[" + to_string(i) + "]";
			glUniformMatrix4fv(
				glGetUniformLocation(lightingShader.Program, uniformName.c_str()),
				1, GL_FALSE, glm::value_ptr(soldBones[i])
			);
		}
		glm::mat4 sold = glm::mat4(1.0f);
		sold = glm::translate(sold, glm::vec3(1.0f, 0.1f, 1.0f - kayaDesp));   // se aleja hacia -Z
		if (kayaDir > 0.0f && kayaDesp > 0.0f)
			sold = glm::rotate(sold, glm::radians(180.0f), glm::vec3(0, 1, 0));    // mira hacia donde camina
		sold = glm::scale(sold, glm::vec3(0.003f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(sold));
		Kaya.Draw(lightingShader);
		glUniform1i(glGetUniformLocation(lightingShader.Program, "useBones"), 0);


		// === Campesino (huesos, bucle continuo) ===
		g_CampAnimator->UpdateAnimation(deltaTime);
		glUniform1i(glGetUniformLocation(lightingShader.Program, "useBones"), 1);
		const auto& campBones = g_CampAnimator->GetFinalBoneMatrices();
		for (int i = 0; i < (int)campBones.size(); i++)
		{
			string uniformName = "finalBonesMatrices[" + to_string(i) + "]";
			glUniformMatrix4fv(
				glGetUniformLocation(lightingShader.Program, uniformName.c_str()),
				1, GL_FALSE, glm::value_ptr(campBones[i])
			);
		}
		glm::mat4 camp = glm::mat4(1.0f);
		// --- Cañon: posicion, giro y "atras" (los usan el campesino y el retroceso) ---
		glm::vec3 posCanon(-5.0f, 0.02f, -6.0f);
		float giroCanon = 0.0f;                          // giro en Y del canon
		// Parte trasera del cañon en el modelo. Si el campesino queda delante de la boca
		glm::vec3 atrasCanon(-0.6f, 0.0f, 0.0f);
		glm::vec3 atrasMundo = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(giroCanon), glm::vec3(0, 1, 0))
			* glm::vec4(atrasCanon, 0.0f));
		float distCampesino = 0.8f;                      // distancia del campesino al centro del canon
		glm::vec3 posCamp = posCanon + atrasMundo * distCampesino;
		float giroCamp = glm::degrees(std::atan2(-atrasMundo.x, -atrasMundo.z));   // mira hacia el canon
		camp = glm::translate(camp, glm::vec3(posCamp.x, 0.1f, posCamp.z));
		camp = glm::rotate(camp, glm::radians(giroCamp), glm::vec3(0, 1, 0));
		camp = glm::scale(camp, glm::vec3(0.003f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(camp));
		Campesino.Draw(lightingShader);
		glUniform1i(glGetUniformLocation(lightingShader.Program, "useBones"), 0);


		// === Caballos + Carroza (animacion procedural: recorren juntos) ===
		glUniform1i(glGetUniformLocation(lightingShader.Program, "useBones"), 0);

		// ---- Parametros para ajustar a ojo ----
		float escCaballo = 0.03f;
		float escCarroza = 0.3f;
		float distCarroza = 1.25f;        // distancia carroza-caballos
		float sepCaballos = 0.2f;         // separacion de cada caballo respecto al centro
		float ajusteCaballoYaw = 0.0f;    // gira solo el modelo del caballo
		float ajusteCarrozaYaw = -90.0f;  // gira solo el modelo de la carroza
		float xIni = 0.0f, xFin = 3.0f;   // tramo del recorrido en X
		float zRec = 5.5f;                // Z del recorrido
		float yBase = 0.3f;               // altura base

		// Ciclo: avanza, da la vuelta, regresa, da la vuelta
		float tc = (float)glfwGetTime();
		float T = 8.0f;                   // segundos de cada tramo
		float Tg = 2.0f;                  // segundos de cada giro
		float L = xFin - xIni;
		float c = fmod(tc, 2.0f * (T + Tg));
		float x, yaw;
		if (c < T) { x = xIni + L * (c / T);                  yaw = 90.0f; }
		else if (c < T + Tg) { x = xFin;                                yaw = 90.0f + 180.0f * ((c - T) / Tg); }
		else if (c < 2 * T + Tg) { x = xFin - L * ((c - T - Tg) / T);       yaw = 270.0f; }
		else { x = xIni;                                yaw = 270.0f + 180.0f * ((c - 2 * T - Tg) / Tg); }

		// Matriz comun del conjunto (la direccion de avance es +Z local)
		glm::mat4 tiro = glm::mat4(1.0f);
		tiro = glm::translate(tiro, glm::vec3(x, yBase, zRec));
		tiro = glm::rotate(tiro, glm::radians(yaw), glm::vec3(0, 1, 0));

		// Dos caballos en paralelo (lado -1 y +1), con el trote desfasado
		for (int lado = -1; lado <= 1; lado += 2)
		{
			float fase = (lado == -1) ? 0.0f : 1.5f;
			glm::mat4 cab = tiro;
			cab = glm::translate(cab, glm::vec3(lado * sepCaballos, 0.03f * fabs(sin(tc * 8.0f + fase)), 0.0f));
			cab = glm::rotate(cab, glm::radians(3.0f * sin(tc * 8.0f + fase)), glm::vec3(1, 0, 0));
			cab = glm::rotate(cab, glm::radians(ajusteCaballoYaw), glm::vec3(0, 1, 0));
			cab = glm::scale(cab, glm::vec3(escCaballo));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cab));
			Caballo.Draw(lightingShader);
		}

		// Carroza (centrada, detras de los caballos)
		glm::mat4 carr = tiro;
		carr = glm::translate(carr, glm::vec3(0.0f, 0.0f, -distCarroza));
		carr = glm::rotate(carr, glm::radians(ajusteCarrozaYaw), glm::vec3(0, 1, 0));
		carr = glm::scale(carr, glm::vec3(escCarroza));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(carr));
		Carroza.Draw(lightingShader);

		// === Bandera ===
		glUniform1i(glGetUniformLocation(lightingShader.Program, "useBones"), 0);
		float escBandera = 0.3f;
		glm::vec3 posBandera(0.0f, -0.09f, -5.0f);
		float giroBandera = 180.0f;
		int   piezaTela = 0;                           // la tela es la pieza 0 (el asta es la 1)

		// ---- Alturas a editar (unidades del modelo, antes de la escala) ----
		// Positivo = sube, negativo = baja
		float alturaTela = -10.0f;                     // cuanto sube o baja la bandera
		float alturaAsta = 0.5f;                       // cuanto sube o baja el asta

		glm::mat4 ban = glm::mat4(1.0f);
		ban = glm::translate(ban, posBandera);
		ban = glm::rotate(ban, glm::radians(giroBandera), glm::vec3(0, 1, 0));
		ban = glm::scale(ban, glm::vec3(escBandera));

		// Asta (todas las piezas menos la tela)
		glm::mat4 asta = glm::translate(ban, glm::vec3(0.0f, alturaAsta, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(asta));
		for (int i = 0; i < Bandera.MeshCount(); i++)
			if (i != piezaTela) Bandera.DrawMesh(i, lightingShader);

		// Tela
		glm::mat4 tela = glm::translate(ban, glm::vec3(0.0f, alturaTela, 0.0f));
		tela = glm::rotate(tela, glm::radians(8.0f * sin(tc * 2.0f)), glm::vec3(0, 1, 0));   // ondeo de la bandera
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(tela));
		Bandera.DrawMesh(piezaTela, lightingShader);

		// === Modelos estaticos del campamento ===
		// Cada fila: { modelo, posicion (x, y, z), tamano, giro en X, giro en Y }
		//   tamano = lado mas grande del objeto en unidades del mundo (el programa lo escala solo)
		//   posicion = centro del objeto; el programa lo apoya sobre la Y indicada
		glUniform1i(glGetUniformLocation(lightingShader.Program, "useBones"), 0);
		struct ItemEstatico { Model* m; glm::vec3 pos; float tam; float rotX; float rotY; };
		ItemEstatico campamento[12] = {
			{ &Cactus,         glm::vec3(13.5f, 0.02f, -9.0f), 0.80f,   0.0f,   0.0f },   // junto a la arena
			{ &Caja,           glm::vec3(5.3f, 0.00f, -2.9f), 0.30f, -90.0f,   0.0f },   // cajas junto a la tienda
			{ &CajaMuniciones, glm::vec3(5.7f, 0.00f, -2.9f), 0.20f,   0.0f,  15.0f },
			{ &Machete,        glm::vec3(5.0f, 0.02f, -2.4f), 0.30f,   0.0f,  20.0f },   // armas al frente
			{ &Rifle,          glm::vec3(5.7f, 0.02f, -2.4f), 0.45f, -90.0f, -10.0f },
			{ &Canon,          posCanon,                       0.55f, -90.0f, giroCanon }, // canon (retrocede)
			{ &Tienda,         glm::vec3(6.0f, 0.00f, -4.0f), 1.20f,   0.0f,   0.0f },
			{ &Cactus2,        glm::vec3(28.0f, 0.02f, -14.0f), 1.00f,   0.0f,   0.0f },   // otra orilla del rio
			{ &Iglesia,        glm::vec3(7.0f, 0.00f, -9.0f), 6.00f,   0.0f,   0.0f },   // junto a la tienda
			{ &Cactus,         glm::vec3(36.0f, 0.02f, -26.0f), 0.75f,  0.0f,   0.0f },   // cactus extra, orilla este
			{ &Cactus2,        glm::vec3(17.5f, 0.02f, -20.5f), 0.90f,  0.0f,   0.0f },   // cactus2 extra, orilla oeste
			{ &Barril,         glm::vec3(7.0f, 0.00f, -3.0f), 0.45f, -90.0f,   0.0f }    // junto a la tienda
		};
		// Retroceso del canon: cada 4 s sale hacia atras rapido y regresa despacio
		float cicloCanon = fmod((float)glfwGetTime(), 4.0f);
		float retroCanon = (cicloCanon < 0.15f) ? cicloCanon / 0.15f
			: (cicloCanon < 1.0f) ? 1.0f - (cicloCanon - 0.15f) / 0.85f : 0.0f;
		float distRetroCanon = 0.12f;   // cuanto retrocede (unidades del mundo)
		for (int k = 0; k < 12; k++)
		{
			Model* mp = campamento[k].m;
			glm::vec3 centro = (mp->bbMin + mp->bbMax) * 0.5f;
			glm::vec3 ext = mp->bbMax - mp->bbMin;
			float mayor = glm::max(ext.x, glm::max(ext.y, ext.z));
			float esc = (mayor > 0.0001f) ? campamento[k].tam / mayor : 1.0f;

			glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(campamento[k].rotY), glm::vec3(0, 1, 0));
			rot = glm::rotate(rot, glm::radians(campamento[k].rotX), glm::vec3(1, 0, 0));

			// punto mas bajo del objeto ya girado, para apoyarlo en el suelo
			float minY = 1e9f;
			for (int c = 0; c < 8; c++)
			{
				glm::vec3 esquina((c & 1) ? mp->bbMax.x : mp->bbMin.x,
					(c & 2) ? mp->bbMax.y : mp->bbMin.y,
					(c & 4) ? mp->bbMax.z : mp->bbMin.z);
				glm::vec4 p = rot * glm::vec4((esquina - centro) * esc, 1.0f);
				if (p.y < minY) minY = p.y;
			}

			glm::vec3 desp(0.0f);
			if (mp == &Canon)   // retroceso hacia la parte trasera del canon
				desp = atrasMundo * (retroCanon * distRetroCanon);
			model = glm::translate(glm::mat4(1.0f), campamento[k].pos + desp + glm::vec3(0.0f, -minY, 0.0f));
			model = model * rot;
			model = glm::scale(model, glm::vec3(esc));
			model = glm::translate(model, -centro);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			mp->Draw(lightingShader);
		}

		// === Actualizacion de la animacion del zorro ===
		float legSwing = 0.0f, bodyUp = 0.0f, bodyPitch = 0.0f, tailSwing = 0.0f, headBob = 0.0f;

		if (foxWalking) {
			foxWalkTime += deltaTime;
			legSwing = 30.0f * sin(foxWalkTime * 6.0f);
			tailSwing = 15.0f * sin(foxWalkTime * 3.0f);
			headBob = 5.0f * sin(foxWalkTime * 6.0f);
			foxZ += foxDir * 5.0f * deltaTime;
			if (foxZ > 15.0f) foxDir = -1.0f;
			if (foxZ < 0.0f)  foxDir = 1.0f;
		}
		if (foxJumping) {
			foxJumpTime += deltaTime;
			float t = foxJumpTime / foxJumpDur;
			if (t >= 1.0f) foxJumping = false;
			else {
				bodyUp = 3.0f * sin(t * 3.14159f);
				bodyPitch = -20.0f * sin(t * 3.14159f);
				legSwing = -35.0f * sin(t * 3.14159f);
			}
		}

		// === Dibujo del zorro ===
		glm::mat4 foxBase = glm::mat4(1.0f);
		foxBase = glm::translate(foxBase, glm::vec3(3.0f, 0.125f, 3.0f));
		foxBase = glm::scale(foxBase, glm::vec3(0.1f));
		foxBase = glm::translate(foxBase, glm::vec3(0.0f, bodyUp, foxZ));
		foxBase = RotarEnPivote(foxBase, glm::vec3(0.0f, 0.0f, -1.2f), bodyPitch, glm::vec3(1, 0, 0));
		glm::mat4 m;
		glm::vec3 ejeX(1, 0, 0);

		// Cuerpo
		m = foxBase;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
		zCuerpo.Draw(lightingShader);

		// Cabeza
		m = RotarEnPivote(foxBase, glm::vec3(0.0f, 0.0f, -0.68f), headBob, ejeX);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
		zCabeza.Draw(lightingShader);

		// Cola
		m = RotarEnPivote(foxBase, glm::vec3(0.0f, 0.0f, -1.79f), tailSwing, glm::vec3(0, 1, 0));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
		zCola.Draw(lightingShader);

		// Patas (diagonales en oposicion)
		m = RotarEnPivote(foxBase, glm::vec3(-0.254f, -0.493f, -0.918f), legSwing, ejeX);   // delantera derecha
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
		zPataDD.Draw(lightingShader);

		m = RotarEnPivote(foxBase, glm::vec3(0.267f, -0.493f, -0.918f), -legSwing, ejeX);   // delantera izquierda
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
		zPataID.Draw(lightingShader);

		m = RotarEnPivote(foxBase, glm::vec3(-0.275f, -0.493f, -1.518f), -legSwing, ejeX);  // trasera derecha
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
		zPataDT.Draw(lightingShader);

		m = RotarEnPivote(foxBase, glm::vec3(0.267f, -0.493f, -1.518f), legSwing, ejeX);    // trasera izquierda
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
		zPataIT.Draw(lightingShader);


		// Also draw the lamp object, again binding the appropriate shader
		lampShader.Use();
		// Get location objects for the matrices on the lamp shader (these could be different on a different shader)
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");

		// Set matrices
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(0);

		// === Dibujo del skybox ===
		glDepthFunc(GL_LEQUAL);
		skyboxShader.Use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);
	delete g_SteveAnim;
	delete g_SteveAnimator;
	delete g_SoldAnim;
	delete g_SoldAnimator;
	delete g_CampAnim;
	delete g_CampAnimator;
	glfwTerminate();
	return 0;
}


void DoMovement()
{
	// Camera controls
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP]) camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN]) camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]) camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT, deltaTime);

}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// L: alterna la luz direccional entre blanco y naranja
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		luzNaranja = !luzNaranja;
	}

	// R, V, B: encienden o apagan las spotlights roja, verde y azul
	if (key == GLFW_KEY_R && action == GLFW_PRESS) luzRoja = !luzRoja;
	if (key == GLFW_KEY_V && action == GLFW_PRESS) luzVerde = !luzVerde;
	if (key == GLFW_KEY_B && action == GLFW_PRESS) luzAzul = !luzAzul;

	// C: Kaya camina (se mueve de su punto) / se pausa
	if (key == GLFW_KEY_C && action == GLFW_PRESS) kayaActiva = !kayaActiva;

	// Z: caminar (alterna), X: saltar
	if (key == GLFW_KEY_Z && action == GLFW_PRESS) foxWalking = !foxWalking;
	if (key == GLFW_KEY_X && action == GLFW_PRESS && !foxJumping) { foxJumping = true; foxJumpTime = 0.0f; }

	// N: animacion de Steve (antes era B)
	if (key == GLFW_KEY_N && action == GLFW_PRESS)
	{
		stevePosZ = 0.0f;
		steveEstado = 0;
		steveDir = 1.0f;

		g_SteveAnimator->PlayAnimation(g_SteveAnim);
		playFBX = true;

		printf("Animacion de Steve iniciada\n");

	}
	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

// Function to handle keyframe animation logic
void AnimationKeys() {

}

// Is called whenever the mouse moves
void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}


void interpolation(void)
{

	KeyFrame[playIndex].incX = (KeyFrame[playIndex + 1].gatoPosX - KeyFrame[playIndex].gatoPosX) / i_max_steps;
	KeyFrame[playIndex].incY = (KeyFrame[playIndex + 1].gatoPosY - KeyFrame[playIndex].gatoPosY) / i_max_steps;
	KeyFrame[playIndex].incZ = (KeyFrame[playIndex + 1].gatoPosZ - KeyFrame[playIndex].gatoPosZ) / i_max_steps;
	KeyFrame[playIndex].headIncGato = (KeyFrame[playIndex + 1].headGato - KeyFrame[playIndex].headGato) / i_max_steps;
	KeyFrame[playIndex].FLegsIncGato = (KeyFrame[playIndex + 1].FLegsGato - KeyFrame[playIndex].FLegsGato) / i_max_steps;
	KeyFrame[playIndex].BLegsIncGato = (KeyFrame[playIndex + 1].BLegsGato - KeyFrame[playIndex].BLegsGato) / i_max_steps;
	KeyFrame[playIndex].tailIncGato = (KeyFrame[playIndex + 1].tailGato - KeyFrame[playIndex].tailGato) / i_max_steps;

	KeyFrame[playIndex].rotGatoInc = (KeyFrame[playIndex + 1].rotGato - KeyFrame[playIndex].rotGato) / i_max_steps;
	KeyFrame[playIndex].inclinacionIncGato = (KeyFrame[playIndex + 1].inclinacionGato - KeyFrame[playIndex].inclinacionGato) / i_max_steps;
}