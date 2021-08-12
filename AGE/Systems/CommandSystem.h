#pragma once

#include <Core/BuildScheme.hpp>
#include <Core/StringMap.hpp>


namespace age
{
#ifdef AGE_RELEASE_DBG_INFO

template<typename t_type, size_t t_size>
class StackArray;

template<typename t_type>
class Function;

class ConstWeakStringView;

class CommandSystem
{
public:
	static constexpr u8 k_maxCommandArguments = 16;
	using CommandArgs = StackArray<const char *, k_maxCommandArguments>;
	using Command = Function<void(const CommandArgs &)>;

	CommandSystem();
	~CommandSystem() = default;

	void init();
	bool runCommand(const char *commandKey, const CommandArgs &args);

private:
	using CommandMap = StringMap<Command *>;
	CommandMap _commandMap;
};

static CommandSystem s_commandSystem;

#endif
}
