#pragma once

template<typename T>
bool CreateComponent(GameObject* obj, SERIALIZE_INPUT& ar)
{
	obj->AddComponent<T>();
	return false;
}

template<>
bool CreateComponent<Renderer>(GameObject* obj, SERIALIZE_INPUT& ar);

template<>
bool CreateComponent<Box2DBody>(GameObject* obj, SERIALIZE_INPUT& ar);

template<class T>
class ReflectionComponent final
{
	ReflectionComponent()
	{
#ifdef DEBUG_TRUE
		AssemblyComponent::IReflection ref(CreateComponent<T>, ReflectionComponent::AddComponent);
		AssemblyComponent::assemblies.emplace(typeid(T).name(),std::move(ref));
#else
		AssemblyComponent::assemblies.emplace(typeid(T).name(), CreateComponent<T>);
#endif
	}

	static void AddComponent(GameObject* obj)
	{
		obj->AddComponent<T>();
	}
public:
	static ReflectionComponent& Instance()
	{
		static ReflectionComponent instance;
		return instance;
	}
};

#define SetReflectionComponent(className) namespace className##Reflection \
	{ \
		 inline ReflectionComponent< className >& reflection = ReflectionComponent< className >::Instance(); \
	}

#define REGISTER_COMPONENT(derived)                     \
    SetReflectionComponent(derived);                      \
    CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, derived)

SetReflectionComponent(Renderer)
SetReflectionComponent(Box2DBody)
SetReflectionComponent(SubWindow)
SetReflectionComponent(Animator)
SetReflectionComponent(Button)
SetReflectionComponent(SFText)

