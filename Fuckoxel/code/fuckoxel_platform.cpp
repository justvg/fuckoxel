#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define Assert(Expression) if(!(Expression)) { *(int *)0 = 0; }
#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

#define Kilobytes(Value) (1024LL*(Value))
#define Megabytes(Value) (1024LL*Kilobytes(Value))
#define Gigabytes(Value) (1024LL*Megabytes(Value))

struct button
{
	bool EndedDown;
	uint32_t HalfTransitionCount;
};
struct game_input
{
	float dt;
	int MouseX, MouseY;
	float MouseDeltaX, MouseDeltaY;
	button MouseLeft, MouseRight;

	union
	{
		button Buttons[4];
		struct
		{
			button MoveForward;
			button MoveBack;
			button MoveRight;
			button MoveLeft;
		};
	};
};

struct game_memory
{
	uint64_t PermanentStorageSize;
	void *PermanentStorage;

	uint64_t TemporaryStorageSize;
	void *TemporaryStorage;
};

static bool
WasDown(button *Button)
{
	bool Result = (Button->HalfTransitionCount > 1) ||
				  ((Button->HalfTransitionCount == 1) && Button->EndedDown);
	
	return(Result);
}

#include "fuckoxel.cpp"

static void
GLFWKeyCallback(GLFWwindow *Window, int Key, int ScanCode, int Action, int Mods)
{
	game_input *Input = (game_input *)glfwGetWindowUserPointer(Window);

	if(Key == GLFW_KEY_W)
	{
		if(Action == GLFW_PRESS)
		{
			Input->MoveForward.EndedDown = true;
			++Input->MoveForward.HalfTransitionCount;
		}
		else if(Action == GLFW_RELEASE)
		{
			Input->MoveForward.EndedDown = false;
			++Input->MoveForward.HalfTransitionCount;
		}
	}
	if(Key == GLFW_KEY_A)
	{
		if(Action == GLFW_PRESS)
		{
			Input->MoveLeft.EndedDown = true;
			++Input->MoveLeft.HalfTransitionCount;
		}
		else if(Action == GLFW_RELEASE)
		{
			Input->MoveLeft.EndedDown = false;
			++Input->MoveLeft.HalfTransitionCount;
		}
	}
	if(Key == GLFW_KEY_D)
	{
		if(Action == GLFW_PRESS)
		{
			Input->MoveRight.EndedDown = true;
			++Input->MoveRight.HalfTransitionCount;
		}
		else if(Action == GLFW_RELEASE)
		{
			Input->MoveRight.EndedDown = false;
			++Input->MoveRight.HalfTransitionCount;
		}
	}
	if(Key == GLFW_KEY_S)
	{
		if(Action == GLFW_PRESS)
		{
			Input->MoveBack.EndedDown = true;
			++Input->MoveBack.HalfTransitionCount;
		}
		else if(Action == GLFW_RELEASE)
		{
			Input->MoveBack.EndedDown = false;
			++Input->MoveBack.HalfTransitionCount;
		}
	}
}

static void
GLFWMouseButtonCallback(GLFWwindow *Window, int Button, int Action, int Mods)
{
	game_input *Input = (game_input *)glfwGetWindowUserPointer(Window);

	if(Button == GLFW_MOUSE_BUTTON_1)
	{
		if(Action == GLFW_PRESS)
		{
			Input->MouseLeft.EndedDown = true;
			++Input->MouseLeft.HalfTransitionCount;
		}
		else if(Action == GLFW_RELEASE)
		{
			Input->MouseLeft.EndedDown = false;
			++Input->MouseLeft.HalfTransitionCount;
		}		
	}
	if(Button == GLFW_MOUSE_BUTTON_2)
	{
		if(Action == GLFW_PRESS)
		{
			Input->MouseRight.EndedDown = true;
			++Input->MouseRight.HalfTransitionCount;
		}
		else if(Action == GLFW_RELEASE)
		{
			Input->MouseRight.EndedDown = false;
			++Input->MouseRight.HalfTransitionCount;
		}		
	}
}

static void
GLFWCursorPosCallback(GLFWwindow *Window, double XPos, double YPos)
{
	game_input *Input = (game_input *)glfwGetWindowUserPointer(Window);

	Input->MouseDeltaX = (float)((int)XPos - Input->MouseX);
	Input->MouseDeltaY = (float)((int)YPos - Input->MouseY);

	Input->MouseX = (int)XPos;
	Input->MouseY = (int)YPos;
}

int main(void)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow *Window = glfwCreateWindow(900, 540, "Fuckoxel", 0, 0);
	glfwMakeContextCurrent(Window);
	glfwSwapInterval(1);
	glfwSetKeyCallback(Window, GLFWKeyCallback);
	glfwSetMouseButtonCallback(Window, GLFWMouseButtonCallback);
	glfwSetCursorPosCallback(Window, GLFWCursorPosCallback);
	glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	game_memory GameMemory = {};
	GameMemory.PermanentStorageSize = Megabytes(256);
	GameMemory.TemporaryStorageSize = Gigabytes(3);
	GameMemory.PermanentStorage = malloc(GameMemory.PermanentStorageSize + GameMemory.TemporaryStorageSize);
	GameMemory.TemporaryStorage = (uint8_t *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize; 
	if(GameMemory.TemporaryStorage)
	{
		memset(GameMemory.PermanentStorage, 0, GameMemory.PermanentStorageSize + GameMemory.TemporaryStorageSize);

		game_input GameInput = {};
		// TODO(georgy): Make this not that cheesy!!!
		GameInput.dt = 0.0166666f;
		glfwSetWindowUserPointer(Window, &GameInput);

		glewInit();

		glViewport(0, 0, 900, 540);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_FRAMEBUFFER_SRGB);
		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glClearColor(0.2f, 0.4f, 0.8f, 1.0f);

		double LastTime = glfwGetTime();
		while(!glfwWindowShouldClose(Window))
		{
			GameInput.MouseDeltaX = GameInput.MouseDeltaY = 0.0f;
			GameInput.MouseLeft.HalfTransitionCount = GameInput.MouseRight.HalfTransitionCount = 0;
			for(uint32_t ButtonIndex = 0; 
				ButtonIndex < ArrayCount(GameInput.Buttons);
				ButtonIndex++)
			{
				button *Button = GameInput.Buttons + ButtonIndex;
				Button->HalfTransitionCount = 0;
			}

			glfwPollEvents();

			GameUpdateAndRender(&GameMemory, &GameInput, 900, 540);

			glfwSwapBuffers(Window);

			double EndTime = glfwGetTime();
			std::cout << "Frame time: " << EndTime - LastTime << std::endl;
			LastTime = EndTime;
		}
	}

	return(0);
}