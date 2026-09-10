#pragma once

#include <windows.h>

enum class TitleAction {
	None,
	Start,
	Exit
};

class TitleScreen {
public:
	TitleScreen();
	~TitleScreen();

	void releaseGraphics();
	void draw(HDC hdc, int width, int height);
	void onMouseMove(int x, int y, int width, int height);
	void onMouseDown(int x, int y, int width, int height);
	TitleAction onMouseUp(int x, int y, int width, int height);
	TitleAction onKeyDown(WPARAM key);

private:
	enum class Button {
		Start,
		Exit
	};

	struct RectF {
		float left;
		float top;
		float right;
		float bottom;
	};

	Button selectedButton = Button::Start;
	Button pressedButton = Button::Start;
	bool hasPressedButton = false;
	unsigned int textureId = 0;
	unsigned int fontBase = 0;
	bool attemptedTextureLoad = false;
	bool gdiplusStarted = false;
	unsigned long long gdiplusToken = 0;

	RectF getButtonRect(Button button, int width, int height) const;
	bool contains(const RectF& rect, int x, int y) const;
	void ensureTextureLoaded();
	void ensureFont(HDC hdc);
	void drawBackground(int width, int height);
	void drawButton(const char* label, const RectF& rect, bool selected, bool pressed);
	void drawText2D(const char* text, float x, float y, float scale);
	TitleAction actionFor(Button button) const;
};