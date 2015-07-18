/**
 * @file core/LogManager.h
 *
 * DevEngine
 * Copyright 2015 Eetu 'Devenec' Oinasmaa
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <core/Singleton.h>

namespace Core
{
	class LogManager final : public Singleton<LogManager>
	{
	public:

		LogManager() = default;

		LogManager(const LogManager& logManager) = delete;
		LogManager(LogManager&& logManager) = delete;

		~LogManager() = default;

		void deinitialise() const;

		void initialise() const;

		LogManager& operator =(const LogManager& logManager) = delete;
		LogManager& operator =(LogManager&& logManager) = delete;

	private:

		class Impl;
	};
}
