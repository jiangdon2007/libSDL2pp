#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "testing.h"

using namespace SDL2pp;

class PixelInspector {
private:
	std::vector<unsigned char> pixels_;
	int width_;
	int height_;
	int bpp_;

public:
	PixelInspector(int width, int height, int bpp) : pixels_(width * height * bpp, 0), width_(width), height_(height), bpp_(bpp) {
	}

	void Retrieve(Renderer& renderer) {
		renderer.ReadPixels(Rect(0, 0, width_, height_), SDL_PIXELFORMAT_ARGB8888, pixels_.data(), width_ * bpp_);
	}

	bool Test(int x, int y, int r, int g, int b, int a = -1) {
		int offset = (x + y * width_) * bpp_;

		if (b >= 0 && pixels_[offset] != b)
			return false;
		if (g >= 0 && pixels_[offset + 1] != g)
			return false;
		if (r >= 0 && pixels_[offset + 2] != r)
			return false;
		if (a >= 0 && pixels_[offset + 3] != a)
			return false;

		return true;
	}

	bool Test3x3(int x, int y, int mask, int r, int g, int b, int a = -1) {
		for (int dy = -1; dy <= 1; dy++) {
			for (int dx = -1; dx <= 1; dx++) {
				bool maskbit = mask & (1 << ((1 - dx) + (1 - dy) * 4));
				if (Test(x + dx, y + dy, r, g, b, a) != !!maskbit)
					return false;
			}
		}
		return true;
	}
};

BEGIN_TEST(int, char*[])
	SDL sdl(SDL_INIT_VIDEO);
	Window window("libSDL2pp test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 320, 240, 0);
	Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);

	PixelInspector pixels(320, 240, 4);

	{
		// Clear
		renderer.SetDrawColor(1, 2, 3);
		renderer.Clear();
		pixels.Retrieve(renderer);

		EXPECT_TRUE(pixels.Test(0, 0, 1, 2, 3));

		renderer.Present();
		SDL_Delay(1000);
	}

	{
		// Draw points
		renderer.SetDrawColor(0,0,0);
		renderer.Clear();

		renderer.SetDrawColor(255, 128, 0);
		renderer.DrawPoint(10, 10);

		renderer.SetDrawColor(0, 255, 128);
		renderer.DrawPoint(20, 20);

		renderer.SetDrawColor(128, 0, 255);
		Point points[] = { {30, 30} };
		renderer.DrawPoints(points, 1);
		pixels.Retrieve(renderer);

		EXPECT_TRUE(pixels.Test3x3(10, 10, 0x020, 255, 128, 0));
		EXPECT_TRUE(pixels.Test3x3(20, 20, 0x020, 0, 255, 128));
		EXPECT_TRUE(pixels.Test3x3(30, 30, 0x020, 128, 0, 255));

		renderer.Present();
		SDL_Delay(1000);
	}

	{
		// Draw lines
		renderer.SetDrawColor(0, 0, 0);
		renderer.Clear();

		renderer.SetDrawColor(255, 128, 0);
		renderer.DrawLine(10, 10, 10, 50);

		renderer.SetDrawColor(0, 255, 128);
		renderer.DrawLine(Point(20, 10), Point(20, 50));

		renderer.SetDrawColor(128, 0, 255);
		Point points[] = { {30, 10}, {30, 50} };
		renderer.DrawLines(points, 2);

		pixels.Retrieve(renderer);

		EXPECT_TRUE(pixels.Test3x3(10, 20, 0x222, 255, 128, 0));
		EXPECT_TRUE(pixels.Test3x3(20, 20, 0x222, 0, 255, 128));
		EXPECT_TRUE(pixels.Test3x3(30, 20, 0x222, 128, 0, 255));

		renderer.Present();
		SDL_Delay(1000);
	}

	{
		// Draw rects
		renderer.SetDrawColor(0, 0, 0);
		renderer.Clear();

		renderer.SetDrawColor(255, 128, 0);
		renderer.DrawRect(10, 10, 19, 19);

		renderer.SetDrawColor(0, 255, 128);
		renderer.DrawRect(Point(30, 10), Point(39, 19));

		renderer.SetDrawColor(128, 0, 255);
		renderer.DrawRect(Rect(10, 30, 10, 10));

		renderer.SetDrawColor(128, 192, 255);
		Rect rects[] = { {30, 30, 10, 10 } };
		renderer.DrawRects(rects, 1);

		pixels.Retrieve(renderer);

		EXPECT_TRUE(pixels.Test3x3(10, 10, 0x032, 255, 128, 0));
		EXPECT_TRUE(pixels.Test3x3(19, 10, 0x062, 255, 128, 0));
		EXPECT_TRUE(pixels.Test3x3(10, 19, 0x230, 255, 128, 0));
		EXPECT_TRUE(pixels.Test3x3(19, 19, 0x260, 255, 128, 0), "depends on OpenGL implementation, which may or may not draw last pixel of a line", NON_FATAL);

		EXPECT_TRUE(pixels.Test3x3(30, 10, 0x032, 0, 255, 128));
		EXPECT_TRUE(pixels.Test3x3(39, 10, 0x062, 0, 255, 128));
		EXPECT_TRUE(pixels.Test3x3(30, 19, 0x230, 0, 255, 128));
		EXPECT_TRUE(pixels.Test3x3(39, 19, 0x260, 0, 255, 128), "depends on OpenGL implementation, which may or may not draw last pixel of a line", NON_FATAL);

		EXPECT_TRUE(pixels.Test3x3(10, 30, 0x032, 128, 0, 255));
		EXPECT_TRUE(pixels.Test3x3(19, 30, 0x062, 128, 0, 255));
		EXPECT_TRUE(pixels.Test3x3(10, 39, 0x230, 128, 0, 255));
		EXPECT_TRUE(pixels.Test3x3(19, 39, 0x260, 128, 0, 255), "depends on OpenGL implementation, which may or may not draw last pixel of a line", NON_FATAL);

		EXPECT_TRUE(pixels.Test3x3(30, 30, 0x032, 128, 192, 255));
		EXPECT_TRUE(pixels.Test3x3(39, 30, 0x062, 128, 192, 255));
		EXPECT_TRUE(pixels.Test3x3(30, 39, 0x230, 128, 192, 255));
		EXPECT_TRUE(pixels.Test3x3(39, 39, 0x260, 128, 192, 255), "depends on OpenGL implementation, which may or may not draw last pixel of a line", NON_FATAL);

		renderer.Present();
		SDL_Delay(1000);
	}

	{
		// Fill rects
		renderer.SetDrawColor(0, 0, 0);
		renderer.Clear();

		renderer.SetDrawColor(255, 128, 0);
		renderer.FillRect(10, 10, 19, 19);

		renderer.SetDrawColor(0, 255, 128);
		renderer.FillRect(Point(30, 10), Point(39, 19));

		renderer.SetDrawColor(128, 0, 255);
		renderer.FillRect(Rect(10, 30, 10, 10));

		renderer.SetDrawColor(128, 192, 255);
		Rect rects[] = { {30, 30, 10, 10 } };
		renderer.FillRects(rects, 1);

		pixels.Retrieve(renderer);

		EXPECT_TRUE(pixels.Test3x3(10, 10, 0x033, 255, 128, 0));
		EXPECT_TRUE(pixels.Test3x3(19, 10, 0x066, 255, 128, 0));
		EXPECT_TRUE(pixels.Test3x3(10, 19, 0x330, 255, 128, 0));
		EXPECT_TRUE(pixels.Test3x3(19, 19, 0x660, 255, 128, 0));

		EXPECT_TRUE(pixels.Test3x3(30, 10, 0x033, 0, 255, 128));
		EXPECT_TRUE(pixels.Test3x3(39, 10, 0x066, 0, 255, 128));
		EXPECT_TRUE(pixels.Test3x3(30, 19, 0x330, 0, 255, 128));
		EXPECT_TRUE(pixels.Test3x3(39, 19, 0x660, 0, 255, 128));

		EXPECT_TRUE(pixels.Test3x3(10, 30, 0x033, 128, 0, 255));
		EXPECT_TRUE(pixels.Test3x3(19, 30, 0x066, 128, 0, 255));
		EXPECT_TRUE(pixels.Test3x3(10, 39, 0x330, 128, 0, 255));
		EXPECT_TRUE(pixels.Test3x3(19, 39, 0x660, 128, 0, 255));

		EXPECT_TRUE(pixels.Test3x3(30, 30, 0x033, 128, 192, 255));
		EXPECT_TRUE(pixels.Test3x3(39, 30, 0x066, 128, 192, 255));
		EXPECT_TRUE(pixels.Test3x3(30, 39, 0x330, 128, 192, 255));
		EXPECT_TRUE(pixels.Test3x3(39, 39, 0x660, 128, 192, 255));

		renderer.Present();
		SDL_Delay(1000);
	}

	{
		// Clip rect
		renderer.SetDrawColor(0, 0, 0);
		renderer.Clear();

		renderer.SetClipRect(Rect(1, 1, 1, 1));

		renderer.SetDrawColor(255, 255, 255);
		renderer.FillRect(0, 0, 10, 10);

		EXPECT_TRUE(renderer.GetClipRect() && renderer.GetClipRect() == Rect(1, 1, 1, 1));

		renderer.SetClipRect(NullOpt);

		EXPECT_TRUE(!renderer.GetClipRect());

		pixels.Retrieve(renderer);

		EXPECT_TRUE(pixels.Test3x3(1, 1, 0x020, 255, 255, 255));

		renderer.Present();
		SDL_Delay(1000);
	}

	{
		// Blend
		renderer.SetDrawColor(0, 0, 0);
		renderer.Clear();

		renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
		renderer.SetDrawColor(255, 255, 255, 127);
		renderer.FillRect(10, 10, 19, 19);

		pixels.Retrieve(renderer);

		EXPECT_TRUE(pixels.Test3x3(10, 10, 0x033, 127, 127, 127));
		EXPECT_TRUE(pixels.Test3x3(19, 10, 0x066, 127, 127, 127));
		EXPECT_TRUE(pixels.Test3x3(10, 19, 0x330, 127, 127, 127));
		EXPECT_TRUE(pixels.Test3x3(19, 19, 0x660, 127, 127, 127));

		renderer.Present();
		SDL_Delay(1000);
	}

#ifdef SDL2PP_WITH_IMAGE
	{
		// Texture
		renderer.SetDrawColor(0, 0, 0);
		renderer.Clear();

		Texture texture(renderer, TESTDATA_DIR "/crate.png");

		EXPECT_EQUAL(texture.GetWidth(), 32);
		EXPECT_EQUAL(texture.GetHeight(), 32);
		EXPECT_EQUAL(texture.GetSize(), Point(32, 32));

		renderer.Copy(texture, NullOpt, Point(0, 0));

		pixels.Retrieve(renderer);

		EXPECT_TRUE(pixels.Test3x3(1, 1, 0x032, 238, 199, 0));

		renderer.Present();
		SDL_Delay(1000);

		// Texture: fill copy
		renderer.SetDrawColor(0, 0, 0);
		renderer.Clear();

		renderer.FillCopy(texture, NullOpt, Rect(0, 0, 48, 48), Point(16, 16), 0);

		pixels.Retrieve(renderer);

		EXPECT_TRUE(pixels.Test3x3(1+16, 1+16, 0x032, 238, 199, 0));

		renderer.Present();
		SDL_Delay(1000);

		// Texture: alpha blending/modulation
		renderer.SetDrawColor(0, 0, 0);
		renderer.Clear();

		EXPECT_EQUAL(texture.GetBlendMode(), SDL_BLENDMODE_NONE);
		texture.SetBlendMode(SDL_BLENDMODE_BLEND);
		EXPECT_EQUAL(texture.GetBlendMode(), SDL_BLENDMODE_BLEND);

		EXPECT_EQUAL((int)texture.GetAlphaMod(), 255);
		texture.SetAlphaMod(127);
		EXPECT_EQUAL((int)texture.GetAlphaMod(), 127);

		renderer.Copy(texture, NullOpt, Point(0, 0));

		pixels.Retrieve(renderer);

		EXPECT_TRUE(pixels.Test3x3(1, 1, 0x032, 119, 99, 0));

		renderer.Present();
		SDL_Delay(1000);

		texture.SetBlendMode();
		EXPECT_EQUAL(texture.GetBlendMode(), SDL_BLENDMODE_NONE);
		texture.SetAlphaMod();
		EXPECT_EQUAL((int)texture.GetAlphaMod(), 255);

		// Texture: color modulation
		renderer.SetDrawColor(0, 0, 0);
		renderer.Clear();

		Uint8 r, g, b;
		texture.GetColorMod(r, g, b);
		EXPECT_EQUAL((int)r, 255);
		EXPECT_EQUAL((int)g, 255);
		EXPECT_EQUAL((int)b, 255);
		texture.SetColorMod(89, 241, 50);

		renderer.Copy(texture, NullOpt, Point(0, 0));

		pixels.Retrieve(renderer);

		EXPECT_TRUE(pixels.Test3x3(1, 1, 0x032, 83, 188, 0));

		renderer.Present();
		SDL_Delay(1000);

		texture.SetColorMod();
		texture.GetColorMod(r, g, b);
		EXPECT_EQUAL((int)r, 255);
		EXPECT_EQUAL((int)g, 255);
		EXPECT_EQUAL((int)b, 255);
	}
#endif // SDL2PP_WITH_IMAGE
END_TEST()
