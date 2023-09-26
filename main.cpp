#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "stb\stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "shader.h"
#include "PerlinNoise.hpp"

#include <iostream>
#include <vector>
#include <math.h>

#include <map>
#include <filesystem>
#include <windows.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char *path);
glm::vec3 getRayFromMouse(double mouseX, double mouseY, glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
glm::vec3 getRayPlaneIntersection(glm::vec3 ray_origin, glm::vec3 ray_direction, glm::vec3 plane_normal, glm::vec3 plane_point);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods, glm::mat4 projMatrix, glm::mat4 viewMatrix, Shader ourShader);
void RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color);

// Create a ray from the mouse cursor
glm::vec3 CreateRay(GLFWwindow* window, glm::mat4 proj, glm::mat4 view)
{
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    float mouseX = xPos / (width * 0.5f) - 1.0f;
    float mouseY = yPos / (height * 0.5f) - 1.0f;

    glm::mat4 invVP = glm::inverse(proj * view);
    glm::vec4 screenPos = glm::vec4(mouseX, -mouseY, 1.0f, 1.0f);
    glm::vec4 worldPos = invVP * screenPos;

    glm::vec3 dir = glm::normalize(glm::vec3(worldPos));

    return dir;
}

bool RayIntersectsObject(glm::vec3 rayStart, glm::vec3 rayDir, glm::vec3 objectPos, glm::vec3 objectSize, float maxDistance,
    glm::vec3* hitNormal) {
    // Calculate the minimum and maximum points of the object's bounding box
    glm::vec3 min = objectPos - objectSize * 0.5f;
    glm::vec3 max = objectPos + objectSize * 0.5f;

    // Calculate the intersection points of the ray with each plane of the bounding box
    float tmin = (min.x - rayStart.x) / rayDir.x;
    float tmax = (max.x - rayStart.x) / rayDir.x;

    if (tmin > tmax) std::swap(tmin, tmax);

    float tymin = (min.y - rayStart.y) / rayDir.y;
    float tymax = (max.y - rayStart.y) / rayDir.y;

    if (tymin > tymax) std::swap(tymin, tymax);

    if (tymin > tmin)
        tmin = tymin;
    if (tymax < tmax)
        tmax = tymax;

    if ((tmin > tmax))
        return false;

    float tzmin = (min.z - rayStart.z) / rayDir.z;
    float tzmax = (max.z - rayStart.z) / rayDir.z;

    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if (tzmin > tmin)
        tmin = tzmin;
    if (tzmax < tmax)
        tmax = tzmax;

    if ((tmin > tmax))
        return false;

    // Calculate the intersection point
    glm::vec3 intersectionPoint = rayStart + tmin * rayDir;

    if (intersectionPoint.x == min.x) *hitNormal = glm::vec3(-1, 0, 0);
    if (intersectionPoint.x == max.x) *hitNormal = glm::vec3(1, 0, 0);

    if (intersectionPoint.y == min.y) *hitNormal = glm::vec3(0, -1, 0);
    if (intersectionPoint.y == max.y) *hitNormal = glm::vec3(0, 1, 0);

    if (intersectionPoint.z == min.z) *hitNormal = glm::vec3(0, 0, -1);
    if (intersectionPoint.z == max.z) *hitNormal = glm::vec3(0, 0, 1);

    // Check if the intersection point is within the maximum distance from the camera
    float distance = glm::length(intersectionPoint - rayStart);
    if (distance > maxDistance)
        return false;

    return true;
}



// set up vertex data (and buffer(s)) and configure vertex attributes
// ------------------------------------------------------------------
float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};



void tree(Shader shader, unsigned int wood_texture, unsigned int leaf_texture, unsigned int x, unsigned int y, unsigned int z)
{
    for (unsigned int height = y; height < y+3.0f; height++)
    {

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wood_texture);

        glm::mat4 wood_model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        wood_model = glm::translate(wood_model, glm::vec3(x, height, z));
        //float angle = 20.0f * i;
        //model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        shader.setMat4("model", wood_model);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }


    for (float z_new = z/2; z_new < z+3.0f; z_new++)
    {
        for (float x_new = x/2; x_new < x+3.0f; x_new++)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, leaf_texture);

            glm::mat4 leaf_model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
            leaf_model = glm::translate(leaf_model, glm::vec3(x_new, y + 3.0f, z_new));
            //float angle = 20.0f * i;
            //model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            shader.setMat4("model", leaf_model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
}

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true; 
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;
float cameraSpeed = 0.05f;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

bool canSpawnCube = true;

bool blocks[30][10][30];
unsigned int block_texture;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;

struct Block {
    glm::vec3 position;
    GLuint texture;
};

// Constants
const float GRAVITY = -9.8f;
const float GROUND_Y = 10.0f;

glm::vec3 cameraVel = glm::vec3(0.0f, 0.0f, 0.0f);

void simulatePhysics(float deltaTime, const std::vector<Block>& cubePositions)
{
    // Apply gravity to the camera
    cameraVel.y += GRAVITY * deltaTime;

    // Update the camera position
    glm::vec3 oldCameraPos = cameraPos;
    cameraPos += cameraVel * deltaTime;

    // Check for collision with the ground
    if (cameraPos.y < GROUND_Y)
    {
        // The camera has collided with the ground
        // Set its position and velocity to zero
        cameraPos.y = GROUND_Y;
        cameraVel.y = 0.0f;
    }

    // Check for collision with each object
    for (const auto& cube : cubePositions)
    {
        if (glm::distance(cameraPos, cube.position) < 1.0f)  // assuming objects are cubes with side length 1
        {
            // The camera has collided with an object
            // Reset its position to the previous frame's position
            cameraPos = oldCameraPos;
            break;
        }
    }
}

unsigned int VBO, VAO, textVBO, textVAO;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Minecraft", NULL, NULL);
    //glfwSwapInterval(0);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_POLYGON_SMOOTH);

    Shader shader("text.vert", "text.frag");
    glm::mat4 textProjection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(textProjection));

        // FreeType
    // --------
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, "arial.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            unsigned int TXTRE;
            glGenTextures(1, &TXTRE);
            glBindTexture(GL_TEXTURE_2D, TXTRE);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                TXTRE,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("shader.vert", "shader.frag");

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // load and create a texture 
    // -------------------------

    unsigned int grasstexture = loadTexture("textures\\grassblock.jpg");
    unsigned int dirttexture = loadTexture("textures\\dirtblock.jpg");
    unsigned int stonetexture = loadTexture("textures\\stoneblock.jpg");
    unsigned int diamondtexture = loadTexture("textures\\diamondblock.jpg");
    unsigned int coaltexture = loadTexture("textures\\coalblock.jpg");
    unsigned int irontexture = loadTexture("textures\\ironblock.jpg");
    unsigned int watertexture = loadTexture("textures\\waterblock.jpg");
    unsigned int leaftexture = loadTexture("textures\\leafblock.jpg");
    unsigned int woodtexture = loadTexture("textures\\woodblock.jpg");
    unsigned int bedrocktexture = loadTexture("textures\\bedrockblock.jpg");
    unsigned int planktexture = loadTexture("textures\\plankblock.jpg");
    unsigned int bricktexture = loadTexture("textures\\brickblock.jpg");
    unsigned int oaktexture = loadTexture("textures\\oakblock.jpg");
    unsigned int glasstexture = loadTexture("textures\\glassblock.jpg");

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    ourShader.use();
    ourShader.setInt("texture1", 0);
    //ourShader.setInt("texture2", 1);

    /*float distance = 5.0f;
    glm::vec3 cubePos = cameraPos + cameraFront + distance;
    std::vector<glm::vec3> cubePositions;
    cubePositions.push_back(cubePos);*/

    float distance = 5.0f;
    glm::vec3 cubePos = cameraPos + cameraFront + distance;
    std::vector<Block> cubePositions;

    double prevTime = 0.0;
    double crntTime = 0.0;
    double timeDiff;
    unsigned int counter = 0;

    glfwSwapInterval(0);

    // uncomment the line below this text to draw everything in wireframe polygons
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Initialize the blocks array
    for (unsigned int x = 0; x < 30; x++)
    {
        for (unsigned int y = 0; y < 10; y++)
        {
            for (unsigned int z = 0; z < 30; z++)
            {
                blocks[x][y][z] = true;
            }
        }
    }

    // Keep track of the time when the last block was spawned
    double lastBlockSpawnTime = 0.0;
    double blockSpawnDelay = 0.5; // Delay between block spawns in seconds

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // TODO: make it so i can build on the left, right, front, bottom sides when building (NOT DONE)
        // TODO: fix block building and block deleting (DONE 75%)
        // TODO: make block building also easier and block deleting also easier (DONE)
        // TODO: build a house (DONE)
        // TODO: add perlin noise generation to the map (NOT DONE)
        // TODO: add rigid bodies (camera gravity, camera collisions) (DONE 30%)
        // TODO: add buttons (NOT DONE)

        //simulatePhysics(.5);

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) block_texture = dirttexture;
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) block_texture = grasstexture;
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) block_texture = stonetexture;
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) block_texture = planktexture;
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) block_texture = bricktexture;
        if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) block_texture = oaktexture;
        if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) block_texture = glasstexture;

        crntTime = glfwGetTime();
        timeDiff = crntTime - prevTime;
        counter++;
        if (timeDiff >= 1.0 / 30.0)
        {
            std::string FPS = std::to_string((1.0 / timeDiff) * counter);
            std::string ms = std::to_string((timeDiff / counter) * 1000);
            std::string newTitle = "Minecraft - " + FPS + "FPS / " + ms + "ms";
            glfwSetWindowTitle(window, newTitle.c_str());
            prevTime = crntTime;
            counter = 0;
        }

        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------

        //61.0f/255.0f, 174.0f/255.0f, 255.0f/255.0f, 1.0f

        glClearColor(61.0f / 255.0f, 174.0f / 255.0f, 255.0f / 255.0f, 1.0f); // this is background text
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        RenderText(shader, "press 1 to change material to dirt", 470.0f, 570.0f, 0.5f, glm::vec3(255.0f, 0.0f, 0.0f));
        RenderText(shader, "press 2 to change material to grass", 470.0f, 550.0f, 0.5f, glm::vec3(255.0f, 0.0f, 0.0f));
        RenderText(shader, "press 3 to change material to stone", 470.0f, 530.0f, 0.5f, glm::vec3(255.0f, 0.0f, 0.0f));
        RenderText(shader, "press 4 to change material to wood", 470.0f, 510.0f, 0.5f, glm::vec3(255.0f, 0.0f, 0.0f));
        RenderText(shader, "press 5 to change material to wall", 470.0f, 490.0f, 0.5f, glm::vec3(255.0f, 0.0f, 0.0f));
        RenderText(shader, "press 6 to change material to wood", 470.0f, 470.0f, 0.5f, glm::vec3(255.0f, 0.0f, 0.0f));
        RenderText(shader, "press 7 to change material to glass", 470.0f, 450.0f, 0.5f, glm::vec3(255.0f, 0.0f, 0.0f));

        // bind textures on corresponding texture units
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, texture2);

        // activate shader
        ourShader.use();

        // pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 objectProjection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", objectProjection);

        // camera/view transformation
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        ourShader.setMat4("view", view);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diamondtexture);

        glm::mat4 diamond_model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        diamond_model = glm::translate(diamond_model, glm::vec3(5.0f, 15.0f, 5.0f));
        //float angle = 20.0f * i;
        //model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        ourShader.setMat4("model", diamond_model);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, irontexture);

        glm::mat4 iron_model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        iron_model = glm::translate(iron_model, glm::vec3(7.0f, 15.0f, 5.0f));
        //float angle = 20.0f * i;
        //model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        ourShader.setMat4("model", iron_model);

        glDrawArrays(GL_TRIANGLES, 0, 36);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, coaltexture);

        glm::mat4 coal_model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        coal_model = glm::translate(coal_model, glm::vec3(9.0f, 15.0f, 5.0f));
        //float angle = 20.0f * i;
        //model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        ourShader.setMat4("model", coal_model);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, watertexture);

        glm::mat4 water_model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        water_model = glm::translate(water_model, glm::vec3(11.0f, 15.0f, 5.0f));
        //float angle = 20.0f * i;
        //model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        ourShader.setMat4("model", water_model);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        //tree(ourShader, woodtexture, leaftexture, 10, 0, 10);

/*// Check if the left mouse button was pressed
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            // Left mouse button was pressed
            glm::vec3 rayDir = CreateRay(window, projection, view);

            // Check if the ray intersects with any objects
            for (unsigned int x = 0; x < 30; x++)
            {
                for (unsigned int y = 0; y < 10; y++)
                {
                    for (unsigned int z = 0; z < 30; z++)
                    {
                        bool intersects = RayIntersectsObject(cameraPos, rayDir, glm::vec3(x, y, z), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);

                        if (intersects && canSpawnCube)
                        {
                            // The ray intersects with the object
                            // Place a new block on top of the intersected block
                            cubePositions.push_back(glm::vec3(x, y + 1, z));
                            canSpawnCube = false;
                        }
                    }
                }
            }

            // Check if the ray intersects with any previously spawned cubes
            for (const auto& cubePos : cubePositions)
            {
                bool intersects = RayIntersectsObject(cameraPos, rayDir, cubePos, glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);

                if (intersects && canSpawnCube)
                {
                    // The ray intersects with the previously spawned cube
                    // Place a new block on top of the intersected block
                    cubePositions.push_back(cubePos + glm::vec3(0.0f, 1, 0.0f));
                    canSpawnCube = false;
                }
            }

            canSpawnCube = true;
        }

        glBindVertexArray(VAO);
        for (const auto& cubePos : cubePositions)
        {

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePos);
            ourShader.setMat4("model", model);

            // add code to check if a cube is touching another cube then delete the insides we cant see........
            //if(cubePos.x+cubePos.)

            // code to draw cube
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            // Right mouse button was pressed
            glm::vec3 rayDir = CreateRay(window, projection, view);

            // Check if the ray intersects with any objects
            bool cubeDeleted = false;
            for (unsigned int x = 0; x < 30 && !cubeDeleted; x++)
            {
                for (unsigned int y = 0; y < 10 && !cubeDeleted; y++)
                {
                    for (unsigned int z = 0; z < 30 && !cubeDeleted; z++)
                    {
                        bool intersects = RayIntersectsObject(cameraPos, rayDir, glm::vec3(x, y, z), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);

                        if (intersects) {
                            // The ray intersects with the object
                            // Remove the object from the scene
                            blocks[x][y][z] = false;
                            cubeDeleted = true;
                        }
                    }
                }
            }

            // Check if the ray intersects with any previously spawned cubes
            for (auto it = cubePositions.begin(); it != cubePositions.end() && !cubeDeleted; ++it)
            {
                bool intersects = RayIntersectsObject(cameraPos, rayDir, *it, glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);

                if (intersects)
                {
                    // The ray intersects with the previously spawned cube
                    // Remove the cube from the scene
                    cubePositions.erase(it);
                    cubeDeleted = true;
                }
            }
        }*/
        // Calculate deltaTime
        /*float currentFrame2 = glfwGetTime();
        float deltaTime = currentFrame2 - lastFrame;
        lastFrame = currentFrame;*/
        
        //(deltaTime, cubePositions);

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            // Left mouse button was pressed
            double currentTime = glfwGetTime();
            if (currentTime - lastBlockSpawnTime >= blockSpawnDelay)
            {
                // Enough time has passed since the last block spawn
                glm::vec3 rayDir = CreateRay(window, objectProjection, view);

                // Check if the ray intersects with any objects
                bool blockSpawned = false;
                for (unsigned int x = 0; x < 30 && !blockSpawned; x++)
                {
                    for (unsigned int y = 0; y < 10 && !blockSpawned; y++)
                    {
                        for (unsigned int z = 0; z < 30 && !blockSpawned; z++)
                        {
                            glm::vec3 hitNormal;
                            bool intersects = RayIntersectsObject(cameraPos, rayDir, glm::vec3(x, y, z), glm::vec3(1.0f, 1.0f, 1.0f), 0.10f, &hitNormal);
                            if (intersects) {
                                // The ray intersects with the object
                                // Place a new block on the side of the intersected block
                                Block newBlock;
                                newBlock.position = glm::vec3(x, y, z) + hitNormal;
                                newBlock.texture = block_texture;
                                cubePositions.push_back(newBlock);
                                lastBlockSpawnTime = currentTime;
                                blockSpawned = true;
                            }
                        }
                    }
                }
                // Check if the ray intersects with any previously spawned cubes
                std::vector<Block> cubePositionsCopy = cubePositions;
                for (const auto& cube : cubePositionsCopy)
                {
                    glm::vec3 hitNormal;
                    bool intersects = RayIntersectsObject(cameraPos, rayDir, cube.position, glm::vec3(1.0f, 1.0f, 1.0f), 0.10f, &hitNormal);
                    if (intersects && canSpawnCube)
                    {
                        // The ray intersects with the previously spawned cube
                        // Place a new block on the side of the intersected block
                        Block newBlock;
                        newBlock.position = cube.position + hitNormal;
                        newBlock.texture = block_texture;
                        cubePositions.push_back(newBlock);
                        canSpawnCube = false;
                        blockSpawned = true;
                    }
                }
                canSpawnCube = true;
            }
        }


        glBindVertexArray(VAO);
        for (const auto& cube : cubePositions)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cube.texture);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cube.position);
            ourShader.setMat4("model", model);
            // code to draw cube
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


        /*glBindVertexArray(VAO);
        for (const auto& cube : cubePositions)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cube.texture);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cube.position);
            ourShader.setMat4("model", model);

            // code to draw cube
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }*/


        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            // Right mouse button was pressed
            glm::vec3 rayDir = CreateRay(window, objectProjection, view);

            // Check if the ray intersects with any objects
            bool cubeDeleted = false;
            for (unsigned int x = 0; x < 30 && !cubeDeleted; x++)
            {
                for (unsigned int y = 0; y < 10 && !cubeDeleted; y++)
                {
                    for (unsigned int z = 0; z < 30 && !cubeDeleted; z++)
                    {
                        glm::vec3 hitNormal;
                        bool intersects = RayIntersectsObject(cameraPos, rayDir, glm::vec3(x, y, z), glm::vec3(1.0f, 1.0f, 1.0f), 0.25f, &hitNormal);
                        if (intersects) {
                            // The ray intersects with the object
                            // Remove the object from the scene
                            blocks[x][y][z] = false;
                            cubeDeleted = true;
                        }
                    }
                }
            }

            // Check if the ray intersects with any previously spawned cubes
            for (auto it = cubePositions.begin(); it != cubePositions.end() && !cubeDeleted; ++it)
            {
                glm::vec3 hitNormal;
                bool intersects = RayIntersectsObject(cameraPos, rayDir, it->position, glm::vec3(1.0f, 1.0f, 1.0f), 0.25f, &hitNormal);
                if (intersects)
                {
                    // The ray intersects with the previously spawned cube
                    // Remove the cube from the scene
                    it->position -= hitNormal;
                    it = cubePositions.erase(it);
                    cubeDeleted = true;
                }
            }
        }


        // render boxes
        glBindVertexArray(VAO);
        for (unsigned int x = 0; x < 30; x++)
        {
            for (unsigned int y = 0; y < 10; y++)
            {
                for (unsigned int z = 0; z < 30; z++)
                {
                    if (!blocks[x][y][z]) continue;

                    // loading textures
                    if (y > 7)
                    {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, grasstexture);
                    }
                    else if (y < 7 && y > 5)
                    {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, dirttexture);
                    }
                    else if (y < 5)
                    {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, stonetexture);
                    }

                    // calculate the model matrix for each object and pass it to shader before drawing
                    glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
                    model = glm::translate(model, glm::vec3(x, y, z));
                    
                    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                    //float angle = 20.0f * i;
                    //model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
                    ourShader.setMat4("model", model);
                    
                    // one face has 6 triangles, 6 faces = 36 triangles
                    // check neighboring cubes
                    bool left = x > 0;
                    bool right = x < 29;
                    bool bottom = y > 0;
                    bool top = y < 9;
                    bool front = z > 0;
                    bool back = z < 29;  

                    // draw only visible faces
                    if (!left) glDrawArrays(GL_TRIANGLES, 0, 6); // left face
                    if (!right) glDrawArrays(GL_TRIANGLES, 6, 6); // right face
                    if (!bottom) glDrawArrays(GL_TRIANGLES, 12, 6); // bottom face
                    if (!top) glDrawArrays(GL_TRIANGLES, 18, 6); // top face
                    if (!front) glDrawArrays(GL_TRIANGLES, 24, 6); // front face
                    if (!back) glDrawArrays(GL_TRIANGLES, 30, 6); // back face

                    // Create a ray from the mouse cursor
                    /*glm::vec3 rayDir = CreateRay(window, projection, view);

                    bool intersects = RayIntersectsObject(cameraPos, rayDir, glm::vec3(x, y, z), glm::vec3(1.0f, 1.0f, 1.0f));

                    if (intersects) {
                        // The ray intersects with the object
                        `// ...  
                        //std::cout << "intersect\n";
                    }
                    else {
                        // The ray does not intersect with the object
                        // ...
                    }*/
                }
            }
        }

        /*if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            if (canSpawnCube)
            {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);

                int width, height;
                glfwGetWindowSize(window, &width, &height);

                float z;
                glReadPixels(xpos, height - ypos - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);

                glm::mat4 model = glm::mat4(1.0f);
                glm::vec4 viewport = glm::vec4(0.0f, 0.0f, (float)width, (float)height);
                glm::vec3 screenPos = glm::vec3(xpos, height - ypos - 1, z);
                glm::vec3 worldPos = glm::unProject(screenPos, view * model, projection, viewport);

                std::cout << "Model: " << glm::to_string(model) << "\n";
                std::cout << "Viewport: " << glm::to_string(viewport) << "\n";
                std::cout << "ScreenPosition: " << glm::to_string(screenPos) << "\n";
                std::cout << "WorldPosition: " << glm::to_string(worldPos) << "\n";

                

                // Add the new cube position to the vector
                //cubePositions.push_back(worldPos);

                canSpawnCube = false;
            }
        }
        else
        {
            canSpawnCube = true;
        }
                // Render all cubes
        glBindVertexArray(VAO);
        for (const auto& cubePos : cubePositions)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePos);
            ourShader.setMat4("model", model);

            // add code to check if a cube is touching another cube then delete the insides we cant see........
            //if(cubePos.x+cubePos.)

            // code to draw cube
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }*/

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

glm::vec3 getRayFromMouse(double mouseX, double mouseY, glm::mat4 projectionMatrix, glm::mat4 viewMatrix)
{
    float x = (2.0f * mouseX) / SCR_WIDTH - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / SCR_HEIGHT;
    float z = 1.0f;
    glm::vec3 ray_nds = glm::vec3(x, y, z);
    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
    glm::vec4 ray_eye = glm::inverse(projectionMatrix) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
    glm::vec3 ray_wor = (glm::inverse(viewMatrix) * ray_eye);
    ray_wor = glm::normalize(ray_wor);
    return ray_wor;
}

glm::vec3 getRayPlaneIntersection(glm::vec3 ray_origin, glm::vec3 ray_direction, glm::vec3 plane_normal, glm::vec3 plane_point)
{
    float d = glm::dot((plane_point - ray_origin), plane_normal) / glm::dot(ray_direction, plane_normal);
    return ray_origin + d * ray_direction;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos.y += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        cameraSpeed = 0.20f;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
    {
        cameraSpeed = 0.05f;
    }
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
    {
        // enabling wireframe mode
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
    {
        // disabling wireframe mode
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

unsigned int loadTexture(const char *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// render line of text
// -------------------
void RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state	
    shader.use();
    glUniform4f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z, 0.5f);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float textVertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(textVertices), textVertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}