
#include <AGE/Systems/CommandSystem.h>

#include <Core/Function.hpp>
#include <Core/StackArray.hpp>
#include <Core/StringView.hpp>

namespace age
{

constexpr const char k_commandNameAssertMessage[] = "Command %s doesn't start with '-'. Every command must start with '-'";

#define AGE_DECLARE_COMMAND(STR, FUNCTION)				\
	static const char FUNCTION ## _str[] = STR;			\
	static CommandSystem::Command FUNCTION ## _func
	
#define AGE_ADD_COMMAND(FUNCTION)																\
	age_assertFatal(FUNCTION ## _str[0] == '-', k_commandNameAssertMessage, FUNCTION ## _str);	\
	FUNCTION ## _func = Command(FUNCTION);														\
	_commandMap.add(FUNCTION ## _str, &FUNCTION ## _func)


constexpr u8 k_preallocatedCommandMapCapacity = 8;

void enableLogCommand(const CommandSystem::CommandArgs &args)
{
	for (const char *tag : args)
		age::logger::enable(tag);
}

AGE_DECLARE_COMMAND("-enableLog", enableLogCommand);

CommandSystem::CommandSystem()
{
	_commandMap = CommandMap(k_preallocatedCommandMapCapacity);
}

void CommandSystem::init()
{
	AGE_ADD_COMMAND(enableLogCommand);
}

bool CommandSystem::runCommand(const char *commandKey, const CommandArgs &args)
{
	const Command *const *commandPtr = _commandMap.at(commandKey);

	if (commandPtr == nullptr)
		return false;

	(**commandPtr)(args);
	return true;
}

}

