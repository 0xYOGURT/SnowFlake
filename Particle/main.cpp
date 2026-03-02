#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

struct Particle {
    float x, y;
    float vx, vy;
    float life;
};

std::vector<Particle> particles;
const int MAX_PARTICLES = 2000;

float zoom = 1.0f;
float emitterX = 0.0f, emitterY = -0.5f; // start near bottom
int particleType = 0; // 0=fire,1=smoke,2=rain

GLuint VAO, VBO;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    zoom *= (yoffset > 0) ? 0.9f : 1.1f;
}

void initParticles() {
    particles.resize(MAX_PARTICLES);
    srand((unsigned int)time(0));
    for (auto& p : particles) {
        p.x = emitterX;
        p.y = emitterY;
        p.vx = ((rand() % 200) / 100.0f) - 1.0f; // spread X
        p.vy = ((rand() % 200) / 200.0f) + 0.5f;  // upward bias
        p.life = (rand() % 100) / 100.0f;
    }
}

void updateParticles(float dt) {
    for (auto& p : particles) {
        p.x += p.vx * dt;
        p.y += p.vy * dt;

        // Apply gravity for smoke/fire
        if (particleType != 2) p.vy -= 0.001f * dt;
        else p.vy -= 0.002f * dt; // rain falls faster

        p.life -= 0.005f * dt;

        if (p.life <= 0.0f || p.y < -1.0f || p.y > 1.0f) { // reset
            p.x = emitterX + (((rand() % 100) / 50.0f) - 1.0f) * 0.1f;
            p.y = emitterY;
            p.vx = ((rand() % 200) / 100.0f) - 1.0f;
            p.vy = ((rand() % 200) / 200.0f) + ((particleType == 2) ? -1.0f : 0.5f);
            p.life = 1.0f;
        }
    }
}

GLuint createShader(const char* vertexSrc, const char* fragmentSrc) {
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSrc, NULL);
    glCompileShader(vertex);

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSrc, NULL);
    glCompileShader(fragment);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return program;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Particle System", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetScrollCallback(window, scroll_callback);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_PROGRAM_POINT_SIZE);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    const char* vertexShaderSrc = R"(
        #version 330 core
        layout(location=0) in vec2 aPos;
        uniform float zoom;
        void main(){
            gl_Position = vec4(aPos*zoom,0.0,1.0);
            gl_PointSize = 5.0;
        }
    )";

    const char* fragmentShaderSrc = R"(
        #version 330 core
        out vec4 FragColor;
        uniform int type;
        void main(){
            if(type==0) { // fire
                FragColor = vec4(1.0, 0.4+gl_PointCoord.y*0.6, 0.0,1.0);
            }
            else if(type==1) { // smoke
                FragColor = vec4(0.3+gl_PointCoord.x*0.4,0.3+gl_PointCoord.y*0.4,0.3,0.8);
            }
            else { // rain
                FragColor = vec4(0.2,0.5,1.0,1.0);
            }
        }
    )";

    GLuint shader = createShader(vertexShaderSrc, fragmentShaderSrc);

    initParticles();

    while (!glfwWindowShouldClose(window)) {
        float dt = 1.0f;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Controls
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) emitterY += 0.01f;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) emitterY -= 0.01f;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) emitterX -= 0.01f;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) emitterX += 0.01f;

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) particleType = 0;
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) particleType = 1;
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) particleType = 2;

        updateParticles(dt);

        // Upload positions
        std::vector<float> buffer(MAX_PARTICLES * 2);
        for (int i = 0; i < MAX_PARTICLES; i++) {
            buffer[i * 2] = particles[i].x;
            buffer[i * 2 + 1] = particles[i].y;
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, buffer.size() * sizeof(float), buffer.data());

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader);
        glUniform1f(glGetUniformLocation(shader, "zoom"), zoom);
        glUniform1i(glGetUniformLocation(shader, "type"), particleType);
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, MAX_PARTICLES);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
