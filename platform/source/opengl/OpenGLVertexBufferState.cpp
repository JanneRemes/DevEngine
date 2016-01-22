/**
 * @file platform/opengl/OpenGLVertexBufferState.cpp
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

#include <core/Memory.h>
#include <core/debug/Assert.h>
#include <graphics/IndexBuffer.h>
#include <graphics/VertexElement.h>
#include <platform/opengl/OpenGL.h>
#include <platform/opengl/OpenGLGraphicsBuffer.h>
#include <platform/opengl/OpenGLGraphicsEnumerations.h>
#include <platform/opengl/OpenGLVertexBufferState.h>

using namespace Core;
using namespace Graphics;
using namespace Platform;

// External

static Uint32 getVertexElementComponentCount(const VertexElement& element, Bool& normalise);
static Uint32 getVertexElementSize(const VertexElement& element);


// Implementation

// Public

VertexBufferState::Implementation::Implementation()
	: _indexBuffer(nullptr),
	  _vertexArrayHandle(0u)
{
	OpenGL::genVertexArrays(1, &_vertexArrayHandle);
	DE_CHECK_ERROR_OPENGL();
}

VertexBufferState::Implementation::~Implementation()
{
	OpenGL::deleteVertexArrays(1, &_vertexArrayHandle);
	DE_CHECK_ERROR_OPENGL();
}

void VertexBufferState::Implementation::setIndexBuffer(IndexBuffer* buffer)
{
	bind();

	if(buffer == nullptr)
		_indexBuffer->_implementation->debind();
	else
		buffer->_implementation->bind();

	debind();
	_indexBuffer = buffer;
}

void VertexBufferState::Implementation::setVertexBuffer(const GraphicsBuffer* buffer,
	const VertexElementList& vertexElements, const Uint32 stride, const Uint offset) const
{
	DE_ASSERT(buffer->_implementation->binding() == static_cast<Uint32>(BufferBinding::Vertex));
	Uint32 bufferHandle = 0u;

	if(buffer != nullptr)
		bufferHandle = buffer->_implementation->handle();

	bind();
	buffer->_implementation->bind();
	setVertexLayout(vertexElements, stride, offset);
	buffer->_implementation->debind();
	debind();
}

// Private

void VertexBufferState::Implementation::bind(const Uint32 vertexArrayHandle) const
{
	OpenGL::bindVertexArray(vertexArrayHandle);
	DE_CHECK_ERROR_OPENGL();
}

void VertexBufferState::Implementation::setVertexLayout(const VertexElementList& vertexElements,
	const Uint32 stride, const Uint bufferOffset) const
{
	Uint elementOffset = bufferOffset;

	for(VertexElementList::const_iterator i = vertexElements.begin(), end = vertexElements.end();
		i != end; ++i)
	{
		if(i->offset != VertexElement::AFTER_PREVIOUS)
			elementOffset += i->offset;

		setVertexElementFormat(*i, elementOffset, stride);
		elementOffset += ::getVertexElementSize(*i);
	}
}

void VertexBufferState::Implementation::setVertexElementFormat(const VertexElement& element,
	const Uint elementOffset, const Uint32 stride) const
{
	Bool normalise;
	const Uint32 componentCount = ::getVertexElementComponentCount(element, normalise);
	const Uint32 elementType = static_cast<Uint32>(element.type) >> 3;
	OpenGL::enableVertexAttribArray(element.index);
	DE_CHECK_ERROR_OPENGL();
	const Void* bufferOffset = reinterpret_cast<Void*>(elementOffset);
	OpenGL::vertexAttribPointer(element.index, componentCount, elementType, normalise, stride, bufferOffset);
	DE_CHECK_ERROR_OPENGL();
}


// Graphics::VertexBufferState

// Public

void VertexBufferState::setIndexBuffer(IndexBuffer* indexBuffer) const
{
	return _implementation->setIndexBuffer(indexBuffer);
}

void VertexBufferState::setVertexBuffer(const GraphicsBuffer* buffer, const VertexElementList& vertexElements,
	const Uint32 stride, const Uint offset) const
{
	_implementation->setVertexBuffer(buffer, vertexElements, stride, offset);
}

// Private

VertexBufferState::VertexBufferState(GraphicsInterfaceHandle graphicsInterfaceHandle)
	: _implementation(nullptr)
{
	static_cast<Void>(graphicsInterfaceHandle);
	_implementation = DE_NEW(Implementation)();
}

VertexBufferState::~VertexBufferState()
{
	DE_DELETE(_implementation, Implementation);
}


// External

static Uint32 getVertexElementComponentCount(const VertexElement& element, Bool& normalise)
{
	normalise = element.normalise;
	Uint32 componentCount = static_cast<Uint32>(element.type) & 0x07;

	if(componentCount > 4u)
	{
		--componentCount;
		normalise = true;
	}

	if(componentCount == 5u)
		componentCount = OpenGL::BGRA;

	return componentCount;
}

static Uint32 getVertexElementSize(const VertexElement& element)
{
	switch(element.type)
	{
		case VertexElementType::Float16:
		case VertexElementType::Int8Vector2:
		case VertexElementType::Int16:
		case VertexElementType::Uint8Vector2:
		case VertexElementType::Uint16:
			return 2u;

		case VertexElementType::Float16Vector2:
		case VertexElementType::Float32:
		case VertexElementType::Int8Vector4:
		case VertexElementType::Int16Vector2:
		case VertexElementType::Int32:
		case VertexElementType::Int32_B10G10R10A2:
		case VertexElementType::Int32_R10G10B10A2:
		case VertexElementType::Uint8Vector4:
		case VertexElementType::Uint16Vector2:
		case VertexElementType::Uint32:
		case VertexElementType::Uint32_B10G10R10A2:
		case VertexElementType::Uint32_R10G10B10A2:
		case VertexElementType::Uint32_R11G11B10_Float:
			return 4u;

		case VertexElementType::Float16Vector3:
		case VertexElementType::Int16Vector3:
		case VertexElementType::Uint16Vector3:
			return 6u;

		case VertexElementType::Float16Vector4:
		case VertexElementType::Float32Vector2:
		case VertexElementType::Float64:
		case VertexElementType::Int16Vector4:
		case VertexElementType::Int32Vector2:
		case VertexElementType::Uint16Vector4:
		case VertexElementType::Uint32Vector2:
			return 8u;

		case VertexElementType::Float32Vector3:
		case VertexElementType::Int32Vector3:
		case VertexElementType::Uint32Vector3:
			return 12u;

		case VertexElementType::Float32Vector4:
		case VertexElementType::Float64Vector2:
		case VertexElementType::Int32Vector4:
		case VertexElementType::Uint32Vector4:
			return 16u;

		case VertexElementType::Float64Vector3:
			return 24u;

		case VertexElementType::Float64Vector4:
			return 32u;

		case VertexElementType::Int8:
		case VertexElementType::Uint8:
			return 1u;

		case VertexElementType::Int8Vector3:
		case VertexElementType::Uint8Vector3:
			return 3u;

		default:
			return 0u;
	}
}
