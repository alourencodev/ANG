#pragma once

#include <Core/Log/Assert.hpp>

namespace age
{

template<typename t_type>
class Function;

template<typename t_returnType, typename... t_argsTypes>
class Function<t_returnType(t_argsTypes...)>
{
public:
	Function() = default;

	template<typename t_type>
	Function(t_type t)
	{
		_callable = new Callable<t_type>(t);
	}

	~Function()
	{
		if (_callable != nullptr)
			delete _callable;
	}

	template<typename t_type>
	Function &operator = (t_type t)
	{
		if (_callable != nullptr)
			delete _callable;

		_callable = new Callable<t_type>(t);
		return *this;
	}

	t_returnType operator()(t_argsTypes... args) const
	{
		age_assertFatal(_callable != nullptr, "Function has no valid callable. Make sure it is correctly initialized or that the defined callable is still alive.");
		return _callable->invoke(args...);
	}

	bool isValid() const { return _callable != nullptr; }

private:
	class ICallable
	{
	public:
		virtual ~ICallable() = default;
		virtual t_returnType invoke(t_argsTypes... args) = 0;
	};

	template <typename t_type>
	class Callable : public ICallable
	{
	public:
		Callable(const t_type &t) : _t(t) {}
		~Callable() override = default;

		t_returnType invoke(t_argsTypes... args) override
		{
			return _t(args...);
		}

	private:
		t_type _t;
	};

	ICallable *_callable = nullptr;
};

}
