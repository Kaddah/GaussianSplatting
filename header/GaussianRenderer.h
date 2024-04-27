#include "Window.h"

class GaussianRenderer : public Window {
public:
	GaussianRenderer(LPCTSTR WindowName, int width, int height, bool fullScreen, HINSTANCE hInstance, int nShowCmd);
	void draw() override;
	void prepareTriangle();

};