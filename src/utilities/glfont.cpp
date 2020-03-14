#include <iostream>
#include "glfont.h"

Mesh generateTextGeometryBuffer(std::string text, float characterHeightOverWidth, float totalTextWidth) {
    float characterWidth = totalTextWidth / float(text.length());
    float characterHeight = characterHeightOverWidth * characterWidth;

    unsigned int vertexCount = 4 * text.length();
    unsigned int indexCount = 6 * text.length();

    Mesh mesh;

    mesh.vertices.resize(vertexCount);
    mesh.indices.resize(indexCount);
    mesh.textureCoordinates.resize(indexCount);

    for(unsigned int i = 0; i < text.length(); i++)
    {
        float baseXCoordinate = float(i) * characterWidth;

        // texture coordiantes
        float textXMin = float(text.at(i))/128.0;
        float textXMax = float(text.at(i)+1)/128.0;
        float textYMin = 0.0;
        float textYMax = 1.0;


        mesh.vertices.at(4 * i + 0) = {baseXCoordinate, 0, 0};
        mesh.vertices.at(4 * i + 1) = {baseXCoordinate + characterWidth, 0, 0};
        mesh.vertices.at(4 * i + 2) = {baseXCoordinate + characterWidth, characterHeight, 0};

        mesh.vertices.at(4 * i + 0) = {baseXCoordinate, 0, 0};
        mesh.vertices.at(4 * i + 2) = {baseXCoordinate + characterWidth, characterHeight, 0};
        mesh.vertices.at(4 * i + 3) = {baseXCoordinate, characterHeight, 0};


        mesh.indices.at(6 * i + 0) = 4 * i + 0;
        mesh.indices.at(6 * i + 1) = 4 * i + 1;
        mesh.indices.at(6 * i + 2) = 4 * i + 2;
        mesh.indices.at(6 * i + 3) = 4 * i + 0;
        mesh.indices.at(6 * i + 4) = 4 * i + 2;
        mesh.indices.at(6 * i + 5) = 4 * i + 3;

        mesh.textureCoordinates.at(6 * i + 0) = {textXMin, textYMin};
        mesh.textureCoordinates.at(6 * i + 1) = {textXMax, textYMin};
        mesh.textureCoordinates.at(6 * i + 2) = {textXMax, textYMax};
        mesh.textureCoordinates.at(6 * i + 3) = {textXMin, textYMin};
        mesh.textureCoordinates.at(6 * i + 4) = {textXMax, textYMax};
        mesh.textureCoordinates.at(6 * i + 5) = {textYMin, textYMax};
    }

    return mesh;
}
