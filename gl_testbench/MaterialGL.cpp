#include <windows.h>
#include <streambuf>
#include <sstream>
#include <istream>
#include <fstream>
#include <vector>
#include <set>
#include <assert.h>

#include "MaterialGL.h"

typedef unsigned int uint;



// recursive function to split a string by a delimiter
// easier to read than all that crap using STL...
void split(std::string text, std::vector<std::string>* temp, const char delim = ' ')
{
	int pos = text.find(delim, 0);
	if (pos == -1)
		temp->push_back(text);
	else {
		temp->push_back(text.substr(0, pos));
		split(text.substr(pos + 1, text.length() - pos - 1), temp, delim);
	}
}

/*
	vtx_shader is the basic shader text coming from the .vs file.
	strings will be added to the shader in this order:
	// - version of GLSL
	// - defines from map
	// - existing shader code
*/
std::vector<std::string> MaterialGL::expandShaderText(std::string& shaderSource, ShaderType type)
{
	//std::vector<std::string> input_definitions;
	//std::ifstream includeFile("IA.h");
	//std::string includeFileString((std::istreambuf_iterator<char>(includeFile)), std::istreambuf_iterator<char>());
	//includeFile.close();
	std::vector<std::string> result{ "\n\n #version 450\n\0" };
	for (auto define : shaderDefines[type])
		result.push_back(define);
	result.push_back(shaderSource);
	return result;
};

MaterialGL::MaterialGL()
{ 
	isValid = false;
	mapShaderEnum[(uint)ShaderType::VS] = GL_VERTEX_SHADER;
	mapShaderEnum[(uint)ShaderType::PS] = GL_FRAGMENT_SHADER;
	mapShaderEnum[(uint)ShaderType::GS] = GL_GEOMETRY_SHADER;
	mapShaderEnum[(uint)ShaderType::CS] = GL_COMPUTE_SHADER;
};

MaterialGL::~MaterialGL() 
{ 
	// most basic for now.
	for (auto shaderObject : shaderObject) {
		glDeleteShader(shaderObject);
	};
	glDeleteProgram(program);
}

void MaterialGL::setShader(const std::string& shaderFileName, ShaderType type)
{
	if (shaderFileNames.find(type) != shaderFileNames.end())
	{
		// removeShader is implemented in a concrete class
		removeShader(type);
	}
	shaderFileNames[type] = shaderFileName;
};


void MaterialGL::removeShader(ShaderType type)
{
	GLuint shader = shaderObject[(GLuint)type];

	// check if name exists (if it doesn't there should not be a shader here.
	if (shaderFileNames.find(type) == shaderFileNames.end())
	{
		assert(shader == 0);
		return;
	}
	else if (shader != 0 && program != 0) {
		glDetachShader(program, shader);
		glDeleteShader(shader);
	};
}

int MaterialGL::compileShader(ShaderType type, std::string& errString)
{
	// index in the the array "shaderObject[]";
	GLuint shaderIdx = (GLuint)type;

	// open the file and read it to a string "shaderText"
	std::ifstream shaderFile(shaderFileNames[type]);
	std::string shaderText;
	if (shaderFile.is_open()) {
		shaderText = std::string((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
		shaderFile.close();
	}
	else {
		errString = "Cannot find file: " + shaderNames[shaderIdx];
		return -1;
	}

	// make final vector<string> with shader source + defines + GLSL version
	std::vector<std::string> shaderLines = expandShaderText(shaderText, type);

	// debug
	for (auto ex : shaderLines) 
		DBOUTW(ex.c_str());

	// OpenGL wants an array of GLchar* with null terminated strings 
	const GLchar** tempShaderLines = new const GLchar*[shaderLines.size()];
	int i = 0; 
	// need "auto&" to force "text" not to be a temp string, as we are using the c_str()
	for (auto &text : shaderLines)
		tempShaderLines[i++] = text.c_str();
	
	GLuint newShader = glCreateShader(mapShaderEnum[shaderIdx]);
	glShaderSource(newShader, shaderLines.size(), tempShaderLines, nullptr);
	// ask GL to compile this
	glCompileShader(newShader);
	// print error or anything...
	INFO_OUT(newShader, Shader);
	std::string err2;
	COMPILE_LOG(newShader, Shader, err2);
	shaderObject[shaderIdx] = newShader;
	return 0;
}

int MaterialGL::compileMaterial(std::string& errString)
{
	// remove all shaders.
	removeShader(ShaderType::VS);
	removeShader(ShaderType::PS);
	// compile shaders
	std::string err;
	if (compileShader(ShaderType::VS, err) < 0) {
		errString = err;
		return -1;
	};
	if (compileShader(ShaderType::PS, err) < 0) {
		errString = err;
		return -1;
	};
	
	// try to link the program
	// link shader program (connect vs and ps)
	if (program != 0)
		glDeleteProgram(program);

	program = glCreateProgram();
	glAttachShader(program, shaderObject[(GLuint)ShaderType::VS]);
	glAttachShader(program, shaderObject[(GLuint)ShaderType::PS]);
	glLinkProgram(program);

	std::string err2;
	INFO_OUT(program, Program);
	COMPILE_LOG(program, Program, err2);
	isValid = true;
	return 0;
}

int MaterialGL::enable() {
	if (program == 0 || isValid == false)
		return -1;
	glUseProgram(program);
	return 0;
}

void MaterialGL::disable() {
	glUseProgram(0);
}

//int MaterialGL::updateAttribute(
//	ShaderType type,
//	std::string &attribName,
//	void* data,
//	unsigned int size)
//{
//	return 0;
//}
