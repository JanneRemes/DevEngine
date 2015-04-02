/**
 * @file graphics/Window.h
 *
 * DevEngine
 * Copyright 2015 Eetu 'Devenec' Oinasmaa
 */

#pragma once

#include <core/String.h>
#include <core/Types.h>

namespace Graphics
{
	using WindowHandle = void*;

	class Window final
	{
	public:

		WindowHandle handle() const;

		Bool processMessages() const;

		void setTitle(const Core::String8& title) const;

		void show() const;

	private:

		friend class WindowManager;

		class Impl;

		Impl* _impl;

		Window(WindowHandle handle);
		Window(const Window& window) = delete;
		Window(Window&& window) = delete;
		~Window();

		Window& operator =(const Window& window) = delete;
		Window& operator =(Window&& window) = delete;
	};
}