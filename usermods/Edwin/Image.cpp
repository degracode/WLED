#include "Image.h"

#include "wled.h"

void Image::blit(Segment& segment, int destX, int destY) const {
    for (int y = 0; y < _height; ++y)
        for (int x = 0; x < _width; ++x)
            segment.setPixelColorXY(x + destX, y + destY, getPixel(x,y));
}

void Image::blitRegion(Segment& segment, int x, int y, int width, int height) const {
    blitRegion(segment, x, y, width, height, x, y);
}

void Image::blitRegion(Segment& segment, int srcX, int srcY, int width, int height, int destX, int destY) const {
    blitRegion(segment, srcX, srcY, width, height, destX, destY, width, height, 0, 0);
}

void Image::blitRegion(Segment& segment, int srcX, int srcY, int srcWidth, int srcHeight, int destX, int destY, int destWidth, int destHeight, int srcXOffset, int srcYOffset) const {
    for (int i = 0; i < destWidth; ++i) {
        int sx = srcX + ((i + srcXOffset) % srcWidth);

        for (int j = 0; j < destHeight; ++j) {
            int sy = srcY + ((j + srcYOffset) % srcHeight);

            uint32_t colour = getPixel(sx, sy);

            int dx = destX + i;
            int dy = destY + j;
            segment.setPixelColorXY(dx, dy, colour);
        }
    }
}