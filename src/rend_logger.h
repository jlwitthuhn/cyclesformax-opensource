/* 
 * This file is part of Cycles for Max. (c) Jeffrey Witthuhn
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
 
#pragma once

/**
 * @file
 * @brief Defines classes to support logging within this plugin.
 */

#include <cstddef>
#include <fstream>
#include <memory>
#include <random>
#include <sstream>
#include <string>

#include "util_enums.h"
#include "util_simple_types.h"

/**
 * @brief Standard interface to support logging.
 */
class LoggerInterface {
public:
	LoggerInterface();
	virtual ~LoggerInterface();

	virtual bool enabled() const;
	virtual bool isReal() const { return false; }

	virtual LogNumberFormat number_format() const;
	virtual LogNumberFormat number_format(LogNumberFormat new_format);

	virtual LoggerInterface& operator<<(LogLevel input);
	virtual LoggerInterface& operator<<(LogCtl input);

	virtual LoggerInterface& operator<<(char input);
	virtual LoggerInterface& operator<<(const char* input);
	virtual LoggerInterface& operator<<(float input);
	virtual LoggerInterface& operator<<(int input);
	virtual LoggerInterface& operator<<(unsigned int input);
	virtual LoggerInterface& operator<<(unsigned long input);
	virtual LoggerInterface& operator<<(long long input);
	virtual LoggerInterface& operator<<(const wchar_t* input);
	virtual LoggerInterface& operator<<(void* input);

	virtual LoggerInterface& operator<<(size_t input);

	virtual LoggerInterface& operator<<(const std::string& input);

	virtual LoggerInterface& operator<<(Int2 input);
	virtual LoggerInterface& operator<<(IntRect input);

private:
	inline LoggerInterface& log_nothing() { return *this; }
};

/**
 * @brief LoggerInterface implementation to support logging details of a single component.
 */
class ComponentLogger : public LoggerInterface {
public:
	ComponentLogger(LogLevel log_level, std::wofstream log_stream);
	virtual ~ComponentLogger();

	virtual bool enabled() const;
	virtual bool isReal() const override { return true; }

	virtual LogNumberFormat number_format() const override;
	virtual LogNumberFormat number_format(LogNumberFormat new_format) override;

	virtual LoggerInterface& operator<<(LogLevel input) override;
	virtual LoggerInterface& operator<<(LogCtl input) override;

	virtual LoggerInterface& operator<<(char input) override;
	virtual LoggerInterface& operator<<(const char* input) override;
	virtual LoggerInterface& operator<<(float input) override;
	virtual LoggerInterface& operator<<(int input) override;
	virtual LoggerInterface& operator<<(unsigned int input) override;
	virtual LoggerInterface& operator<<(unsigned long input) override;
	virtual LoggerInterface& operator<<(long long input) override;
	virtual LoggerInterface& operator<<(const wchar_t* input) override;
	virtual LoggerInterface& operator<<(void* input);

	virtual LoggerInterface& operator<<(size_t input) override;

	virtual LoggerInterface& operator<<(const std::string& input) override;

	virtual LoggerInterface& operator<<(Int2 input) override;
	virtual LoggerInterface& operator<<(IntRect input) override;

private:
	void write_line();

	bool should_write_current_line() const;

	std::string get_level_string() const;
	std::string get_time_string() const;

	void apply_number_format();

	void reset_line_stream();

	const LogLevel log_level;
	std::wofstream log_stream;

	LogLevel line_level;
	std::wstringstream line_stream;

	LogNumberFormat num_format = LogNumberFormat::DECIMAL;
};

/**
 * @brief Class responsible for creating logger objects to be used throughout the plugin.
 */
class GlobalLogManager {
public:
	GlobalLogManager();
	void init();

	std::unique_ptr<LoggerInterface> new_logger(wchar_t* component_name, bool global = false, bool force_suffix = false);

	void new_render_begin(LogLevel log_level);

private:
	std::wstring generate_render_tag();
	std::wstring generate_process_tag();
	std::wstring generate_suffix_tag();

	std::minstd_rand rng;
	std::wstring process_tag;

	LogLevel log_level = LogLevel::DEBUG;
	std::wstring render_tag;

	std::unique_ptr<LoggerInterface> logger;
};

extern GlobalLogManager global_log_manager;
