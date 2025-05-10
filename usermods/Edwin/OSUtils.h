#pragma once

extern const Image* img_Wallpaper;

void DrawDesktopWallpaper() {
    img_Wallpaper->blit(SEGMENT, 0, 0);
}

void DrawTaskbar(int numButtons) {
    img_Taskbar.blit(SEGMENT, 0, 7);

    for (int i = 0; i < numButtons; ++i) {
        img_Taskbar.blitRegion(SEGMENT, 0, 0, 2, 1, (i+1)*3, 7);
    }
}

void DrawDesktopIcons() {
    SEGMENT.setPixelColorXY(0, 0, RGBW32(64, 64, 64, 0));
    SEGMENT.setPixelColorXY(0, 2, RGBW32(64, 64, 64, 0));
    SEGMENT.setPixelColorXY(0, 4, RGBW32(64, 64, 64, 0));
    SEGMENT.setPixelColorXY(2, 0, RGBW32(64, 64, 64, 0));
}