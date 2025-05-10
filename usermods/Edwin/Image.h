#pragma once

#include <cstdint>

struct Segment;

class Image {
public:
    Image(int width, int height, const uint32_t* rgbwData)
    : _width(width)
    , _height(height)
    , _rgbwData(rgbwData)
    {}

    uint32_t getPixel(int x, int y) const { return _rgbwData[x + (y * _width)]; }
    int getWidth() const { return _width; }
    int getHeight() const { return _height; }

    void blit(Segment& segment, int destX, int destY) const;
    void blitRegion(Segment& segment, int x, int y, int width, int height) const;
    void blitRegion(Segment& segment, int srcX, int srcY, int width, int height, int destX, int destY) const;
    void blitRegion(Segment& segment, int srcX, int srcY, int srcWidth, int srcHeight, int destX, int destY, int destWidth, int destHeight, int srcXOffset, int srcYOffset) const;

private:
    int _width;
    int _height;
    const uint32_t* _rgbwData;
};