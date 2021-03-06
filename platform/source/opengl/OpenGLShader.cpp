/**
 * @file platform/opengl/OpenGLShader.cpp
 *
 * DevEngine
 * Copyright 2015-2016 Eetu 'Devenec' Oinasmaa
 *
 * DevEngine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * DevEngine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DevEngine. If not, see <http://www.gnu.org/licenses/>.
 */

#include <core/Array.h>
#include <core/Error.h>
#include <core/Log.h>
#include <core/Memory.h>
#include <core/debug/Assert.h>
#include <platform/opengl/OpenGL.h>
#include <platform/opengl/OpenGLShader.h>

using namespace Core;
using namespace Graphics;
using namespace Platform;

// External

static const Char8* COMPONENT_TAG = "[Graphics::Shader - OpenGL] ";

static const Array<const Char8*, 6u> SHADER_TYPE_NAMES
{{
	"compute",
	"fragment",
	"geometry",
	"tessellation control",
	"tessellation evaluation"
	"vertex"
}};

static Uint32 getShaderTypeId(const ShaderType& shaderType);


// Implementation

// Public

Shader::Implementation::Implementation(const ShaderType& type, const ByteList& shaderCode)
	: _shaderHandle(0u)
{
	createShader(type);
	compileShader(reinterpret_cast<const Char8*>(shaderCode.data()));
	checkCompilationStatus();
}

Shader::Implementation::~Implementation()
{
	OpenGL::deleteShader(_shaderHandle);
	DE_CHECK_ERROR_OPENGL();
}

// Private

void Shader::Implementation::createShader(const ShaderType& type)
{
	_shaderHandle = OpenGL::createShader(::getShaderTypeId(type));
	DE_CHECK_ERROR_OPENGL();

	if(_shaderHandle == 0u)
	{
		defaultLog << LogLevel::Error << ::COMPONENT_TAG << "Failed to create the " <<
			::SHADER_TYPE_NAMES[static_cast<Uint32>(type)] << " shader." << Log::Flush();

		DE_ERROR(0x0);
	}
}

void Shader::Implementation::compileShader(const Char8* shaderSource) const
{
	OpenGL::shaderSource(_shaderHandle, 1, &shaderSource, nullptr);
	DE_CHECK_ERROR_OPENGL();
	OpenGL::compileShader(_shaderHandle);
	DE_CHECK_ERROR_OPENGL();
}

void Shader::Implementation::checkCompilationStatus() const
{
	const Int32 compilationStatus = getParameter(OpenGL::COMPILE_STATUS);

	if(compilationStatus == OpenGL::FALSE)
	{
		outputCompilerFailureLog();
		DE_ERROR(0x0);
	}
	else
	{
		outputCompilerSuccessLog();
	}
}

Int32 Shader::Implementation::getParameter(const Uint32 parameterName) const
{
	Int32 parameter;
	OpenGL::getShaderiv(_shaderHandle, parameterName, &parameter);
	DE_CHECK_ERROR_OPENGL();

	return parameter;
}

void Shader::Implementation::outputCompilerFailureLog() const
{
	defaultLog << LogLevel::Error << ::COMPONENT_TAG << "Failed to compile the shader:";
	const Int32 logLength = getParameter(OpenGL::INFO_LOG_LENGTH);

	if(logLength > 1)
		defaultLog << '\n' << getInfoLog(logLength).data();
	else
		defaultLog << " No compiler log available.";

	defaultLog << Log::Flush();
}

void Shader::Implementation::outputCompilerSuccessLog() const
{
	const Int32 logLength = getParameter(OpenGL::INFO_LOG_LENGTH);

	if(logLength > 1)
	{
		defaultLog << LogLevel::Warning << ::COMPONENT_TAG << "The shader compiled with warning(s):\n" <<
			getInfoLog(logLength).data() << Log::Flush();
	}
}

Shader::Implementation::CharacterBuffer Shader::Implementation::getInfoLog(const Uint32 logLength) const
{
	CharacterBuffer logBuffer(logLength);

	OpenGL::getShaderInfoLog(_shaderHandle, static_cast<Int32>(logBuffer.size()), nullptr,
		logBuffer.data());

	DE_CHECK_ERROR_OPENGL();
	return logBuffer;
}


// Graphics::Shader

// Private

Shader::Shader(GraphicsInterfaceHandle graphicsInterfaceHandle, const ShaderType& type,
	const ByteList& shaderCode)
	: _implementation(nullptr)
{
	static_cast<Void>(graphicsInterfaceHandle);
	_implementation = DE_NEW(Implementation)(type, shaderCode);
}

Shader::~Shader()
{
	DE_DELETE(_implementation, Implementation);
}


// External

static Uint32 getShaderTypeId(const ShaderType& shaderType)
{
	switch(shaderType)
	{
		case ShaderType::Compute:
			return OpenGL::COMPUTE_SHADER;

		case ShaderType::Fragment:
			return OpenGL::FRAGMENT_SHADER;

		case ShaderType::Geometry:
			return OpenGL::GEOMETRY_SHADER;

		case ShaderType::TessellationControl:
			return OpenGL::TESS_CONTROL_SHADER;

		case ShaderType::TessellationEvaluation:
			return OpenGL::TESS_EVALUATION_SHADER;

		case ShaderType::Vertex:
			return OpenGL::VERTEX_SHADER;

		default:
			return 0u;
	}
}
