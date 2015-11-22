/**
 * @file platform/windows/WindowsFileStream.cpp
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

#include <core/FileStream.h>
#include <core/Log.h>
#include <core/Memory.h>
#include <core/debug/Assert.h>
#include <platform/windows/Windows.h>

using namespace Core;

// External

static const Char8* COMPONENT_TAG = "[Core::FileStream - Windows]";

static LARGE_INTEGER createLargeInteger(const Int64& value = 0);
static Uint32 getAccessMode(const OpenMode& openMode);
static Uint32 getCreationMode(const OpenMode& openMode);


// Implementation

class FileStream::Implementation final
{
public:

	Implementation()
		: _handle(nullptr),
		  _openMode(OpenMode()) { }

	Implementation(const Implementation& implementation) = delete;
	Implementation(Implementation&& implementation) = delete;

	~Implementation()
	{
		close();
	}

	void close()
	{
		if(isOpen())
		{
			if((_openMode & OpenMode::Write) == OpenMode::Write)
				flushBuffer();
			
			const Int32 result = CloseHandle(_handle);

			if(result == 0)
			{
				defaultLog << LogLevel::Error << ::COMPONENT_TAG << " Failed to close the file." << Log::Flush();
				DE_ERROR_WINDOWS(0x0);
			}

			_openMode = OpenMode();
			_handle = nullptr;
		}
	}

	Bool isAtEndOfFile() const
	{
		DE_ASSERT(isOpen());
		return position() >= size();
	}

	Bool isOpen() const
	{
		return _handle != nullptr;
	}

	void open(const String8& filepath, const OpenMode& openMode)
	{
		DE_ASSERT((openMode & OpenMode::Read) == OpenMode::Read || (openMode & OpenMode::Write) == OpenMode::Write);
		DE_ASSERT(!isOpen());
		const String16 filepath16 = toString16(filepath);
		const Uint32 accessMode = ::getAccessMode(openMode);
		const Uint32 creationMode = ::getCreationMode(openMode);

		_handle = CreateFileW(filepath16.c_str(), accessMode, FILE_SHARE_READ, nullptr, creationMode,
			FILE_ATTRIBUTE_NORMAL, nullptr);

		if(_handle == INVALID_HANDLE_VALUE)
		{
			defaultLog << LogLevel::Error << ::COMPONENT_TAG << " Failed to open file '" << filepath << "'." <<
				Log::Flush();

			DE_ERROR_WINDOWS(0x0);
		}

		SetLastError(0u);
		_openMode = openMode;
	}

	Int64 position() const
	{
		DE_ASSERT(isOpen());
		const LARGE_INTEGER offset = ::createLargeInteger();
		LARGE_INTEGER position;
		const Int32 result = SetFilePointerEx(_handle, offset, &position, FILE_CURRENT);

		if(result == 0)
		{
			defaultLog << LogLevel::Error << ::COMPONENT_TAG << " Failed to get the file pointer position." <<
				Log::Flush();

			DE_ERROR_WINDOWS(0x0);
		}

		return position.QuadPart;
	}

	Uint32 read(Byte* buffer, const Uint32 size) const
	{
		DE_ASSERT(buffer != nullptr);
		DE_ASSERT(isOpen());
		DE_ASSERT((_openMode & OpenMode::Read) == OpenMode::Read);
		unsigned long bytesRead;
		const Int32 result = ReadFile(_handle, buffer, size, &bytesRead, nullptr);

		if(result == 0)
		{
			defaultLog << LogLevel::Error << ::COMPONENT_TAG << " Failed to read the file." << Log::Flush();
			DE_ERROR_WINDOWS(0x0);
		}

		return bytesRead;
	}

	void seek(const Int64& position) const
	{
		seek(SeekPosition::Begin, position);
	}

	void seek(const SeekPosition& position, const Int64& offset) const
	{
		DE_ASSERT(isOpen());
		const LARGE_INTEGER seekOffset = ::createLargeInteger(offset);
		const Int32 result = SetFilePointerEx(_handle, seekOffset, nullptr, static_cast<Int32>(position));

		if(result == 0)
		{
			defaultLog << LogLevel::Error << ::COMPONENT_TAG << " Failed to seek the file." << Log::Flush();
			DE_ERROR_WINDOWS(0x0);
		}
	}

	Int64 size() const
	{
		DE_ASSERT(isOpen());
		LARGE_INTEGER size;
		const Int32 result = GetFileSizeEx(_handle, &size);

		if(result == 0)
		{
			defaultLog << LogLevel::Error << ::COMPONENT_TAG << " Failed to get the file size." << Log::Flush();
			DE_ERROR_WINDOWS(0x0);
		}

		return size.QuadPart;
	}

	Uint32 write(const Byte* data, const Uint32 size) const
	{
		DE_ASSERT(data != nullptr);
		DE_ASSERT(isOpen());
		DE_ASSERT((_openMode & OpenMode::Write) == OpenMode::Write);
		unsigned long bytesWritten;
		const Int32 result = WriteFile(_handle, data, size, &bytesWritten, nullptr);

		if(result == 0)
		{
			defaultLog << LogLevel::Error << ::COMPONENT_TAG << " Failed to write to the file." << Log::Flush();
			DE_ERROR_WINDOWS(0x0);
		}

		return bytesWritten;
	}

	Implementation& operator =(const Implementation& implementation) = delete;
	Implementation& operator =(Implementation&& implementation) = delete;

private:

	HANDLE _handle;
	OpenMode _openMode;

	void flushBuffer() const
	{
		const Int32 result = FlushFileBuffers(_handle);

		if(result == 0)
		{
			defaultLog << LogLevel::Error << ::COMPONENT_TAG << " Failed to flush the file buffer." << Log::Flush();
			DE_ERROR_WINDOWS(0x0);
		}
	}
};


// Core::FileStream

// Public

FileStream::FileStream()
	: _implementation(DE_NEW(Implementation)()) { }

FileStream::FileStream(const String8& filepath, const OpenMode& openMode)
	: FileStream()
{
	_implementation->open(filepath, openMode);
}

FileStream::~FileStream()
{
	DE_DELETE(_implementation, Implementation);
}

void FileStream::close() const
{
	_implementation->close();
}

Bool FileStream::isAtEndOfFile() const
{
	return _implementation->isAtEndOfFile();
}

Bool FileStream::isOpen() const
{
	return _implementation->isOpen();
}

void FileStream::open(const String8& filepath, const OpenMode& openMode) const
{
	_implementation->open(filepath, openMode);
}

Int64 FileStream::position() const
{
	return _implementation->position();
}

Uint32 FileStream::read(Byte* buffer, const Uint32 size) const
{
	return _implementation->read(buffer, size);
}

void FileStream::seek(const Int64& position) const
{
	_implementation->seek(position);
}

void FileStream::seek(const SeekPosition& position, const Int64& offset) const
{
	_implementation->seek(position, offset);
}

Int64 FileStream::size() const
{
	return _implementation->size();
}

Uint32 FileStream::write(const Byte* data, const Uint32 size) const
{
	return _implementation->write(data, size);
}


// External

static LARGE_INTEGER createLargeInteger(const Int64& value)
{
	LARGE_INTEGER integer;
	integer.QuadPart = value;

	return integer;
}

static Uint32 getAccessMode(const OpenMode& openMode)
{
	Uint32 mode = 0u;

	if((openMode & OpenMode::Read) == OpenMode::Read)
		mode |= GENERIC_READ;

	if((openMode & OpenMode::Write) == OpenMode::Write)
		mode |= GENERIC_WRITE;

	return mode;
}

static Uint32 getCreationMode(const OpenMode& openMode)
{
	Uint32 mode = 0u;

	if((openMode & OpenMode::Read) == OpenMode::Read)
		mode = OPEN_EXISTING;

	if((openMode & OpenMode::Write) == OpenMode::Write)
	{
		if((openMode & OpenMode::Truncate) == OpenMode::Truncate)
			mode = CREATE_ALWAYS;
		else
			mode = OPEN_ALWAYS;
	}

	return mode;
}
