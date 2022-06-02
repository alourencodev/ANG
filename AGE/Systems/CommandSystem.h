#pragma once

#include <Core/BuildScheme.hpp>
#include <Core/HashMap.hpp>
#include <Core/StackArray.hpp>


#ifdef AGE_RELEASE_DBG_INFO

namespace age
{

template<typename t_type>
class Function;

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
	using CommandMap = HashMap<const char *, Command*>;
	CommandMap _commandMap;
};

static CommandSystem s_commandSystem;

}

#define AGE_DECLARE_COMMAND(STR, FUNCTION)				\
	static const char FUNCTION ## _str[] = STR;			\
	static CommandSystem::Command FUNCTION ## _func
	
#define AGE_ADD_COMMAND(FUNCTION)																\
	age_assertFatal(FUNCTION ## _str[0] == '-', k_commandNameAssertMessage, FUNCTION ## _str);	\
	FUNCTION ## _func = Command(FUNCTION);														\
	_commandMap.add(FUNCTION ## _str, &FUNCTION ## _func)

#else

#define AGE_DECLARE_COMMAND(STR, FUNCTION)
#define AGE_ADD_COMMAND(FUNCTION)

#endif // AGE_RELEASE_DBG_INFO

