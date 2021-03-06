#include <chrono>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <SFML/Audio/SoundBuffer.hpp>
#include <utilities/shader.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <utilities/timeutils.h>
#include <utilities/mesh.h>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <SFML/Audio/Sound.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fmt/format.h>
#include "gamelogic.h"
#include "sceneGraph.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <string.h>

#include "utilities/imageLoader.hpp"
#include "utilities/glfont.h"

enum KeyFrameAction{
    BOTTOM, TOP
};

#include <timestamps.h>

// Load textures
PNGImage charmapPNG = loadPNGFile("../res/textures/charmap.png");
PNGImage diffuseWallPNG = loadPNGFile("../res/textures/Brick03_col.png");
PNGImage normalWallPNG = loadPNGFile("../res/textures/Brick03_nrm.png");

// character height and width
const float char_width  = 29.0;
const float char_height = 39.0;

double padPositionX = 0;
double padPositionZ = 0;

unsigned int currentKeyFrame  = 0;
unsigned int previousKeyFrame = 0;

// Geomertry
SceneNode* rootNode;
SceneNode* boxNode;
SceneNode* ballNode;
SceneNode* padNode;

// Text
SceneNode* textNode;

// Lights
SceneNode* rootLightNode;
SceneNode* padLightNode;
SceneNode* ballLightNode;

double ballRadius = 3.0f;

// These are heap allocated, because they should not be initialised at the start of the program
sf::SoundBuffer* buffer;
Gloom::Shader* shader;
Gloom::Shader* _2DShader;
sf::Sound* sound;

const glm::vec3 boxDimensions(180, 90, 90);
const glm::vec3 padDimensions(30, 3, 40);

glm::vec3 ballPosition(0, ballRadius + padDimensions.y, boxDimensions.z / 2);
glm::vec3 ballDirection(1, 1, 0.2f);

CommandLineOptions options;

bool hasStarted = false;
bool hasLost = false;
bool jumpedToNextFrame = false;
bool isPaused = false;

bool mouseLeftPressed   = false;
bool mouseLeftReleased  = false;
bool mouseRightPressed  = false;
bool mouseRightReleased = false;

// Modify if you want the music to start further on in the track. Measured in seconds.
const float debug_startTime = 0;
double totalElapsedTime = debug_startTime;
double gameElapsedTime  = debug_startTime;

double mouseSensitivity = 1.0;
double lastMouseX = windowWidth / 2;
double lastMouseY = windowHeight / 2;
void mouseCallback(GLFWwindow* window, double x, double y) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    double deltaX = x - lastMouseX;
    double deltaY = y - lastMouseY;

    padPositionX -= mouseSensitivity * deltaX / windowWidth;
    padPositionZ -= mouseSensitivity * deltaY / windowHeight;

    if (padPositionX > 1) padPositionX = 1;
    if (padPositionX < 0) padPositionX = 0;
    if (padPositionZ > 1) padPositionZ = 1;
    if (padPositionZ < 0) padPositionZ = 0;

    glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
}

//// A few lines to help you if you've never used c++ structs
struct LightSource {

    glm::vec4 position;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;

    LightSource(){
        position  = glm::vec4(glm::vec3(0.0f), 1.0f);
        // initialized as white light
        ambient   = glm::vec3(0.05f, 0.05f, 0.05f);
        diffuse   = glm::vec3(1.0f);
        specular  = glm::vec3(1.0f);
        constant = 1.0f;
        linear = 0.0022f;
        quadratic = 0.00019f;
    }
};

const unsigned int NR_OF_LIGHTS = 3;
LightSource lightSources[NR_OF_LIGHTS];

void initGame(GLFWwindow* window, CommandLineOptions gameOptions) {
    buffer = new sf::SoundBuffer();
    if (!buffer->loadFromFile("../res/Hall of the Mountain King.ogg")) {
        return;
    }

    options = gameOptions;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouseCallback);

    shader = new Gloom::Shader();
    _2DShader = new Gloom::Shader();
    shader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");
    _2DShader->makeBasicShader("../res/shaders/simple2d.vert", "../res/shaders/simple2d.frag");
    shader->activate();

    // Create meshes
    Mesh pad = cube(padDimensions, glm::vec2(30, 40), true);
    Mesh box = cube(boxDimensions, glm::vec2(90), true, true);
    Mesh sphere = generateSphere(1.0, 40, 40);

    // texture mesh
    std::string textToRender = "Hello World!";
    Mesh text = generateTextGeometryBuffer(textToRender, char_height/char_width, textToRender.length()*char_width);

    // Fill buffers
    unsigned int ballVAO = generateBuffer(sphere);
    unsigned int boxVAO  = generateBuffer(box);
    unsigned int padVAO  = generateBuffer(pad);
    unsigned int textVAO = generateBuffer(text);

    // Create texture buffers
    unsigned int charmapTextureID = generateTextureID(charmapPNG);
    unsigned int normalWallTextureID = generateTextureID(normalWallPNG);
    unsigned int diffuseWallTextureID = generateTextureID(diffuseWallPNG);

    // Construct scene
    rootNode  = createSceneNode();
    boxNode   = createSceneNode();
    padNode   = createSceneNode();
    ballNode  = createSceneNode();
    textNode  = createSceneNode();

    // Creating lightsourceNodes
    rootLightNode = createSceneNode();
    padLightNode  = createSceneNode();
    ballLightNode = createSceneNode();

    // RootLightNode position.
    rootLightNode->position = boxDimensions/3.0f*glm::vec3(1.0f, -1.0f, -0.5f);
    // PadLightNode position
    padLightNode ->position = glm::vec3(0.0f, ballRadius, 0.0f);
    // BallLightNode position
    ballLightNode->position = (boxDimensions/3.0f)*glm::vec3(-1.0f, -1.0f, -0.5f);

    // Creating a 2D geometry node;
    textNode->nodeType = _2D_GEOMETRY;

    // Setting the box node as normal mapped
    boxNode->nodeType = NORMAL_MAPPED_GEOMETRY;

    // Setting lightsources as point lights
    rootLightNode->nodeType = POINT_LIGHT;
    padLightNode ->nodeType = POINT_LIGHT;
    ballLightNode->nodeType = POINT_LIGHT;

    // Attaching light sources
    rootLightNode->lightSourceID = 0;
    padLightNode ->lightSourceID = 1;
    ballLightNode->lightSourceID = 2;

    // Setting colours for lightSources
    lightSources[rootLightNode->lightSourceID].diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    lightSources[rootLightNode->lightSourceID].specular = glm::vec3(1.0f, 1.0f, 1.0f);

    lightSources[padLightNode->lightSourceID].diffuse = glm::vec3(0.5f, 0.5f, 0.25f);
    lightSources[padLightNode->lightSourceID].specular = glm::vec3(0.5f, 0.5f, 0.25f);

    lightSources[ballLightNode->lightSourceID].diffuse = glm::vec3(0.0f, 0.25f, 0.75f);
    lightSources[ballLightNode->lightSourceID].specular = glm::vec3(0.0f, 0.25f, 0.75f);

    // Initializing scene graph
    rootNode->children.push_back(boxNode);
    rootNode->children.push_back(padNode);
    rootNode->children.push_back(ballNode);
    rootNode->children.push_back(textNode);

    // attach Light sources to the desired scene nodes.
    rootNode->children.push_back(rootLightNode);
    padNode ->children.push_back(padLightNode);
    rootNode->children.push_back(ballLightNode);

    // assigning IDs to the nodes
    boxNode->vertexArrayObjectID = boxVAO;
    boxNode->VAOIndexCount = box.indices.size();
    boxNode->normalMappedGeometryID = normalWallTextureID;
    boxNode->textureID = diffuseWallTextureID;

    padNode->vertexArrayObjectID = padVAO;
    padNode->VAOIndexCount = pad.indices.size();

    ballNode->vertexArrayObjectID = ballVAO;
    ballNode->VAOIndexCount = sphere.indices.size();

    textNode->vertexArrayObjectID = textVAO;
    textNode->VAOIndexCount = text.indices.size();
    textNode->textureID = charmapTextureID;

    getTimeDeltaSeconds();

    std::cout << fmt::format("Initialized scene with {} SceneNodes.", totalChildren(rootNode)) << std::endl;

    std::cout << "Ready. Click to start!" << std::endl;
}

void updateFrame(GLFWwindow* window) {
    shader -> activate();
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    double timeDelta = getTimeDeltaSeconds();

    const float ballBottomY = boxNode->position.y - (boxDimensions.y/2) + ballRadius + padDimensions.y;
    const float ballTopY    = boxNode->position.y + (boxDimensions.y/2) - ballRadius;
    const float BallVerticalTravelDistance = ballTopY - ballBottomY;

    const float cameraWallOffset = 30; // Arbitrary addition to prevent ball from going too much into camera

    const float ballMinX = boxNode->position.x - (boxDimensions.x/2) + ballRadius;
    const float ballMaxX = boxNode->position.x + (boxDimensions.x/2) - ballRadius;
    const float ballMinZ = boxNode->position.z - (boxDimensions.z/2) + ballRadius;
    const float ballMaxZ = boxNode->position.z + (boxDimensions.z/2) - ballRadius - cameraWallOffset;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
        mouseLeftPressed  = true;
        mouseLeftReleased = false;
    } else {
        mouseLeftReleased = mouseLeftPressed;
        mouseLeftPressed  = false;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
        mouseRightPressed  = true;
        mouseRightReleased = false;
    } else {
        mouseRightReleased = mouseRightPressed;
        mouseRightPressed  = false;
    }

    if(!hasStarted) {
        if (mouseLeftPressed) {
            if (options.enableMusic) {
                sound = new sf::Sound();
                sound->setBuffer(*buffer);
                sf::Time startTime = sf::seconds(debug_startTime);
                sound->setPlayingOffset(startTime);
                sound->play();
            }
            totalElapsedTime = debug_startTime;
            gameElapsedTime = debug_startTime;
            hasStarted = true;
        }

        ballPosition.x = ballMinX + (1 - padPositionX) * (ballMaxX - ballMinX);
        ballPosition.y = ballBottomY;
        ballPosition.z = ballMinZ + (1 - padPositionZ) * ((ballMaxZ+cameraWallOffset) - ballMinZ);
    } else {
        totalElapsedTime += timeDelta;
        if(hasLost) {
            if (mouseLeftReleased) {
                hasLost = false;
                hasStarted = false;
                currentKeyFrame = 0;
                previousKeyFrame = 0;
            }
        } else if (isPaused) {
            if (mouseRightReleased) {
                isPaused = false;
                if (options.enableMusic) {
                    sound->play();
                }
            }
        } else {
            gameElapsedTime += timeDelta;
            if (mouseRightReleased) {
                isPaused = true;
                if (options.enableMusic) {
                    sound->pause();
                }
            }
            // Get the timing for the beat of the song
            for (unsigned int i = currentKeyFrame; i < keyFrameTimeStamps.size(); i++) {
                if (gameElapsedTime < keyFrameTimeStamps.at(i)) {
                    continue;
                }
                currentKeyFrame = i;
            }

            jumpedToNextFrame = currentKeyFrame != previousKeyFrame;
            previousKeyFrame = currentKeyFrame;

            double frameStart = keyFrameTimeStamps.at(currentKeyFrame);
            double frameEnd = keyFrameTimeStamps.at(currentKeyFrame + 1); // Assumes last keyframe at infinity

            double elapsedTimeInFrame = gameElapsedTime - frameStart;
            double frameDuration = frameEnd - frameStart;
            double fractionFrameComplete = elapsedTimeInFrame / frameDuration;

            double ballYCoord;

            KeyFrameAction currentOrigin = keyFrameDirections.at(currentKeyFrame);
            KeyFrameAction currentDestination = keyFrameDirections.at(currentKeyFrame + 1);

            // Synchronize ball with music
            if (currentOrigin == BOTTOM && currentDestination == BOTTOM) {
                ballYCoord = ballBottomY;
            } else if (currentOrigin == TOP && currentDestination == TOP) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance;
            } else if (currentDestination == BOTTOM) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance * (1 - fractionFrameComplete);
            } else if (currentDestination == TOP) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance * fractionFrameComplete;
            }

            // Make ball move
            const float ballSpeed = 60.0f;
            ballPosition.x += timeDelta * ballSpeed * ballDirection.x;
            ballPosition.y = ballYCoord;
            ballPosition.z += timeDelta * ballSpeed * ballDirection.z;

            // Make ball bounce
            if (ballPosition.x < ballMinX) {
                ballPosition.x = ballMinX;
                ballDirection.x *= -1;
            } else if (ballPosition.x > ballMaxX) {
                ballPosition.x = ballMaxX;
                ballDirection.x *= -1;
            }
            if (ballPosition.z < ballMinZ) {
                ballPosition.z = ballMinZ;
                ballDirection.z *= -1;
            } else if (ballPosition.z > ballMaxZ) {
                ballPosition.z = ballMaxZ;
                ballDirection.z *= -1;
            }

            if(options.enableAutoplay) {
                padPositionX = 1-(ballPosition.x - ballMinX) / (ballMaxX - ballMinX);
                padPositionZ = 1-(ballPosition.z - ballMinZ) / ((ballMaxZ+cameraWallOffset) - ballMinZ);
            }

            // Check if the ball is hitting the pad when the ball is at the bottom.
            // If not, you just lost the game! (hehe)
            if (jumpedToNextFrame && currentOrigin == BOTTOM && currentDestination == TOP) {
                double padLeftX  = boxNode->position.x - (boxDimensions.x/2) + (1 - padPositionX) * (boxDimensions.x - padDimensions.x);
                double padRightX = padLeftX + padDimensions.x;
                double padFrontZ = boxNode->position.z - (boxDimensions.z/2) + (1 - padPositionZ) * (boxDimensions.z - padDimensions.z);
                double padBackZ  = padFrontZ + padDimensions.z;

                if (   ballPosition.x < padLeftX
                    || ballPosition.x > padRightX
                    || ballPosition.z < padFrontZ
                    || ballPosition.z > padBackZ) {
                    hasLost = true;
                    if (options.enableMusic) {
                        sound->stop();
                        delete sound;
                    }
                }
            }
        }
    }

    glm::mat4 projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);
    glm::vec3 cameraPosition = glm::vec3(0, 2, -20);

    // Some math to make the camera move in a nice way
    float lookRotation = -0.6 / (1 + exp(-5 * (padPositionX-0.5))) + 0.3;
    glm::mat4 cameraTransform =
                    glm::rotate(0.3f + 0.2f * float(-padPositionZ*padPositionZ), glm::vec3(1, 0, 0)) *
                    glm::rotate(lookRotation, glm::vec3(0, 1, 0)) *
                    glm::translate(-cameraPosition);

    glm::mat4 VP = projection * cameraTransform;

    // Move and rotate various SceneNodes
    boxNode->position = { 0, -10, -80 };

    ballNode->position = ballPosition;
    ballNode->scale = glm::vec3(ballRadius);
    ballNode->rotation = { 0, totalElapsedTime*2, 0 };

    padNode->position  = {
        boxNode->position.x - (boxDimensions.x/2) + (padDimensions.x/2) + (1 - padPositionX) * (boxDimensions.x - padDimensions.x),
        boxNode->position.y - (boxDimensions.y/2) + (padDimensions.y/2),
        boxNode->position.z - (boxDimensions.z/2) + (padDimensions.z/2) + (1 - padPositionZ) * (boxDimensions.z - padDimensions.z)
    };

    glUniform3fv(13, 1, glm::value_ptr(cameraPosition));
    glUniform3fv(14, 1, glm::value_ptr(ballNode->position));
    updateNodeTransformations(rootNode, VP, glm::mat4(1.0f));
}


void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar, glm::mat4 parentModelMatrix) {
    glm::mat4 transformationMatrix =
              glm::translate(node->position)
            * glm::translate(node->referencePoint)
            * glm::rotate(node->rotation.y, glm::vec3(0,1,0))
            * glm::rotate(node->rotation.x, glm::vec3(1,0,0))
            * glm::rotate(node->rotation.z, glm::vec3(0,0,1))
            * glm::scale(node->scale)
            * glm::translate(-node->referencePoint);

    node->currentTransformationMatrix = transformationThusFar * transformationMatrix;
    node->currentModelMatrix = parentModelMatrix * transformationMatrix;
    node->normalMatrix = glm::mat3(glm::transpose(glm::inverse(parentModelMatrix*transformationMatrix)));


    switch(node->nodeType) {
        case GEOMETRY: break;

        case _2D_GEOMETRY:
            if(node->textureID != -1){}
            break;

        case NORMAL_MAPPED_GEOMETRY:
            if(node->textureID != -1){}
            break;

        case POINT_LIGHT:

            if(node->lightSourceID != -1)
            {
                // Getting the lightsource ID;
                lightSources[node->lightSourceID].position = (node->currentModelMatrix)*glm::vec4(glm::vec3(0.0f), 1.0f);

                // getting ID from shader
                GLint posLocation = shader->getUniformFromName(fmt::format("lightSources[{}].position" , node->lightSourceID));
                GLint ambLocation = shader->getUniformFromName(fmt::format("lightSources[{}].ambient"  , node->lightSourceID));
                GLint difLocation = shader->getUniformFromName(fmt::format("lightSources[{}].diffuse"  , node->lightSourceID));
                GLint speLocation = shader->getUniformFromName(fmt::format("lightSources[{}].specular" , node->lightSourceID));
                GLint conLocation = shader->getUniformFromName(fmt::format("lightSources[{}].constant" , node->lightSourceID));
                GLint linLocation = shader->getUniformFromName(fmt::format("lightSources[{}].linear"   , node->lightSourceID));
                GLint quaLocation = shader->getUniformFromName(fmt::format("lightSources[{}].quadratic", node->lightSourceID));
                // sending to shader
                glUniform4fv(posLocation, 1, glm::value_ptr(lightSources[node->lightSourceID].position));
                glUniform3fv(ambLocation, 1, glm::value_ptr(lightSources[node->lightSourceID].ambient));
                glUniform3fv(difLocation, 1, glm::value_ptr(lightSources[node->lightSourceID].diffuse));
                glUniform3fv(speLocation, 1, glm::value_ptr(lightSources[node->lightSourceID].specular));
                glUniform1f(conLocation, lightSources[node->lightSourceID].constant);
                glUniform1f(linLocation, lightSources[node->lightSourceID].linear);
                glUniform1f(quaLocation, lightSources[node->lightSourceID].quadratic);
            }
            break;
        case SPOT_LIGHT: break;
    }

    for(SceneNode* child : node->children) {
        updateNodeTransformations(child, node->currentTransformationMatrix, node->currentModelMatrix);
    }
}

void renderNode(SceneNode* node) {

    switch(node->nodeType) {
        case GEOMETRY:
            if(node->vertexArrayObjectID != -1)
            {
                glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));
                glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(node->currentModelMatrix));
                glUniformMatrix3fv(5, 1, GL_FALSE, glm::value_ptr(node->normalMatrix));
                // toggle normal mapped geomerty
                glUniform1i(15, GL_FALSE);

                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;

        case _2D_GEOMETRY:
            if(node->textureID != -1 && node->vertexArrayObjectID != -1)
            {
                glm::mat4 OP = glm::ortho(0.0f, 1336.0f, 0.0f, 768.0f);
                _2DShader -> activate();

                glUniformMatrix4fv(20, 1, GL_FALSE, glm::value_ptr(OP));

                glBindVertexArray(node->vertexArrayObjectID);
                glBindTextureUnit(0, node -> textureID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;

        case NORMAL_MAPPED_GEOMETRY:
            if(node->vertexArrayObjectID != -1)
            {
                glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));
                glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(node->currentModelMatrix));
                glUniformMatrix3fv(5, 1, GL_FALSE, glm::value_ptr(node->normalMatrix));

                // toggle normal mapped geometry
                glUniform1i(15, GL_TRUE);

                glBindVertexArray(node->vertexArrayObjectID);
                glBindTextureUnit(0, node -> textureID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;

        case POINT_LIGHT:
            break;

        case SPOT_LIGHT: break;
    }

    for(SceneNode* child : node->children) {
        renderNode(child);
    }
}

void renderFrame(GLFWwindow* window) {

    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    renderNode(rootNode);
}
