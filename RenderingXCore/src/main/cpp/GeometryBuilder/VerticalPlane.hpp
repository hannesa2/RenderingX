//
// Created by geier on 23/04/2020.
//

#ifndef CONSTANTIN_RENDERINGX_CORE_VERTICAL_PLANE
#define CONSTANTIN_RENDERINGX_CORE_VERTICAL_PLANE

#include "GLProgramTexture.h"
#include "VerticalPlane.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace VerticalPlane{
    /**
     * By default this creates a vertical rectangle where size=1.0f and origin==(0,0,0)
     * The vertices need to be drawn using a proper index buffer (See createIndicesPlane / createIndicesWireframe)
     * Optionally use @param modelMatrix to place the plane in 3D space and use
     * @param textureMatrix to scale and translate the texture (u,v) coordinates
     */
    static std::vector<GLProgramTexture::Vertex> createVerticesPlaneTextured(const unsigned int tessellation,
            const glm::mat4 modelMatrix= glm::mat4(1.0f),const glm::mat4 textureMatrix= glm::mat4(1.0f)){
        std::vector<GLProgramTexture::Vertex> vertices((tessellation+1)*(tessellation+1));
        const int tessellationX=tessellation;
        const int tessellationY=tessellation;
        const float subW=1.0f/(float)tessellationX;
        const float subH=1.0f/(float)tessellationY;
        float subURange=1.0f/(float)tessellationX;
        float subVRange=1.0f/(float)tessellationY;
        unsigned int count=0;
        for(int i=0;i<tessellationX+1;i++){
            for(int j=0;j<tessellationY+1;j++){
                const glm::vec4 pos={-0.5f+subW*j,-0.5f+subH*i,0.0f,1.0f};
                const glm::vec4 texture={subURange*j,subVRange*i,0.0f,1.0f};
                const glm::vec4 pos2=modelMatrix*pos;
                const glm::vec4 texture2=textureMatrix*texture;
                GLProgramTexture::Vertex& v=vertices.at(count);
                v.x=pos2.x;
                v.y=pos2.y;
                v.z=pos2.z;
                v.u=texture2.x;
                v.v=texture2.y;
                count++;
            }
        }
        return vertices;
    }

    //Creates the vertices forming a Grid with
    //N row(s)==tessellation and N column(s)==tessellation. Example:
    //Tessellation 0 : undefined
    //Tessellation 1 : [ ]
    //Tessellation 2 : [ | ]
    //Tessellation 3 : [ | | ]
    static std::vector<GLProgramVC::Vertex> createVerticesPlaneColored(const unsigned int tessellation,
            const glm::mat4 modelMatrix= glm::mat4(1.0f),const TrueColor color= Color::BLACK){
        std::vector<GLProgramVC::Vertex> vertices((tessellation+1)*(tessellation+1));
        const int tessellationX=tessellation;
        const int tessellationY=tessellation;
        const float subW=1.0f/(float)tessellationX;
        const float subH=1.0f/(float)tessellationY;
        unsigned int count=0;
        for(int i=0;i<tessellationX+1;i++){
            for(int j=0;j<tessellationY+1;j++){
                const glm::vec4 pos={-0.5f+subW*j,-0.5f+subH*i,0.0f,1.0f};
                const glm::vec4 pos2=modelMatrix*pos;
                GLProgramVC::Vertex& v=vertices.at(count);
                v.x=pos2.x;
                v.y=pos2.y;
                v.z=pos2.z;
                v.colorRGBA=color;
                count++;
            }
        }
        return vertices;
    }

    //create indices that can be used to draw a grid generated by createVerticesPlaneColored()
    //GL_TRIANGLES render a solid plane
    static std::vector<unsigned int> createIndicesPlane(const unsigned int tessellation){
        const int indicesX=tessellation;
        const int indicesY=tessellation;
        const int rowSize=tessellation+1;
        unsigned int count=0;
        std::vector<unsigned int> indices(6*tessellation*tessellation);
        for(int i=0;i<indicesX;i++){
            for(int j=0;j<indicesY;j++){
                indices.at(count++)=(GLushort)(i*rowSize+j);
                indices.at(count++)=(GLushort)((i+1)*rowSize+j);
                indices.at(count++)=(GLushort)(i*rowSize+j+1);
                indices.at(count++)=(GLushort)(i*rowSize+j+1);
                indices.at(count++)=(GLushort)((i+1)*rowSize+j);
                indices.at(count++)=(GLushort)((i+1)*rowSize+j+1);
            }
        }
        return indices;
    }
    //create indices that can be used to draw a grid generated by createVerticesPlane()
    //GL_LINES creating a wireframe grid
    static std::vector<unsigned int> createIndicesWireframe(const unsigned int tessellation){
        //Create the indices forming vertical lines
        const unsigned int tessellationPlus1=tessellation+1;
        std::vector<unsigned int> indices;
        for(int i=0;i<tessellationPlus1;i++){
            const unsigned int begin=i*tessellationPlus1;
            for(int j=0;j<tessellation;j++){
                indices.push_back(begin+j);
                indices.push_back(begin+j+1);
            }
        }
        //Create the indices forming horizontal lines
        for(int i=0;i<tessellationPlus1;i++){
            const auto begin=i;
            for(int j=0;j<tessellation;j++){
                indices.push_back(begin+j*tessellationPlus1);
                indices.push_back(begin+(j+1)*tessellationPlus1);
            }
        }
        return indices;
    }

    // Reduced / simplified some parameters for FPV_VR, which does not rotate the plane (UI) and only modifies texture u values
    static std::vector<GLProgramTexture::Vertex> createVerticesPlaneTextured(const unsigned int tessellation, const glm::vec3& translation,const glm::vec2& scale,
                                                                             const float uOffset,const float uRange){
        auto modelMatrix=glm::translate(translation)*glm::scale(glm::vec3(scale.x,scale.y,1));
        auto textureMatrix=glm::translate(glm::vec3(uOffset,0,0))*glm::scale(glm::vec3(uRange,1,1));
        return VerticalPlane::createVerticesPlaneTextured(tessellation, modelMatrix, textureMatrix);
    }
    // Reduced / simplified some parameters for FPV_VR, which does not rotate the plane (UI)
    static std::vector<GLProgramVC::Vertex> createVerticesPlaneColored(const unsigned int tessellation,
                                                                       const glm::vec3& translation,const glm::vec2& scale,const TrueColor color){
        auto modelMatrix=glm::translate(translation)*glm::scale(glm::vec3(scale.x,scale.y,1));
        return VerticalPlane::createVerticesPlaneColored(tessellation, modelMatrix,color);
    }
}

#endif