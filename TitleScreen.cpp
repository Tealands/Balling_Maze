#include "TitleScreen.h"

#include <algorithm>
#include <string>
#include <vector>
#include <windowsx.h>
#include <gdiplus.h>
#include <GL/gl.h>

#pragma comment(lib, "gdiplus.lib")

using Gdiplus::Bitmap;
using Gdiplus::Color;
using Gdiplus::GdiplusShutdown;
using Gdiplus::GdiplusStartup;
using Gdiplus::GdiplusStartupInput;

TitleScreen::TitleScreen()
{
	GdiplusStartupInput input;
	ULONG_PTR token = 0;
	if (GdiplusStartup(&token, &input, nullptr) == 0) {
		gdiplusToken = static_cast<unsigned long long>(token);
		gdiplusStarted = true;
	}
}

TitleScreen::~TitleScreen()
{
	releaseGraphics();
	if (gdiplusStarted) {
		GdiplusShutdown(static_cast<ULONG_PTR>(gdiplusToken));
	}
}

void TitleScreen::releaseGraphics()
{
	if (textureId != 0) {
		glDeleteTextures(1, &textureId);
		textureId = 0;
	}
	if (fontBase != 0) {
		glDeleteLists(fontBase, 96);
		fontBase = 0;
	}
}

void TitleScreen::draw(HDC hdc, int width, int height)
{
	ensureTextureLoaded();
	ensureFont(hdc);

	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width, height, 0.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	drawBackground(width, height);
	drawText2D("Balling Maze", width * 0.5f - 190.0f, height * 0.22f, 2.0f);

	RectF startRect = getButtonRect(Button::Start, width, height);
	RectF exitRect = getButtonRect(Button::Exit, width, height);
	drawButton("START", startRect, selectedButton == Button::Start, hasPressedButton && pressedButton == Button::Start);
	drawButton("EXIT", exitRect, selectedButton == Button::Exit, hasPressedButton && pressedButton == Button::Exit);

	glEnable(GL_DEPTH_TEST);
}

void TitleScreen::onMouseMove(int x, int y, int width, int height)
{
	if (contains(getButtonRect(Button::Start, width, height), x, y)) {
		selectedButton = Button::Start;
	} else if (contains(getButtonRect(Button::Exit, width, height), x, y)) {
		selectedButton = Button::Exit;
	}
}

void TitleScreen::onMouseDown(int x, int y, int width, int height)
{
	if (contains(getButtonRect(Button::Start, width, height), x, y)) {
		pressedButton = Button::Start;
		hasPressedButton = true;
	} else if (contains(getButtonRect(Button::Exit, width, height), x, y)) {
		pressedButton = Button::Exit;
		hasPressedButton = true;
	}
}

TitleAction TitleScreen::onMouseUp(int x, int y, int width, int height)
{
	if (!hasPressedButton) return TitleAction::None;
	hasPressedButton = false;
	RectF rect = getButtonRect(pressedButton, width, height);
	return contains(rect, x, y) ? actionFor(pressedButton) : TitleAction::None;
}

TitleAction TitleScreen::onKeyDown(WPARAM key)
{
	if (key == VK_RETURN) return actionFor(selectedButton);
	if (key == VK_ESCAPE) return TitleAction::Exit;
	if (key == VK_UP || key == VK_DOWN || key == VK_TAB) {
		selectedButton = (selectedButton == Button::Start) ? Button::Exit : Button::Start;
		return TitleAction::None;
	}
	return TitleAction::None;
}

TitleScreen::RectF TitleScreen::getButtonRect(Button button, int width, int height) const
{
	float responsiveWidth = static_cast<float>(width) * 0.55f;
	float buttonWidth = responsiveWidth < 320.0f ? responsiveWidth : 320.0f;
	float buttonHeight = 62.0f;
	float left = (static_cast<float>(width) - buttonWidth) * 0.5f;
	float top = static_cast<float>(height) * 0.55f + (button == Button::Exit ? 82.0f : 0.0f);
	return { left, top, left + buttonWidth, top + buttonHeight };
}

bool TitleScreen::contains(const RectF& rect, int x, int y) const
{
	return x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom;
}

void TitleScreen::ensureTextureLoaded()
{
	if (attemptedTextureLoad || !gdiplusStarted) return;
	attemptedTextureLoad = true;

	Bitmap source(L"assets\\title_bg.png");
	if (source.GetLastStatus() != 0 || source.GetWidth() == 0 || source.GetHeight() == 0) return;

	Bitmap bitmap(source.GetWidth(), source.GetHeight(), PixelFormat32bppARGB);
	Gdiplus::Graphics graphics(&bitmap);
	graphics.DrawImage(&source, 0, 0, source.GetWidth(), source.GetHeight());

	std::vector<unsigned char> pixels(source.GetWidth() * source.GetHeight() * 4);
	for (UINT y = 0; y < source.GetHeight(); ++y) {
		for (UINT x = 0; x < source.GetWidth(); ++x) {
			Color color;
			bitmap.GetPixel(x, y, &color);
			size_t offset = (static_cast<size_t>(y) * source.GetWidth() + x) * 4;
			pixels[offset + 0] = color.GetRed();
			pixels[offset + 1] = color.GetGreen();
			pixels[offset + 2] = color.GetBlue();
			pixels[offset + 3] = color.GetAlpha();
		}
	}

	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, source.GetWidth(), source.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void TitleScreen::ensureFont(HDC hdc)
{
	if (fontBase != 0) return;
	fontBase = glGenLists(96);
	HFONT font = CreateFontA(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE, "Segoe UI");
	HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, font));
	wglUseFontBitmapsA(hdc, 32, 96, fontBase);
	SelectObject(hdc, oldFont);
	DeleteObject(font);
}

void TitleScreen::drawBackground(int width, int height)
{
	if (textureId != 0) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(static_cast<float>(width), 0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(static_cast<float>(width), static_cast<float>(height));
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, static_cast<float>(height));
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
		return;
	}

	glBegin(GL_QUADS);
	glColor3f(0.05f, 0.10f, 0.14f); glVertex2f(0.0f, 0.0f);
	glColor3f(0.15f, 0.29f, 0.26f); glVertex2f(static_cast<float>(width), 0.0f);
	glColor3f(0.53f, 0.45f, 0.24f); glVertex2f(static_cast<float>(width), static_cast<float>(height));
	glColor3f(0.10f, 0.12f, 0.15f); glVertex2f(0.0f, static_cast<float>(height));
	glEnd();
}

void TitleScreen::drawButton(const char* label, const RectF& rect, bool selected, bool pressed)
{
	float inset = pressed ? 4.0f : 0.0f;
	glColor3f(selected ? 0.95f : 0.72f, selected ? 0.75f : 0.62f, selected ? 0.28f : 0.42f);
	glBegin(GL_QUADS);
	glVertex2f(rect.left + inset, rect.top + inset);
	glVertex2f(rect.right - inset, rect.top + inset);
	glVertex2f(rect.right - inset, rect.bottom - inset);
	glVertex2f(rect.left + inset, rect.bottom - inset);
	glEnd();

	glColor3f(0.04f, 0.07f, 0.08f);
	drawText2D(label, rect.left + (rect.right - rect.left) * 0.5f - 48.0f, rect.top + 41.0f + inset, 1.0f);
}

void TitleScreen::drawText2D(const char* text, float x, float y, float scale)
{
	if (fontBase == 0) return;
	glPushMatrix();
	glTranslatef(x, y, 0.0f);
	glScalef(scale, scale, 1.0f);
	glRasterPos2f(0.0f, 0.0f);
	glListBase(fontBase - 32);
	glCallLists(static_cast<GLsizei>(std::char_traits<char>::length(text)), GL_UNSIGNED_BYTE, text);
	glPopMatrix();
}

TitleAction TitleScreen::actionFor(Button button) const
{
	return button == Button::Start ? TitleAction::Start : TitleAction::Exit;
}