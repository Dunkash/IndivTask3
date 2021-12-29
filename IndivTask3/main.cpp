#include <gl/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

#include "figure_model.h"

// В C и C++ есть оператор #, который позволяет превращать параметры макроса в строку
#define TO_STRING(x) #x

struct Tex
{
	GLfloat x;
	GLfloat y;
};

struct Normal
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

struct Index
{
	GLushort x;
	GLushort y;
	GLushort z;
};

struct Surface
{
	Index vert[3];
};

struct Material
{
	float emission[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float ambient[4] = { 0.1f, 0.1f, 0.1f , 1.0f };
	float diffuse[4] = { 1.0f, 1.0f, 1.0f , 1.0f };
	float specular[4] = { 1.0f, 1.0f, 1.0f , 1.0f };
	float shininess = 128.0f;
};
Material material;

struct Light
{
	float ambient[4] = { 0.1f, 0.1f, 0.1f , 1.0f };
	float diffuse[4] = { 0.9f, 0.9f, 0.9f , 1.0f };
	float specular[4] = { 1.0f, 1.0f, 1.0f , 1.0f };
	float direction[3] = { 0.0f, 80.0f, -150.0f };
};
Light light;

GLuint shaderProgram;

GLint attribVertex;

GLint attribTex;

GLint attribNormal;

GLint unifTexture;

GLint unifRotate;
GLint unifMove;
GLint unifScale;

GLuint roadVBO;
GLuint busVBO;
GLuint grassVBO;

GLint roadTextureHandle;
GLint busTextureHandle;
GLint grassTextureHandle;

sf::Texture roadTexture;
sf::Texture busTexture;
sf::Texture grassTexture;

GLuint roadVAO;
GLuint busVAO;
GLuint grassVAO;

GLint Unif_transform_viewPosition;

GLint Unif_material_emission;
GLint Unif_material_ambient;
GLint Unif_material_diffuse;
GLint Unif_material_specular;
GLint Unif_material_shininess;

GLint Unif_light_ambient;
GLint Unif_light_diffuse;
GLint Unif_light_specular;
GLint Unif_light_attenuation;
GLint Unif_light_direction;

float busRotate[3] = { 0.0f, -3.14f, 0 };
float busMove[3] = { 0.0f, -1.0f, 0.5f };
float busScale[3] = { 0.1f, 0.1f, 0.1f };

float road1Move[3] = { 0.0f, -1.0f, 10.0f };
float road2Move[3] = { 0.0f, -1.0f, 30.0f };
float road3Move[3] = { 0.0f, -1.0f, 50.0f };
float roadRotate[3] = { 0.0f, 0.0f, 0.0f };
float roadScale[3] = { 0.1f, 0.1f, 0.1f };

float grassLeftRotate[3] = { 0.0f, -3.1415f, 0.0f };
float grassRightRotate[3] = { 0.0f, 0.0f, 0.0f };
float grassLeft1Move[3] = { -11.5f, -1.0f, 10.0f };
float grassLeft2Move[3] = { -11.5f, -1.0f, 30.0f };
float grassLeft3Move[3] = { -11.5f, -1.0f, 50.0f };
float grassRight1Move[3] = { 11.5f, -1.0f, 10.0f };
float grassRight2Move[3] = { 11.5f, -1.0f, 30.0f };
float grassRight3Move[3] = { 11.5f, -1.0f, 50.0f };
float grassScale[3] = { 0.1f, 0.1f, 0.1f };

float viewPosition[3] = { 0.0, 0.0, -0.0 };

int road_count = 0;
int bus_count = 0;
int grass_count = 0;
int plane_count = 0;

// Исходный код вершинного шейдера
const char* VertexShaderSource = R"(
    #version 330 core

	uniform vec3 rotate;
	uniform vec3 move;
	uniform vec3 scale;

	uniform vec3 viewPosition;

	uniform struct PointLight {
		vec3 direction;
		vec4 ambient;
		vec4 diffuse;
		vec4 specular;
	} light;

    in vec3 coord;
	in vec2 texcoord;
	in vec3 normal;

	out Vertex {
		vec2 texcoord;
		vec3 normal;
		vec3 lightDir;
		vec3 viewDir;
	} Vert;

    void main() {

		float x_scale = scale[0];
		float y_scale = scale[1];
		float z_scale = scale[2];
		vec3 vertex = coord * mat3(
									x_scale, 0, 0,
									0, y_scale, 0,
									0, 0, z_scale);	

		float x_angle = rotate[0];
        float y_angle = rotate[1];
		float z_angle = rotate[2];
        
        // Поворачиваем вершину
        mat3 rotate = mat3(
            1, 0, 0,
            0, cos(x_angle), -sin(x_angle),
            0, sin(x_angle), cos(x_angle)
        ) * mat3(
            cos(y_angle), 0, sin(y_angle),
            0, 1, 0,
            -sin(y_angle), 0, cos(y_angle)
        ) * mat3(
			cos(z_angle), -sin(z_angle), 0,
			sin(z_angle), cos(z_angle), 0,
			0, 0, 1
		);	

		vertex *= rotate;

		float x_move = move[0];
        float y_move = move[1];
		float z_move = move[2];
        
        vec4 vert = vec4(vertex, 1.0);

		vert *= mat4(
					1, 0, 0, x_move,
					0, 1, 0, y_move,
					0, 0, 1, z_move,
					0, 0, 0, 1);

		float c = -1;
		float last_z = vert.z;
		vert *= mat4(
					1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, 1, 0,
					0, 0, -1/c, 1);

		gl_Position = vec4(vert.xy, last_z * vert[3] / 1000, vert[3]);

		Vert.texcoord = texcoord;
		Vert.normal = normal * rotate;
		Vert.lightDir = light.direction;
		Vert.viewDir = viewPosition - vec3(vert);
    }
)";

// Исходный код фрагментного шейдера
const char* FragShaderSource = R"(
    #version 330 core

	in Vertex {
		vec2 texcoord;
		vec3 normal;
		vec3 lightDir;
		vec3 viewDir;
	} Vert;

	uniform struct PointLight {
		vec3 direction;
		vec4 ambient;
		vec4 diffuse;
		vec4 specular;
	} light;

	uniform struct Material {
		vec4 ambient;
		vec4 diffuse;
		vec4 specular;
		vec4 emission;
		float shininess;
	} material;

	uniform sampler2D textureData;

	out vec4 color;

    void main() {
		
		vec3 normal = normalize(Vert.normal);
		vec3 lightDir = normalize(Vert.lightDir);
		vec3 viewDir = normalize(Vert.viewDir);

		color = material.emission;
		color += material.ambient * light.ambient;

		float Ndot = max(dot(normal, lightDir), 0.0);
		color += material.diffuse * light.diffuse * Ndot;

		float RdotVpow = max(pow(dot(reflect(-lightDir, normal), viewDir), material.shininess), 0.0);
		color +=material.specular * light.specular * RdotVpow;

		color *= texture(textureData, Vert.texcoord);
    }
)";

void Init();
void Draw();
void Release();
void UpdateGrass();
void UpdateRoad();
void UpdateBus();

bool left = false;
bool right = false;
bool hasLight = true;

double angle = 0.5;
double radius = 150.0;

int main() {
	sf::Window window(sf::VideoMode(600, 600), "Ride of the Metallica", sf::Style::Default, sf::ContextSettings(24));
	window.setVerticalSyncEnabled(true);
	window.setActive(true);

	// Инициализация glew
	glewInit();
	glEnable(GL_DEPTH_TEST);

	Init();

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			else if (event.type == sf::Event::Resized) {
				glViewport(0, 0, event.size.width, event.size.height);
			}
			else if (event.type == sf::Event::KeyPressed) {
				switch (event.key.code) {
				case (sf::Keyboard::A):
				{
					left = true;
					break;
				}
				case (sf::Keyboard::D):
				{
					right = true;
					break;
				}
				case (sf::Keyboard::Left):
				{
					if (hasLight)
						light.direction[0] -= 5.0;
					break;
				}
				case (sf::Keyboard::Right):
				{
					if (hasLight)
					{ 
						angle += 0.1;
						light.direction[0] = radius * std::sin(angle) - 50;
						light.direction[1] = radius * std::cos(angle);
					}
					break;
				}
				case (sf::Keyboard::Up):
				{
					if (hasLight)
					{
						angle -= 0.1;
						light.direction[0] = radius * std::sin(angle) - 50;
						light.direction[1] = radius * std::cos(angle);
					}
					break;
				}
				case (sf::Keyboard::Down):
				{
					if (hasLight)
					{
						light.direction[2] -= 1.0;
					}
					break;
				}
				case (sf::Keyboard::Escape):
				{
					if (hasLight) 
					{
						light.direction[0] = 0.0f;
						light.direction[1] = 0.0f;
						light.direction[2] = 0.0f;
					}
					else 
					{
						light.direction[0] = 0.0f;
						light.direction[1] = 80.0f;
						light.direction[2] = -150.0f;
					}
					hasLight = !hasLight;
					break;
				}
				default: break;
				}
			}
			else if (event.type == sf::Event::KeyReleased) {
				switch (event.key.code) {
				case (sf::Keyboard::A):
				{
					left = false;
					break;
				}
				case (sf::Keyboard::D):
				{
					right = false;
					break;
				}
				default: break;
				}
			}
		}


		UpdateGrass();
		UpdateRoad();
		UpdateBus();


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Draw();

		window.display();
	}

	Release();
	return 0;
}




// Проверка ошибок OpenGL, если есть то вывод в консоль тип ошибки
void checkOpenGLerror() {
	GLenum errCode;
	// Коды ошибок можно смотреть тут
	// https://www.khronos.org/opengl/wiki/OpenGL_Error
	if ((errCode = glGetError()) != GL_NO_ERROR)
		std::cout << "OpenGl error!: " << errCode << std::endl;
}


void Draw() {
	glUseProgram(shaderProgram);

	glUniform4fv(Unif_material_emission, 1, material.emission);
	glUniform4fv(Unif_material_ambient, 1, material.ambient);
	glUniform4fv(Unif_material_diffuse, 1, material.diffuse);
	glUniform4fv(Unif_material_specular, 1, material.specular);
	glUniform1f(Unif_material_shininess, material.shininess);

	glUniform4fv(Unif_light_ambient, 1, light.ambient);
	glUniform4fv(Unif_light_diffuse, 1, light.diffuse);
	glUniform4fv(Unif_light_specular, 1, light.specular);
	glUniform3fv(Unif_light_direction, 1, light.direction);

	glUniform3fv(Unif_transform_viewPosition, 1, viewPosition);


	glUniform3fv(unifRotate, 1, roadRotate);
	glUniform3fv(unifMove, 1, road1Move);
	glUniform3fv(unifScale, 1, roadScale);
	glActiveTexture(GL_TEXTURE0);
	sf::Texture::bind(&roadTexture);
	glUniform1i(unifTexture, 0);
	glBindVertexArray(roadVAO);
	glDrawArrays(GL_TRIANGLES, 0, road_count);
	glBindVertexArray(0);

	glUniform3fv(unifRotate, 1, roadRotate);
	glUniform3fv(unifMove, 1, road2Move);
	glUniform3fv(unifScale, 1, roadScale);
	glActiveTexture(GL_TEXTURE0);
	sf::Texture::bind(&roadTexture);
	glUniform1i(unifTexture, 0);
	glBindVertexArray(roadVAO);
	glDrawArrays(GL_TRIANGLES, 0, road_count);
	glBindVertexArray(0);

	glUniform3fv(unifRotate, 1, roadRotate);
	glUniform3fv(unifMove, 1, road3Move);
	glUniform3fv(unifScale, 1, roadScale);
	glActiveTexture(GL_TEXTURE0);
	sf::Texture::bind(&roadTexture);
	glUniform1i(unifTexture, 0);
	glBindVertexArray(roadVAO);
	glDrawArrays(GL_TRIANGLES, 0, road_count);
	glBindVertexArray(0);

	glUniform3fv(unifRotate, 1, grassLeftRotate);
	glUniform3fv(unifMove, 1, grassLeft1Move);
	glUniform3fv(unifScale, 1, grassScale);
	glActiveTexture(GL_TEXTURE0);
	sf::Texture::bind(&grassTexture);
	glUniform1i(unifTexture, 0);
	glBindVertexArray(grassVAO);
	glDrawArrays(GL_TRIANGLES, 0, grass_count);
	glBindVertexArray(0);

	glUniform3fv(unifRotate, 1, grassLeftRotate);
	glUniform3fv(unifMove, 1, grassLeft2Move);
	glUniform3fv(unifScale, 1, grassScale);
	glActiveTexture(GL_TEXTURE0);
	sf::Texture::bind(&grassTexture);
	glUniform1i(unifTexture, 0);
	glBindVertexArray(grassVAO);
	glDrawArrays(GL_TRIANGLES, 0, grass_count);
	glBindVertexArray(0);


	glUniform3fv(unifRotate, 1, grassLeftRotate);
	glUniform3fv(unifMove, 1, grassLeft3Move);
	glUniform3fv(unifScale, 1, grassScale);
	glActiveTexture(GL_TEXTURE0);
	sf::Texture::bind(&grassTexture);
	glUniform1i(unifTexture, 0);
	glBindVertexArray(grassVAO);
	glDrawArrays(GL_TRIANGLES, 0, grass_count);
	glBindVertexArray(0);

	glUniform3fv(unifRotate, 1, grassRightRotate);
	glUniform3fv(unifMove, 1, grassRight1Move);
	glUniform3fv(unifScale, 1, grassScale);
	glActiveTexture(GL_TEXTURE0);
	sf::Texture::bind(&grassTexture);
	glUniform1i(unifTexture, 0);
	glBindVertexArray(grassVAO);
	glDrawArrays(GL_TRIANGLES, 0, grass_count);
	glBindVertexArray(0);

	glUniform3fv(unifRotate, 1, grassRightRotate);
	glUniform3fv(unifMove, 1, grassRight2Move);
	glUniform3fv(unifScale, 1, grassScale);
	glActiveTexture(GL_TEXTURE0);
	sf::Texture::bind(&grassTexture);
	glUniform1i(unifTexture, 0);
	glBindVertexArray(grassVAO);
	glDrawArrays(GL_TRIANGLES, 0, grass_count);
	glBindVertexArray(0);

	glUniform3fv(unifRotate, 1, grassRightRotate);
	glUniform3fv(unifMove, 1, grassRight3Move);
	glUniform3fv(unifScale, 1, grassScale);
	glActiveTexture(GL_TEXTURE0);
	sf::Texture::bind(&grassTexture);
	glUniform1i(unifTexture, 0);
	glBindVertexArray(grassVAO);
	glDrawArrays(GL_TRIANGLES, 0, grass_count);
	glBindVertexArray(0);


	glUniform3fv(unifRotate, 1, busRotate);
	glUniform3fv(unifMove, 1, busMove);
	glUniform3fv(unifScale, 1, busScale);
	glActiveTexture(GL_TEXTURE0);
	sf::Texture::bind(&busTexture);
	glUniform1i(unifTexture, 0);
	glBindVertexArray(busVAO);
	glDrawArrays(GL_TRIANGLES, 0, bus_count);
	glBindVertexArray(0);

	glUseProgram(0);
	checkOpenGLerror();
}


// Функция печати лога шейдера
void ShaderLog(unsigned int shader)
{
	int infologLen = 0;
	int charsWritten = 0;
	char* infoLog;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
	if (infologLen > 1)
	{
		infoLog = new char[infologLen];
		if (infoLog == NULL)
		{
			std::cout << "ERROR: Could not allocate InfoLog buffer" << std::endl;
			exit(1);
		}
		glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog);
		std::cout << "InfoLog: " << infoLog << "\n\n\n";
		delete[] infoLog;
	}
}



void InitVBO()
{
	auto pos_tex = figure_model("models/road.obj").get_all_values();
	road_count = pos_tex.size() * 3;

	glGenBuffers(1, &roadVBO);
	glGenVertexArrays(1, &roadVAO);

	glBindVertexArray(roadVAO);

	glEnableVertexAttribArray(attribVertex);
	glEnableVertexAttribArray(attribTex);
	glEnableVertexAttribArray(attribNormal);
	glBindBuffer(GL_ARRAY_BUFFER, roadVBO);
	glBufferData(GL_ARRAY_BUFFER, pos_tex.size() * sizeof(GLfloat), pos_tex.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(attribTex, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(attribNormal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);



	pos_tex = figure_model("models/grass.obj").get_all_values();
	grass_count = pos_tex.size() * 3;

	glGenBuffers(1, &grassVBO);
	glGenVertexArrays(1, &grassVAO);

	glBindVertexArray(grassVAO);

	glEnableVertexAttribArray(attribVertex);
	glEnableVertexAttribArray(attribTex);
	glEnableVertexAttribArray(attribNormal);
	glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
	glBufferData(GL_ARRAY_BUFFER, pos_tex.size() * sizeof(GLfloat), pos_tex.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(attribTex, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(attribNormal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);


	glBindVertexArray(0);



	pos_tex = figure_model("models/bus2.obj").get_all_values();
	bus_count = pos_tex.size() * 3;

	glGenBuffers(1, &busVBO);
	glGenVertexArrays(1, &busVAO);

	glBindVertexArray(busVAO);

	glEnableVertexAttribArray(attribVertex);
	glEnableVertexAttribArray(attribTex);
	glEnableVertexAttribArray(attribNormal);
	glBindBuffer(GL_ARRAY_BUFFER, busVBO);
	glBufferData(GL_ARRAY_BUFFER, pos_tex.size() * sizeof(GLfloat), pos_tex.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(attribTex, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(attribNormal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);


	checkOpenGLerror();
}

void InitUniform(GLint& id, const char* name)
{
	id = glGetUniformLocation(shaderProgram, name);
	if (id == -1)
	{
		std::cout << "could not bind uniform " << name << std::endl;
		return;
	}
}

void InitShader() {
	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vShader, 1, &VertexShaderSource, NULL);
	glCompileShader(vShader);
	std::cout << "vertex shader \n";
	ShaderLog(vShader);

	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fShader, 1, &FragShaderSource, NULL);
	glCompileShader(fShader);
	std::cout << "fragment shader \n";
	ShaderLog(fShader);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vShader);
	glAttachShader(shaderProgram, fShader);

	glLinkProgram(shaderProgram);
	int link_status;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &link_status);
	if (!link_status)
	{
		std::cout << "error attach shaders \n";
		return;
	}

	attribVertex = glGetAttribLocation(shaderProgram, "coord");
	if (attribVertex == -1)
	{
		std::cout << "could not bind attrib coord" << std::endl;
		return;
	}

	attribTex = glGetAttribLocation(shaderProgram, "texcoord");
	if (attribVertex == -1)
	{
		std::cout << "could not bind attrib texcoord" << std::endl;
		return;
	}

	attribNormal = glGetAttribLocation(shaderProgram, "normal");
	if (attribNormal == -1)
	{
		std::cout << "could not bind attrib normal" << std::endl;
		return;
	}

	InitUniform(unifTexture, "textureData");
	InitUniform(unifRotate, "rotate");
	InitUniform(unifMove, "move");
	InitUniform(unifScale, "scale");
	InitUniform(Unif_transform_viewPosition, "viewPosition");
	InitUniform(Unif_material_emission, "material.emission");
	InitUniform(Unif_material_ambient, "material.ambient");
	InitUniform(Unif_material_diffuse, "material.diffuse");
	InitUniform(Unif_material_specular, "material.specular");
	InitUniform(Unif_material_shininess, "material.shininess");
	InitUniform(Unif_light_ambient, "light.ambient");
	InitUniform(Unif_light_diffuse, "light.diffuse");
	InitUniform(Unif_light_specular, "light.specular");
	InitUniform(Unif_light_direction, "light.direction");

	checkOpenGLerror();
}

void InitTexture()
{
	const char* road = "textures/road.png";
	const char* bus = "textures/bus2.png";
	const char* grass = "textures/grass.png";
	if (!busTexture.loadFromFile(bus))
	{
		std::cout << "could not load texture bus";
		return;
	}
	if (!roadTexture.loadFromFile(road))
	{
		std::cout << "could not load texture road";
		return;
	}
	if (!grassTexture.loadFromFile(grass))
	{
		std::cout << "could not load texture grass";
		return;
	}

	roadTextureHandle = roadTexture.getNativeHandle();
	busTextureHandle = busTexture.getNativeHandle();
	grassTextureHandle = grassTexture.getNativeHandle();
}


void Init() {
	InitShader();
	InitVBO();
	InitTexture();
}

void ReleaseShader() {
	glUseProgram(0);
	glDeleteProgram(shaderProgram);
}

void ReleaseVBO()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteVertexArrays(1, &roadVAO);
	glDeleteBuffers(1, &roadVBO);

	glDeleteVertexArrays(1, &busVAO);
	glDeleteBuffers(1, &busVBO);
}

void Release() {
	ReleaseShader();
	ReleaseVBO();
}

void BusRight()
{
	busMove[0] += 0.05;
	busRotate[1] += 0.05;
}

void BusLeft()
{
	busMove[0] -= 0.05;
	busRotate[1] -= 0.05;
}

void UpdateBus()
{
	if (left && busMove[0] >= -1.3)
		BusLeft();
	if (right && busMove[0] <= 1.3)
		BusRight();

	if (busRotate[1] < -3.14f && std::abs(busRotate[1] + 3.14) > 0.03f)
		busRotate[1] += 0.03f;
	else if (busRotate[1] > -3.14f && std::abs(busRotate[1] + 3.14) > 0.03f)
		busRotate[1] -= 0.03f;
	if (std::abs(busMove[1] + 0.25) < 0.01f)
		busMove[1] += 1.0f;
}

void UpdateRoad()
{
	road1Move[2] -= 0.1;
	road2Move[2] -= 0.1;
	road3Move[2] -= 0.1;
	if (std::abs(road1Move[2] + 10) < 0.1)
		road1Move[2] = 10 + std::abs(road1Move[2] + 10);
	if (std::abs(road2Move[2] - 10) < 0.1)
		road2Move[2] = 30  +std::abs(road2Move[2] - 10);
	if (std::abs(road3Move[2] - 30) < 0.1)
		road3Move[2] = 50 + std::abs(road3Move[2] - 30);
}

void UpdateGrass()
{
	grassLeft1Move[2] -= 0.07;
	grassLeft2Move[2] -= 0.07;
	grassLeft3Move[2] -= 0.07;
	grassRight1Move[2] -= 0.07;
	grassRight2Move[2] -= 0.07;
	grassRight3Move[2] -= 0.07;

	if (std::abs(grassLeft1Move[2] + 10) < 0.1)
		grassLeft1Move[2] = 10;
	if (std::abs(grassLeft2Move[2] - 10) < 0.1)
		grassLeft2Move[2] = 30;
	if (std::abs(grassLeft3Move[2] - 30) < 0.1)
		grassLeft3Move[2] = 50;

	if (std::abs(grassRight1Move[2] + 10) < 0.1)
		grassRight1Move[2] = 10;
	if (std::abs(grassRight2Move[2] - 10) < 0.1)
		grassRight2Move[2] = 30;
	if (std::abs(grassRight3Move[2] - 30) < 0.1)
		grassRight3Move[2] = 50;
}



