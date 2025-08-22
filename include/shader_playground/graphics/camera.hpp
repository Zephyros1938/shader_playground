#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <opencv2/opencv.hpp>
#include <glad/glad.h>

class Camera
{
public:
    cv::Mat Frame;
    cv::VideoCapture cap;
    GLuint textureID;
    int width, height = 0;

    Camera(int cameraNumber, int width = 640, int height = 480)
    {
        cap = cv::VideoCapture(cameraNumber, cv::CAP_V4L2);
        int camWidth = width == 0 ? static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH)) : width;
        int camHeight = height == 0 ? static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT)) : height;
        std::cout <<
            "Found camera with resolution <" <<
            static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH)) <<
            "x" <<
            static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT)) <<
            ">!" <<
            std::endl;

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Allocate texture memory

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, camWidth, camHeight, 0,
                     GL_BGR, GL_UNSIGNED_BYTE, nullptr);
    }

    void updateTexture()
    {
        cap >> Frame;
        if (!cap.read(Frame) || Frame.empty())
            return;

        cv::flip(Frame, Frame, 0); // Flip vertically for OpenGL

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                        Frame.cols, Frame.rows,
                        GL_BGR, GL_UNSIGNED_BYTE, Frame.data);
    }

    void bind(GLenum textureUnit = GL_TEXTURE0) {
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }
};

#endif
