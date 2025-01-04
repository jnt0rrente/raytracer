#include "renderTarget.h"
#include <random>
#include <sstream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <fstream>
#include <png.h>

#ifdef __EMSCRIPTEN__
EM_JS(void, download_image_js, (const char *format, const uint8_t *data, size_t size), {
    console.log('Downloading image...');
    var data = new Uint8Array(HEAPU8.buffer, data, size);
    var blob = new Blob([data], { type: 'application/octet-stream' });
    var url = URL.createObjectURL(blob);
    var a = document.createElement('a');
    a.href = url;
    a.download = 'image.' + UTF8ToString(format);
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
});
#endif

RenderTarget::RenderTarget(std::vector<Color> pixels, int width, int height)
    : width(width), height(height), pixels(pixels)
{
    std::stringstream ss;
    std::random_device rd;
    ss << std::hex << rd();
    identifier = ss.str();
}

RenderTarget::~RenderTarget() {}

void RenderTarget::setPixel(int x, int y, Color color)
{
    if (x >= 0 && x < width && y >= 0 && y < height)
    {
        pixels[y * width + x] = color;
    }
}

Color RenderTarget::getPixel(int x, int y) const
{
    if (x >= 0 && x < width && y >= 0 && y < height)
    {
        return pixels[y * width + x];
    }
    return Color(); // Return a default color if out of bounds
}

int RenderTarget::getWidth() const
{
    return width;
}

int RenderTarget::getHeight() const
{
    return height;
}

std::vector<Color> RenderTarget::getPixels() const
{
    return pixels;
}

std::string RenderTarget::getIdentifier() const
{
    return identifier;
}



#pragma region images
void RenderTarget::save_image(const std::string &format)
{
    std::clog << "Saving image..." << std::endl;

    std::string path = "/tmp/image_" + std::to_string(rand()) + "." + format;

    if (format == "png")
    {
        std::vector<uint8_t> imageData;
        imageData = save_png_to_memory();

        #ifdef __EMSCRIPTEN__
        download_image_js(format.c_str(), imageData.data(), imageData.size());
        #else
        save_png_to_file(path);
        #endif
    }
    else
    {
        std::cerr << "Unsupported image format: " << format << std::endl;
    }

    std::clog << "Image saved to " << path << std::endl;
}



std::vector<uint8_t> RenderTarget::save_png_to_memory()
{
    std::vector<uint8_t> imageData;
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);

    if (!png || !info)
    {
        std::cerr << "Could not initialize PNG write struct." << std::endl;
        return imageData;
    }

    png_set_IHDR(
        png,
        info,
        width,
        height,
        8,
        PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);

    png_byte **row_pointers = new png_byte *[height];
    for (int y = 0; y < height; y++)
    {
        row_pointers[y] = new png_byte[3 * width];
        for (int x = 0; x < width; x++)
        {
            int i = y * width + x;
            Color pixel = pixels[i];
            row_pointers[y][x * 3] = static_cast<uint8_t>(255.99 * pixel.x());
            row_pointers[y][x * 3 + 1] = static_cast<uint8_t>(255.99 * pixel.y());
            row_pointers[y][x * 3 + 2] = static_cast<uint8_t>(255.99 * pixel.z());
        }
    }

    png_set_rows(png, info, row_pointers);

    png_set_write_fn(png, &imageData, [](png_structp png, png_bytep data, png_size_t length) {
        std::vector<uint8_t> *p = static_cast<std::vector<uint8_t> *>(png_get_io_ptr(png));
        p->insert(p->end(), data, data + length);
    }, nullptr);

    png_write_png(png, info, PNG_TRANSFORM_IDENTITY, nullptr);

    for (int y = 0; y < height; y++)
    {
        delete[] row_pointers[y];
    }
    delete[] row_pointers;

    png_destroy_write_struct(&png, &info);

    return imageData;
}

void RenderTarget::save_png_to_file(const std::string &filename)
{
    FILE *fp = fopen(filename.c_str(), "wb");

    if (!fp)
    {
        std::cerr << "Could not open file for writing: " << filename << std::endl;
        return;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png)
    {
        std::cerr << "Could not create write struct." << std::endl;
        fclose(fp);
        return;
    }

    png_infop info = png_create_info_struct(png);
    if (!info)
    {
        std::cerr << "Could not create info struct." << std::endl;
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        return;
    }

    if (setjmp(png_jmpbuf(png)))
    {
        std::cerr << "Error during png creation." << std::endl;
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        return;
    }

    png_init_io(png, fp);

    png_set_IHDR(
        png,
        info,
        width,
        height,
        8,
        PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png, info);

    png_bytep row = (png_bytep) malloc(3 * width * sizeof(png_byte));
    if (!row)
    {
        std::cerr << "Could not allocate memory for row." << std::endl;
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        return;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int i = y * width + x;
            if (i >= (int)pixels.size()) {
                std::cerr << "Index out of bounds: " << i << std::endl;
                break;
            }
            Color pixel = pixels[i];
            row[x*3] = (unsigned char)(255.99 * pixel.x());
            row[x*3 + 1] = (unsigned char)(255.99 * pixel.y());
            row[x*3 + 2] = (unsigned char)(255.99 * pixel.z());
        }
        png_write_row(png, row);
    }

    free(row);
    png_write_end(png, NULL);
    fclose(fp);

    png_destroy_write_struct(&png, &info);
}

#pragma endregion