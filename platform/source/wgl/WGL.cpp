/**
 * @file platform/wgl/WGL.cpp
 *
 * DevEngine
 * Copyright 2015 Eetu 'Devenec' Oinasmaa
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

#include <core/Error.h>
#include <core/Log.h>
#include <graphics/LogUtility.h>
#include <platform/GraphicsExtensionHelper.h>
#include <platform/wgl/WGL.h>
#include <platform/wgl/WGLGraphicsContextBase.h>
#include <platform/windows/Windows.h>

using namespace Core;
using namespace Graphics;
using namespace Platform;

// External

static const Char8* COMPONENT_TAG = "[Platform::WGL]";

static void checkExtensions();
static void getExtensionFunctions();
static ExtensionNameList getExtensionNames(const GraphicsContextBase& graphicsContext);


// Public

// Standard

WGL::CreateContext WGL::createContext = wglCreateContext;
WGL::DeleteContext WGL::deleteContext = wglDeleteContext;
WGL::GetCurrentContext WGL::getCurrentContext = wglGetCurrentContext;
WGL::GetProcAddress WGL::getProcAddress = wglGetProcAddress;
WGL::MakeCurrent WGL::makeCurrent = wglMakeCurrent;

// WGL_ARB_create_context

WGL::CreateContextAttribsARB WGL::createContextAttribsARB = nullptr;

// WGL_ARB_extensions_string

WGL::GetExtensionsStringARB WGL::getExtensionsStringARB = nullptr;

// WGL_ARB_pixel_format

WGL::ChoosePixelFormatARB WGL::choosePixelFormatARB = nullptr;
WGL::GetPixelFormatAttribFVARB WGL::getPixelFormatAttribfvARB = nullptr;
WGL::GetPixelFormatAttribIVARB WGL::getPixelFormatAttribivARB = nullptr;


// Static

void WGL::initialise(const GraphicsContextBase& graphicsContext)
{
	::getExtensionFunctions();
	::checkExtensions();
	// TODO: check current context state?
	const ExtensionNameList extensionNames = ::getExtensionNames(graphicsContext);
	logGraphicsExtensions("graphics context", extensionNames);
}


// Platform::GraphicsExtensionHelper

// Private

// Static

GraphicsExtensionHelper::Function GraphicsExtensionHelper::getFunctionInternal(const Char8* name)
{
	return reinterpret_cast<Function>(WGL::getProcAddress(name));
}


// External

static void checkExtensions()
{
	if(WGL::getExtensionsStringARB == nullptr)
	{
		defaultLog << LogLevel::Error << ::COMPONENT_TAG << " Graphics context extensions are not supported." <<
			Log::Flush();

		DE_ERROR(0x0);
	}
}

static void getExtensionFunctions()
{
	// WGL_ARB_create_context

	WGL::createContextAttribsARB =
		GraphicsExtensionHelper::getFunction<WGL::CreateContextAttribsARB>("wglCreateContextAttribsARB");

	// WGL_ARB_extensions_string

	WGL::getExtensionsStringARB =
		GraphicsExtensionHelper::getFunction<WGL::GetExtensionsStringARB>("wglGetExtensionsStringARB");

	// WGL_ARB_pixel_format

	WGL::choosePixelFormatARB =
		GraphicsExtensionHelper::getFunction<WGL::ChoosePixelFormatARB>("wglChoosePixelFormatARB");

	WGL::getPixelFormatAttribfvARB =
		GraphicsExtensionHelper::getFunction<WGL::GetPixelFormatAttribFVARB>("wglGetPixelFormatAttribfvARB");

	WGL::getPixelFormatAttribivARB =
		GraphicsExtensionHelper::getFunction<WGL::GetPixelFormatAttribIVARB>("wglGetPixelFormatAttribivARB");
}

static ExtensionNameList getExtensionNames(const GraphicsContextBase& graphicsContext)
{
	const String8 extensionNamesString(WGL::getExtensionsStringARB(graphicsContext.deviceContextHandle()));
	ExtensionNameList extensionNames;

	if(extensionNamesString.length() != 0u)
	{
		Uint32 currentPosition = 0u;
		Uint32 spacePosition;

		while((spacePosition = extensionNamesString.find(' ', currentPosition)) != String8::npos)
		{
			extensionNames.push_back(extensionNamesString.substr(currentPosition, spacePosition - currentPosition));
			currentPosition = spacePosition + 1u;
		}
	}

	return extensionNames;
}
