#ifndef GUI_H
#define GUI_H

class GLFWwindow;

class Gui
{
	GLFWwindow* m_window = nullptr;
	Gui(GLFWwindow* m_window);
public:
	~Gui();
	static bool Init();
	static Gui* CreateGui(const char* title, int width, int height);
	void Run();
};

#endif // GUI_H
