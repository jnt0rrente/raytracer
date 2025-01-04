#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "color.h"
#include <stdint.h>
#include <vector>

class RenderTarget //has a vector of Color, width and height
{
public :
    RenderTarget(std::vector<Color> pixels, int width, int height);
    ~RenderTarget();
    void setPixel(int x, int y, Color color);
    Color getPixel(int x, int y) const;
    int getWidth() const;
    int getHeight() const;
    std::vector<Color> getPixels() const;
    std::string getIdentifier() const;

    void save_image(const std::string &format);
    void save_png_to_file(const std::string &filename);
    std::vector<uint8_t> save_png_to_memory();

private:
    std::vector<Color> pixels;
    int width;
    int height;
    std::string identifier;
};

#endif