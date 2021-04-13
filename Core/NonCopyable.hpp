#pragma once

/**
@brief Class to be used as superclass in order to make it's subclass non copyable.
**/
class NonCopyable
{
public:
	NonCopyable(const NonCopyable &) = delete;
	const NonCopyable &operator=(const NonCopyable &) = delete;

	NonCopyable(NonCopyable &&other) = default;
	NonCopyable &&operator=(NonCopyable &&other) { return std::move(other); }

protected:
	NonCopyable() = default;
	~NonCopyable() = default;
};
