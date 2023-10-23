#pragma once

#if !defined(WLED_USE_SD_MMC) && !defined(WLED_USE_SD_SPI)
#error "SpriteShow usermod requires SD card to be enabled. Define WLED_USE_SD_MMC or WLED_USE_SD_SPI."
#endif
#ifndef SD_ADAPTER
#error "usermnod_sd_card.h must be included before sprite_show_usermod.h. Does the order of usermods_list.cpp need adjusting?"
#endif

#include "wled.h"

#define STBI_ASSERT(x)                 \
	Serial.printf("Assert: %s\n", #x); \
	assert(x)
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA
#define STBI_ONLY_GIF
#include "stb_image.h"

// class name. Use something descriptive and leave the ": public Usermod" part :)
class SpriteShowUsermod : public Usermod
{
public:
	virtual void setup() override;
	virtual ~SpriteShowUsermod() {}

	virtual void loop() override;

	uint16_t effectLoop();

	static uint16_t staticGetId() { return USERMOD_ID_SPRITE_SHOW; }
	virtual uint16_t getId() override { return staticGetId(); }

private:
	static SpriteShowUsermod *&getInstance()
	{
		static SpriteShowUsermod *instance = nullptr;
		return instance;
	}

	static uint16_t mode_effect();
	void advanceNextImage();
	void updateImage();

	struct Image
	{
		std::string fileName;
	};
	std::vector<Image> images;
	int lastImage;

	struct PixelData
	{
		PixelData()
			: pixels(nullptr), imageWidth(0), imageHeight(0), offsetX(0), offsetY(0), numComponents(0)
		{
		}

		stbi_uc *pixels;
		int imageWidth;
		int imageHeight;
		int offsetX;
		int offsetY;
		int numComponents;
	};
	PixelData currentPixelData;

	unsigned long imageStartTime;
	unsigned long imageSpriteNum;
};

uint16_t SpriteShowUsermod::mode_effect()
{
	if (SpriteShowUsermod *instance = getInstance())
		return instance->effectLoop();
	return FRAMETIME;
}

std::vector<std::string> ListFiles(const char *dirPath)
{
	std::vector<std::string> fileNames;

	File dir = SD_ADAPTER.open(dirPath);
	if (!dir)
	{
		Serial.printf("%s could not be opened\n", dirPath);
		return fileNames;
	}
	if (!dir.isDirectory())
	{
		Serial.printf("%s is not a directory\n", dirPath);
		return fileNames;
	}

	File file = dir.openNextFile();
	while (file)
	{
		Serial.printf("Found %s\n", file.name());

		if (!file.isDirectory())
		{
			fileNames.push_back(file.name());
		}
		file = dir.openNextFile();
	}

	return fileNames;
}

const char *GetExtension(const char *fileName)
{
	return strrchr(fileName, '.');
}

void SpriteShowUsermod::setup()
{
	getInstance() = this;
	lastImage = -1;
	imageStartTime = 0;
	imageSpriteNum = 0;

	Serial.println("SpriteShowUsermod::Setup");

	static const char _data_FX_MODE[] PROGMEM = "Sprite Show";

	strip.addEffect(255, &SpriteShowUsermod::mode_effect, _data_FX_MODE);

	images.clear();
	std::vector<std::string> files = ListFiles("/");
	for (std::string &file : files)
	{
		const char *ext = GetExtension(file.c_str());
		if (!ext)
			continue;

		if (strstr(".png.jpg.jpeg.gif.bmp.tga", ext))
		{
			Serial.printf("Found image: %s\n", file.c_str());

			Image image;
			image.fileName = std::move(file);
			images.push_back(std::move(image));
		}
		else
		{
			Serial.printf("Disregarded %s, ext: %s\n", file.c_str(), ext);
		}
	}
}

void SpriteShowUsermod::advanceNextImage()
{
	int imageNum = random16(images.size() - (lastImage >= 0 ? 1 : 0));
	// Stop repeats by reducing the random range by 1, and selecting the last if the previous image comes up again
	if (imageNum == lastImage)
		imageNum = images.size() - 1;

	Serial.printf("Selected image %d/%d\n", imageNum, images.size());

	stbi_image_free(currentPixelData.pixels);
	currentPixelData.pixels = nullptr;

	lastImage = imageNum;
	imageStartTime = millis();
	currentPixelData.offsetX = 0;
	currentPixelData.offsetY = 0;

	const Image &image = images[lastImage];

	const std::string &filePath = image.fileName;
	Serial.printf("Opening image %s\n", filePath.c_str());
	File file = SD_ADAPTER.open(filePath.c_str(), FILE_READ);
	int fileSize = file.size();
	if (!file)
		Serial.println("Failed to open image");
	Serial.printf("Opened image - %d bytes\n", fileSize);

	std::vector<char> buffer;
	buffer.resize(fileSize);

	int bytesRead = file.readBytes(buffer.data(), fileSize);
	Serial.printf("Read %d bytes (of %d)\n", bytesRead, fileSize);
	file.close();

	currentPixelData.pixels = stbi_load_from_memory((stbi_uc *)buffer.data(), fileSize, &currentPixelData.imageWidth, &currentPixelData.imageHeight, &currentPixelData.numComponents, 3);
	buffer = std::vector<char>(); // release the allocation

	if (!currentPixelData.pixels)
		Serial.printf("Image load failed: %s\n", stbi_failure_reason());
	Serial.printf("Image loaded %ux%u\n", currentPixelData.imageWidth, currentPixelData.imageHeight);
}

void SpriteShowUsermod::updateImage()
{
	const unsigned long imageShowTimeMillis = 5000;
	unsigned long timeMillis = millis();
	if (!currentPixelData.pixels || imageStartTime + imageShowTimeMillis <= timeMillis)
	{
		advanceNextImage();
		return;
	}

	const unsigned long spriteShowTimeMillis = 1000 / 10;
	int spriteNum = (timeMillis - imageStartTime) / spriteShowTimeMillis;
	int numSpritesX = currentPixelData.imageWidth / SEGMENT.width();
	int numSpritesY = currentPixelData.imageHeight / SEGMENT.height();

	int curSpriteX = spriteNum % numSpritesX;
	int curSpriteY = (spriteNum / numSpritesX) % numSpritesY;

	currentPixelData.offsetX = curSpriteX * SEGMENT.width();
	currentPixelData.offsetY = curSpriteY * SEGMENT.height();
}

void SpriteShowUsermod::loop()
{
}

uint16_t SpriteShowUsermod::effectLoop()
{
	updateImage();

	if (!currentPixelData.pixels)
		return FRAMETIME;

	int startX = currentPixelData.offsetX;
	int startY = currentPixelData.offsetY;

	int imageWidth = currentPixelData.imageWidth;
	int imageHeight = currentPixelData.imageHeight;

	int ledWidth = SEGMENT.width();
	int ledHeight = SEGMENT.height();

	int widthToCopy = min(imageWidth - startX, ledWidth);
	int heightToCopy = min(imageHeight - startY, ledHeight);

	int numComponents = currentPixelData.numComponents;

	for (int y = 0; y < heightToCopy; ++y)
	{
		int pixelY = startY + y;
		stbi_uc *pixel = currentPixelData.pixels + (((pixelY * imageWidth) + startX) * numComponents);
		for (int x = 0; x < widthToCopy; ++x)
		{
			SEGMENT.setPixelColorXY(x, y, pixel[0], pixel[1], pixel[2]);
			pixel += numComponents;
		}
	}

	return FRAMETIME;
}