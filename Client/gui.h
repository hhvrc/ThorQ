#ifndef GUI_H
#define GUI_H

class GLFWwindow;

class Gui
{
	int m_dims[2];
	GLFWwindow* m_window;
	Gui(GLFWwindow* m_window, int width, int height);
public:
	~Gui();
	static bool Init();
	static Gui* CreateGui(const char* title, int width, int height);
	void Run();


};

#endif // GUI_H
