#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <vector>
#include <iostream>

const char* vertexShaderRota = R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    uniform mat4 uProjection;
    void main()
    {
        gl_Position = uProjection * vec4(aPos, 1.0f);
    }
)glsl";

const char* fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(1.0, 0.5, 0.2, 1.0);
    }
)glsl";

float deltaTime = 0.0;
float lastTime = 0.0;
int nbFrames = 0;

bool running = true;
bool pause = false;

// handle window resize : adjust the viewport
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// keyboard input callback
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        pause = !pause; // act as a toggle key
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        glfwTerminate();
        glfwSetWindowShouldClose(window, true);
    }
};

GLFWwindow* StartGLU() {
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW, panic" << std::endl;
        return nullptr;
    }
    // for MSAA (multisampling anti-aliasing) : reduce sketchy edge of primitive type
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(800, 600, "DoublePendulum", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    // register framebuffer_size_callback() as the callback function for window resize
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // register keyCallback() as the callback for keyboard input
    glfwSetKeyCallback(window, keyCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW." << std::endl;
        glfwTerminate();
        return nullptr;
    }

    //glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, 800, 600);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // for MSAA
    glEnable(GL_MULTISAMPLE);

    return window;
}

GLuint CreateShaderProgram(const char* vertexSource, const char* fragmentSource) {
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
    }

    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
    }

    // Shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

static void CreateVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, int vertexCount) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // Bind VAO
    glBindVertexArray(VAO);
    // Bind and fill VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices, GL_STATIC_DRAW);
    // Vertex attribute (VAO)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

glm::mat4 ComputeOrthoMat(int width, int height) {
    float aspect = (float)width / (float)height;  // e.g., 800 / 600 = 1.333

    glm::mat4 projection;
    if (aspect >= 1.0f) {
        // Wide window (e.g. 800x600): expand X range
        projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f);
    }
    else {
        // Tall window (e.g. 600x800): expand Y range
        projection = glm::ortho(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect);
    }
    //projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f);
    return projection;
}

void buildCircle(float radius, int vCount, float xOffset, float yOffset, std::vector<glm::vec3>& vertices)
{
    float angle = 360.0f / vCount;
    int triangleCount = vCount - 2;
    std::vector<glm::vec3> temp;

    for (int i = 0; i < vCount; i++)
    {
        float currentAngle = angle * i;
        float x = radius * cos(glm::radians(currentAngle)) + xOffset;
        float y = radius * sin(glm::radians(currentAngle)) + yOffset;

        temp.push_back(glm::vec3(x, y, 0.0f));
    }

    // push indexes of each triangle points
    for (int i = 0; i < triangleCount; i++)
    {
        vertices.push_back(temp[0]);
        vertices.push_back(temp[i + 1]);
        vertices.push_back(temp[i + 2]);
    }
}

const float m1 = 2.0;
const float m2 = 2.0;
const float l1 = 0.5;
const float l2 = 0.5;

const float g = 9.81;

// small time interval, set the simulation speed
const float h = 0.003;

// initial condition
std::vector<float> stateVec = { glm::pi<float>() / 2 , 0.0f , glm::pi<float>() / 2 , 0.0f };

// y is the state vector at instant t, this function give the time derivative of this vector
std::vector<float> functionForRK4(std::vector<float> y) {
    float theta1 = y[0];
    float omega1 = y[1];
    float theta2 = y[2];
    float omega2 = y[3];

    float delta = theta1 - theta2;
    float denom1 = l1 * (2 * m1 + m2 - m2 * cosf(2 * delta));
    float denom2 = l2 * (2 * m1 + m2 - m2 * cosf(2 * delta));

    float omega1_dot = (
        -g * (2 * m1 + m2) * sinf(theta1)
        - m2 * g * sinf(theta1 - 2 * theta2)
        - 2 * sinf(delta) * m2 * (omega2 * omega2 * l2 + omega1 * omega1 * l1 * cosf(delta))
        ) / denom1;

    float omega2_dot = (
        2 * sinf(delta) * (omega1 * omega1 * l1 * (m1 + m2)
            + g * (m1 + m2) * cosf(theta1)
            + omega2 * omega2 * l2 * m2 * cosf(delta))
        ) / denom2;

    return { omega1, omega1_dot, omega2, omega2_dot };
}

// add 2 vector together
std::vector<float> addVec(std::vector<float> a, std::vector<float> b) {
    for (int i = 0; i < b.size(); i++) {
        a[i] = b[i] + a[i];
    }
    return a;
}

// multiply a vector by a scalar
std::vector<float> multVec(std::vector<float> vec, float scalar) {
    for (int i = 0; i < vec.size(); i++) {
        vec[i] = vec[i] * scalar;
    }
    return vec;
}

// given y_{n} at a time instant t_{n} it compute and return y_{n+1} for the next time instant t_{n+1}
// y contain theta1 and theta2
std::vector<float> computeRK4(std::vector<float> y) {
    std::vector<float> k1 = functionForRK4(y);
    std::vector<float> k2 = functionForRK4(addVec(y, multVec(k1, h/2)));
    std::vector<float> k3 = functionForRK4(addVec(y, multVec(k2, h/2)));
    std::vector<float> k4 = functionForRK4(addVec(y, multVec(k3, h  )));

    std::vector<float> res = {0,0,0,0};
    float fac = h / 6;

    for (int i = 0; i < res.size(); i++) {
        res[i] = y[i] + fac * (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]);
    }
    return res;
}

// compute the position (x,y) of the pendulum, given the 2 angle theta1 and theta2
std::vector<std::vector<float>> polarToCart(float theta1, float theta2) {
    std::vector<float> r1 =            { l1 * sinf(theta1) , -l1 * cosf(theta1) };
    std::vector<float> r2 = addVec(r1, { l2 * sinf(theta2) , -l2 * cosf(theta2) });
    return { r1 , r2 };
}



int main()
{
    GLFWwindow* window = StartGLU();

    // when enabled : fps sync with monitor frequency, 144 Hz => 144 fps
    // when disabled : fps = 2000-4000
    // drasticly reduce GPU usage by enabling V-sync
    glfwSwapInterval(1);

    // -- init vertices buffer
    std::vector<glm::vec3> circleVerti;
    buildCircle(0.5, 64, 0, 0, circleVerti);

    // Vertex Array Object, Vertex Buffer Object
    GLuint VAO, VBO;
    //CreateVBOVAO(VAO, VBO, vertices, sizeof(vertices));
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // Bind VAO
    glBindVertexArray(VAO);
    // Bind and fill VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * circleVerti.size(), &circleVerti[0], GL_STATIC_DRAW);
    // Vertex attribute (VAO)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);


    // shader creation and linking
    GLuint shaderProgram = CreateShaderProgram(vertexShaderRota, fragmentShaderSource);
    glUseProgram(shaderProgram);

    // get variable of the shader program for later use
    GLuint projLoc = glGetUniformLocation(shaderProgram, "uProjection");

    // to have a "square" NDC, read more below
    int lastWidth;
    int lastHeight;
    glfwGetFramebufferSize(window, &lastWidth, &lastHeight);
    // set the first ortho projection matrix to scale the NDC for the first time
    glm::mat4 projection = ComputeOrthoMat(lastWidth, lastHeight);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


    // Render loop
    while (!glfwWindowShouldClose(window)) {

        //set the font color of the window
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        
        // simulation and rendering part

        if (!pause) {
            circleVerti.clear();

            // compute the new state vector
            stateVec = computeRK4(stateVec);

            // first vector is the first (simple) pendulum
            // second vector is the double pendulum
            std::vector<std::vector<float>> posVec = polarToCart(stateVec[0], stateVec[2]);

            // simple pendulum
            buildCircle(0.1, 64, posVec[0][0], posVec[0][1], circleVerti);
            // double pendulum
            buildCircle(0.1, 64, posVec[1][0], posVec[1][1], circleVerti);
        }


        // Bind and update VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * circleVerti.size(), &circleVerti[0], GL_STATIC_DRAW);



        // since the viewport isn't always a square, the NDC isn't one neither
        // so we define our vertices to be like in a cartesian plane, and then scale it to match the NDC deformation
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        // only update the orthoprojection matrix in the shader if we resized the window
        if (width != lastWidth || height != lastHeight) {
            std::cout << "ortho matrix updated (window resized)\n";
            lastWidth = width;
            lastHeight = height;
            glm::mat4 projection = ComputeOrthoMat(width, height);
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        }

        // poll IO event (keyboard, moove/resize, close)
        glfwPollEvents();

        // render stuff on the screen
        glDrawArrays(GL_TRIANGLES, 0, circleVerti.size()); // 3
        //glDrawElements(GL_TRIANGLES, circleIndices.size(), GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
    }

    // properly exit the application
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}
