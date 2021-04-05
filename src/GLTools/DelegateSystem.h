/*  Copyright 2021 Pierre Even,
      co-author of paper:
      Even, P., Grzesznik, A., Gebhardt, A., Chenal, T., Even, P. and Ngo, P.,
      2021,
      Fast extraction of linear structures fromLiDAR raw data
      for archaeomorphological structure prospection.
      In the International Archives of the Photogrammetry, Remote Sensing
      and Spatial Information Sciences (proceedings of the 2021 edition
      of the XXIVth ISPRS Congress).

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once
#include <vector>

#define DECLARE_DELEGATE_SINGLECAST(name, ...) using name =  DelegateSingleCast<void, __VA_ARGS__>;
#define DECLARE_DELEGATE_SINGLECAST_RETURN(name, returnType, ...) typedef DelegateSingleCast<returnType, __VA_ARGS__> name;
#define DECLARE_DELEGATE_MULTICAST(name, ...) using name = DelegateMultiCast<__VA_ARGS__>;


template<typename R, typename... Args>
class DelegateFunctionPtrWrapper {
public:
	inline virtual R Execute(Args&...) = 0;
	inline virtual bool operator==(const void*) const = 0;
};


template<typename Class, typename R, typename... Args>
class DelegateFunctionPtr : public DelegateFunctionPtrWrapper<R, Args...> {
	Class* objectPtr;
	R (Class::* functionPtr)(Args...);

public:

	DelegateFunctionPtr(Class* objPtr, R (Class::* funcPtr)(Args...)) : objectPtr(objPtr), functionPtr(funcPtr) {}

	inline R Execute(Args&... inArgs) { return (objectPtr->*functionPtr)(inArgs...); }

	inline bool operator==(const void* objPtr) const { return objectPtr == objPtr; }
};


template<typename R, typename... Args>
class DelegateSingleCast
{
	DelegateFunctionPtrWrapper<R, Args...>* functionPtr = nullptr;

public:

	template <typename Class>
	inline void Bind(Class* inObjPtr, R (Class::* inFunc)(Args...)) {
		if (functionPtr)
			delete functionPtr;
		functionPtr = new DelegateFunctionPtr<Class, R, Args...>(inObjPtr, inFunc);
	}

	inline void Clear() {
		if (functionPtr) {
			delete functionPtr;
			functionPtr = nullptr;
		}
	}

	inline R Execute(Args&... inArgs) {
		assert(functionPtr);
		return functionPtr->Execute(inArgs...);
	}
};

template<typename... Args>
class DelegateMultiCast
{
	std::vector<DelegateFunctionPtrWrapper<void, Args...>*> functions;

public:

	template <typename Class>
	inline void Add(Class* inObjPtr, void(Class::* inFunc)(Args...)) {
		functions.push_back(new DelegateFunctionPtr<Class, void, Args...>(inObjPtr, inFunc));
	}

	inline void Clear() {
		for (const auto& fct : functions)
		{
			delete fct;
		}
		functions.clear();
	}

	inline void UnbindObj(void* ObjPtr)
	{
		for (int64_t i = functions.size() - 1; i >= 0; --i)
		{
			if (*functions[i] == ObjPtr) functions.erase(functions.begin() + i);
		}
	}

	inline void Execute(Args&... inArgs) {
		for (const auto& fct : functions)
		{
			fct->Execute(inArgs...);
		}
	}
};
