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
		struct
		{
			button MoveForward;
			button MoveBack;
			button MoveRight;
			button MoveLeft;
			button MoveUp;;

			button H;
		};
		button Buttons[6];
	};
};

static bool
WasDown(button *Button)
{
	bool Result = (Button->HalfTransitionCount > 1) ||
				  ((Button->HalfTransitionCount == 1) && Button->EndedDown);
	
	return(Result);
}

struct job_system_queue;
typedef void job_system_callback(void *Data);
static void JobSystemAddEntry(job_system_queue *JobSystem, job_system_callback *Callback, void *Data);
static void JobSystemCompleteAllWork(job_system_queue *JobSystem);

struct game_memory
{
	uint64_t PermanentStorageSize;
	void *PermanentStorage;

	uint64_t TemporaryStorageSize;
	void *TemporaryStorage;

	job_system_queue *JobSystemQueue;
};


#include <Windows.h>
#include <intrin.h>

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
	if(Key == GLFW_KEY_SPACE)
	{
		if(Action == GLFW_PRESS)
		{
			Input->MoveUp.EndedDown = true;
			++Input->MoveUp.HalfTransitionCount;
		}
		else if(Action == GLFW_RELEASE)
		{
			Input->MoveUp.EndedDown = false;
			++Input->MoveUp.HalfTransitionCount;
		}
	}
	if(Key == GLFW_KEY_H)
	{
		if(Action == GLFW_PRESS)
		{
			Input->H.EndedDown = true;
			++Input->H.HalfTransitionCount;
		}
		else if(Action == GLFW_RELEASE)
		{
			Input->H.EndedDown = false;
			++Input->H.HalfTransitionCount;
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

struct job_system_entry
{
	job_system_callback *Callback;
	void *Data;
};
struct job_system_queue
{
	uint32_t volatile JobsToCompleteCount;
	uint32_t volatile JobsToCompleteGoal;

	uint32_t volatile EntryToRead;
	uint32_t volatile EntryToWrite;
	HANDLE Semaphore;

	job_system_entry Entries[128];
};

static void
JobSystemAddEntry(job_system_queue *JobSystem, job_system_callback *Callback, void *Data)
{
	uint32_t NewEntryToWrite = (JobSystem->EntryToWrite + 1) % ArrayCount(JobSystem->Entries);
	Assert(NewEntryToWrite != JobSystem->EntryToRead);

	job_system_entry *Entry = JobSystem->Entries + JobSystem->EntryToWrite;
	Entry->Callback = Callback;
	Entry->Data = Data;

	++JobSystem->JobsToCompleteGoal;
	_WriteBarrier();

	JobSystem->EntryToWrite = NewEntryToWrite;
	ReleaseSemaphore(JobSystem->Semaphore, 1, 0);
}

static bool
DoNextJobQueueEntry(job_system_queue *JobSystem)
{
	bool Sleep = false;

	uint32_t OriginalEntryToRead = JobSystem->EntryToRead;
	uint32_t NewEntryToRead = (OriginalEntryToRead + 1) % ArrayCount(JobSystem->Entries);
	if(JobSystem->EntryToRead != JobSystem->EntryToWrite)
	{
		uint32_t Index = InterlockedCompareExchange((LONG volatile *)&JobSystem->EntryToRead, NewEntryToRead, OriginalEntryToRead);

		if(Index == OriginalEntryToRead)
		{
			job_system_entry *Entry = JobSystem->Entries + Index;
			Entry->Callback(Entry->Data);
			InterlockedIncrement((LONG volatile *)&JobSystem->JobsToCompleteCount);
		}
	}
	else
	{
		Sleep = true;
	}

	return(Sleep);
}

static void
JobSystemCompleteAllWork(job_system_queue *JobSystem)
{
	while(JobSystem->JobsToCompleteCount != JobSystem->JobsToCompleteGoal)
	{
		DoNextJobQueueEntry(JobSystem);
	}

	JobSystem->JobsToCompleteCount = 0;
	JobSystem->JobsToCompleteGoal = 0;
}

DWORD WINAPI
ThreadProc(LPVOID lpParameter)
{
	job_system_queue *JobSystem = (job_system_queue *)lpParameter;

	while(1)
	{
		if(DoNextJobQueueEntry(JobSystem))
		{
			WaitForSingleObjectEx(JobSystem->Semaphore, INFINITE, false);
		}
	}

	return(0);
}

static void
InitializeJobSystem(job_system_queue *JobSystem, uint32_t ThreadCount)
{
	JobSystem->JobsToCompleteCount = 0;
	JobSystem->JobsToCompleteGoal = 0;

	JobSystem->EntryToRead = 0;
	JobSystem->EntryToWrite = 0;
	
	uint32_t InitialValue = 0;
	JobSystem->Semaphore = CreateSemaphoreEx(0, InitialValue, INT_MAX, 0, 0, SEMAPHORE_ALL_ACCESS);

	for(uint32_t ThreadIndex = 0;
		ThreadIndex < ThreadCount;
		ThreadIndex++)
	{
		HANDLE ThreadHandle = CreateThread(0, 0, ThreadProc, JobSystem, 0, 0);
		CloseHandle(ThreadHandle);
	}
}

int main(void)
{
	job_system_queue JobSystem;
	InitializeJobSystem(&JobSystem, 3);

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
	GameMemory.JobSystemQueue = &JobSystem;
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
			// std::cout << "Frame time: " << EndTime - LastTime << '\n';
			LastTime = EndTime;
		}
	}

	return(0);
}