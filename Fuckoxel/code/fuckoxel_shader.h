#pragma once
#include <iostream>

class shader
{
	private:
		uint32_t ID;

	public:
		shader() {}
		shader(const char *VSPath, const char *FSPath);

		void Use(void);
		void SetR32(const char *Name, float Value);
		void SetI32(const char *Name, int32_t Value);
		void SetVec2(const char *Name, vec2 V);
		void SetVec3(const char *Name, vec3 V);
		void SetVec4(const char *Name, vec4 V);
		void SetMat4(const char *Name, const mat4 &M);
        void SetVec2Array(const char *Name, uint32_t Count, vec2 *V);
};

struct read_entire_file_result
{
	void* Memory;
	uint32_t Size;
};
static read_entire_file_result
ReadEntireFile(const char *Filename)
{
    read_entire_file_result Result = {};

    FILE *File = fopen(Filename, "rb");
    if(File)
    {
        fseek(File, 0, SEEK_END);
        Result.Size = ftell(File);
        fseek(File, 0, SEEK_SET);

        Result.Memory = malloc(Result.Size);
		fread(Result.Memory, 1, Result.Size, File);
    
        fclose(File);
    }

    return(Result);
}

shader::shader(const char *VSPath, const char *FSPath)
{
    read_entire_file_result VSSourceCode = ReadEntireFile(VSPath);
    read_entire_file_result FSSourceCode = ReadEntireFile(FSPath);

    GLuint VS = glCreateShader(GL_VERTEX_SHADER);
    GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);
    ID = glCreateProgram();

    int32_t Success;
    char InfoLog[1024];

    glShaderSource(VS, 1, (char **)&VSSourceCode.Memory, (GLint *)&VSSourceCode.Size);
    glCompileShader(VS);
    glGetShaderiv(VS, GL_COMPILE_STATUS, &Success);
    if(!Success)
	{
		glGetShaderInfoLog(VS, sizeof(InfoLog), 0, InfoLog);
		std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: VS\n" << InfoLog << std::endl;
	}

    glShaderSource(FS, 1, (char **)&FSSourceCode.Memory, (GLint*)&FSSourceCode.Size);
    glCompileShader(FS);
    glGetShaderiv(FS, GL_COMPILE_STATUS, &Success);
    if(!Success)
	{
		glGetShaderInfoLog(FS, sizeof(InfoLog), 0, InfoLog);
		std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: FS\n" << InfoLog << std::endl;
	}

    glAttachShader(ID, VS);
    glAttachShader(ID, FS);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &Success);
    if(!Success)
	{
		glGetProgramInfoLog(ID, sizeof(InfoLog), 0, InfoLog);
		std::cout << "ERROR::PROGRAM_LINKING_ERROR of type:: PROGRAM\n" << InfoLog << "\n";
	}

    glDeleteShader(VS);
    glDeleteShader(FS);
    free(VSSourceCode.Memory);
    free(FSSourceCode.Memory);
}

void shader::Use(void)
{
    glUseProgram(ID);
}

void shader::SetR32(const char *Name, float Value)
{
    glUniform1f(glGetUniformLocation(ID, Name), Value);
}

void shader::SetI32(const char *Name, int32_t Value)
{
    glUniform1i(glGetUniformLocation(ID, Name), Value);
}

void shader::SetVec2(const char *Name, vec2 V)
{
    glUniform2f(glGetUniformLocation(ID, Name), V.x, V.y);
}

void shader::SetVec3(const char *Name, vec3 V)
{
    glUniform3fv(glGetUniformLocation(ID, Name), 1, &V.x);
}

void shader::SetVec4(const char *Name, vec4 V)
{
    glUniform4fv(glGetUniformLocation(ID, Name), 1, &V.x);
}

void shader::SetMat4(const char *Name, const mat4 &M)
{
    glUniformMatrix4fv(glGetUniformLocation(ID, Name), 1, GL_FALSE, M.E);
}

void shader::SetVec2Array(const char *Name, uint32_t Count, vec2 *V)
{
    glUniform2fv(glGetUniformLocation(ID, Name), Count, (GLfloat *)V);
}