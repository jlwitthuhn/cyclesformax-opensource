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
 
#include "rend_logger.h"

#include <chrono>
#include <iomanip>
#include <sstream>

#include "util_windows.h"

extern bool enable_plugin_debug;

static constexpr int PROCESS_TAG_LENGTH = 4;
static constexpr int RENDER_TAG_LENGTH = 5;

GlobalLogManager global_log_manager;

LoggerInterface::LoggerInterface()
{

}

LoggerInterface::~LoggerInterface()
{

}

bool LoggerInterface::enabled() const
{
	return false;
}

LogNumberFormat LoggerInterface::number_format() const
{
	return LogNumberFormat::DECIMAL;
}

LogNumberFormat LoggerInterface::number_format(const LogNumberFormat)
{
	return LogNumberFormat::DECIMAL;
}

LoggerInterface& LoggerInterface::operator<<(const LogLevel)
{
	return log_nothing();
}

LoggerInterface& LoggerInterface::operator<<(const LogCtl)
{
	return log_nothing();
}

LoggerInterface& LoggerInterface::operator<<(const char)
{
	return log_nothing();
}

LoggerInterface& LoggerInterface::operator<<(const char* const)
{
	return log_nothing();
}

LoggerInterface& LoggerInterface::operator<<(const float)
{
	return log_nothing();
}

LoggerInterface& LoggerInterface::operator<<(const int)
{
	return log_nothing();
}

LoggerInterface& LoggerInterface::operator<<(const unsigned int)
{
	return log_nothing();
}

LoggerInterface& LoggerInterface::operator<<(const unsigned long)
{
	return log_nothing();
}

LoggerInterface& LoggerInterface::operator<<(const long long)
{
	return log_nothing();
}

LoggerInterface& LoggerInterface::operator<<(const wchar_t* const)
{
	return log_nothing();
}

LoggerInterface& LoggerInterface::operator<<(void* const)
{
	return log_nothing();
}

LoggerInterface& LoggerInterface::operator<<(const size_t)
{
	return log_nothing();
}

LoggerInterface& LoggerInterface::operator<<(const std::string&)
{
	return log_nothing();
}

LoggerInterface& LoggerInterface::operator<<(const Int2)
{
	return log_nothing();
}

LoggerInterface& LoggerInterface::operator<<(const IntRect)
{
	return log_nothing();
}

ComponentLogger::ComponentLogger(const LogLevel log_level, std::wofstream log_stream) :
	log_level(log_level),
	log_stream(std::move(log_stream)),
	line_level(LogLevel::INFO)
{

}

ComponentLogger::~ComponentLogger()
{

}

bool ComponentLogger::enabled() const
{
	return true;
}

LogNumberFormat ComponentLogger::number_format() const
{
	return num_format;
}

LogNumberFormat ComponentLogger::number_format(const LogNumberFormat new_format)
{
	const LogNumberFormat initial_format = num_format;
	num_format = new_format;
	apply_number_format();
	return initial_format;
}

LoggerInterface& ComponentLogger::operator<<(const LogLevel input)
{
	line_level = input;
	return *this;
}

LoggerInterface& ComponentLogger::operator<<(const LogCtl input)
{
	switch (input) {
		case LogCtl::SEPARATOR:
		{
			const LogLevel initial_level = line_level;
			*this << LogLevel::ALWAYS;
			reset_line_stream();
			line_stream << "--------------------------------";
			*this << LogCtl::WRITE_LINE;
			*this << initial_level;
			break;
		}
		case LogCtl::WRITE_LINE:
			write_line();
			break;
	}
	return *this;
}

LoggerInterface& ComponentLogger::operator<<(const char input)
{
	line_stream << input;
	return *this;
}

LoggerInterface& ComponentLogger::operator<<(const char* const input)
{
	line_stream << input;
	return *this;
}

LoggerInterface& ComponentLogger::operator<<(const float input)
{
	line_stream << input;
	return *this;
}

LoggerInterface& ComponentLogger::operator<<(const int input)
{
	line_stream << input;
	return *this;
}

LoggerInterface& ComponentLogger::operator<<(const unsigned int input)
{
	line_stream << input;
	return *this;
}

LoggerInterface& ComponentLogger::operator<<(const unsigned long input)
{
	line_stream << input;
	return *this;
}

LoggerInterface& ComponentLogger::operator<<(const long long input)
{
	line_stream << input;
	return *this;
}

LoggerInterface& ComponentLogger::operator<<(const wchar_t* const input)
{
	line_stream << input;
	return *this;
}

LoggerInterface& ComponentLogger::operator<<(void* const input)
{
	line_stream << input;
	return *this;
}

LoggerInterface& ComponentLogger::operator<<(const size_t input)
{
	line_stream << input;
	return *this;
}

LoggerInterface& ComponentLogger::operator<<(const std::string& input)
{
	*this << input.c_str();
	return *this;
}

LoggerInterface& ComponentLogger::operator<<(const Int2 input)
{
	line_stream << "(" << input.x() << ", " << input.y() << ")";
	return *this;
}

LoggerInterface& ComponentLogger::operator<<(const IntRect input)
{
	*this << "IntRect(" << input.begin() << ", " << input.end() << ")";
	return *this;
}

void ComponentLogger::write_line()
{
	if (should_write_current_line() == false) {
		line_stream.clear();
		return;
	}
	const std::string time_str = get_time_string();
	const std::string level_str = get_level_string();
	const std::wstring line_content = line_stream.str();
	reset_line_stream();
	log_stream << time_str.c_str() << ' ' << level_str.c_str() << ' ' << line_content << std::endl;
	*this << LogLevel::INFO;
}

bool ComponentLogger::should_write_current_line() const
{
	return (line_level <= log_level);
}

std::string ComponentLogger::get_level_string() const
{
	switch (line_level) {
		case LogLevel::ALWAYS:
			return "[LOGG]";
		case LogLevel::ERR:
			return "[ERRO]";
		case LogLevel::WARN:
			return "[WARN]";
		case LogLevel::INFO:
			return "[INFO]";
		case LogLevel::DEBUG:
			return "[DEBG]";
	}
	return "[UKWN]";
}

std::string ComponentLogger::get_time_string() const
{
	const long long time_ns = std::chrono::steady_clock::now().time_since_epoch().count();
	const long long time_ms = time_ns / (1'000'000L);
	const long long time_s = time_ms / 1000L;

	const int ms_portion = time_ms % 1000L;
	const int s_portion = time_s % 10'000L;

	std::ostringstream out_stream;
	out_stream << '['
		<< std::setw(4) << std::setfill('0') << s_portion
		<< '.'
		<< std::setw(3) << ms_portion
		<< std::setw(0) << ']';

	return out_stream.str();
}

void ComponentLogger::apply_number_format()
{
	switch (num_format) {
		case LogNumberFormat::OCTAL:
			line_stream << std::setbase(8);
			break;
		case LogNumberFormat::HEX:
			line_stream << std::setbase(16);
			break;
		default:
			line_stream << std::setbase(10);
			break;
	}
}

void ComponentLogger::reset_line_stream()
{
	line_stream.str(L"");
	line_stream.clear();
	apply_number_format();
}

GlobalLogManager::GlobalLogManager()
{
	const long long current_time_ns = std::chrono::steady_clock::now().time_since_epoch().count();
	rng.seed(static_cast<unsigned int>(current_time_ns));
	process_tag = generate_process_tag();
	new_render_begin(LogLevel::DEBUG);

	logger = std::make_unique<LoggerInterface>();
}

void GlobalLogManager::init()
{
	if (enable_plugin_debug && logger->isReal() == false)
	{
		logger = new_logger(L"LogManager", true);
		*logger << LogCtl::SEPARATOR;
	}
	else
	{
		*logger << "extra init" << LogCtl::WRITE_LINE;
	}
}

std::unique_ptr<LoggerInterface> GlobalLogManager::new_logger(wchar_t* const component_name, const bool global, const bool force_suffix)
{
	*logger << L"new_logger called..." << LogCtl::WRITE_LINE;
	*logger << L"component: " << component_name << LogCtl::WRITE_LINE;

	std::unique_ptr<LoggerInterface> result = std::make_unique<LoggerInterface>();
	if (enable_plugin_debug == false) {
		// Debug is off, no need to log reason as there is no logging
		return result;
	}

	const std::wstring top_log_dir = get_user_dir() + L"\\CyclesMaxLog";
	create_directory(top_log_dir);
	const std::wstring proc_log_dir = top_log_dir + L"\\" + process_tag;
	if (global) {
		*logger << L"creating global logger" << LogCtl::WRITE_LINE;
		const std::wstring global_log_dir = proc_log_dir + L"\\Global";
		const std::wstring log_file_path = global_log_dir + L"\\" + component_name + generate_suffix_tag() + L".txt";
		create_directory(proc_log_dir);
		create_directory(global_log_dir);
		std::wofstream stream(log_file_path, std::wofstream::app);
		result = std::make_unique<ComponentLogger>(log_level, std::move(stream));
	}
	else {
		*logger << L"creating render-specific logger" << LogCtl::WRITE_LINE;
		std::wstring filename_suffix = L"";
		if (force_suffix) {
			filename_suffix = generate_render_tag();
		}
		const std::wstring session_log_dir = proc_log_dir + L"\\" + render_tag;
		const std::wstring log_file_path = session_log_dir + L"\\" + component_name + generate_suffix_tag() + L".txt";
		create_directory(proc_log_dir);
		create_directory(session_log_dir);
		std::wofstream stream(log_file_path, std::wofstream::app);
		result = std::make_unique<ComponentLogger>(log_level, std::move(stream));
	}

	*logger << L"returning result" << LogCtl::WRITE_LINE;
	return result;
}

void GlobalLogManager::new_render_begin(const LogLevel log_level)
{
	this->log_level = log_level;
	this->render_tag = generate_render_tag();
}

std::wstring GlobalLogManager::generate_render_tag()
{
	constexpr int NUM_CHARACTERS = 26;
	const char LETTERS_BEGIN = 'A';

	std::wostringstream tag_stream;
	for (int i = 0; i < RENDER_TAG_LENGTH; i++) {
		const char offset = rng() % NUM_CHARACTERS;
		tag_stream << static_cast<wchar_t>(LETTERS_BEGIN + offset);
	}

	return std::wstring(L"Rend") + tag_stream.str();
}

std::wstring GlobalLogManager::generate_process_tag()
{
	constexpr int NUM_CHARACTERS = 26;
	const char LETTERS_BEGIN = 'A';

	std::wostringstream tag_stream;
	for (int i = 0; i < PROCESS_TAG_LENGTH; i++) {
		const char offset = rng() % NUM_CHARACTERS;
		tag_stream << static_cast<wchar_t>(LETTERS_BEGIN + offset);
	}

	return std::wstring(L"Proc") + tag_stream.str();
}

std::wstring GlobalLogManager::generate_suffix_tag()
{
	constexpr int NUM_CHARACTERS = 26;
	const char LETTERS_BEGIN = 'A';

	std::wostringstream tag_stream;
	for (int i = 0; i < 5; i++) {
		const char offset = rng() % NUM_CHARACTERS;
		tag_stream << static_cast<wchar_t>(LETTERS_BEGIN + offset);
	}

	return std::wstring(L"_") + tag_stream.str();
}
