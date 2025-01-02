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

template<>
bool CreateComponent<Animator>(GameObject* obj, SERIALIZE_INPUT& ar);

template<>
bool CreateComponent<TileMap>(GameObject* obj, SERIALIZE_INPUT& ar);

template<class T>
class ReflectionComponent final
{
	ReflectionComponent()
	{
#ifdef DEBUG_TRUE
		AssemblyComponent::IReflection ref(CreateComponent<T>, AddComponentFunc);
		AssemblyComponent::assemblies.emplace(typeid(T).name(),std::move(ref));
#else
		AssemblyComponent::assemblies.emplace(typeid(T).name(), CreateComponent<T>);
#endif
	}

	static void AddComponentFunc(GameObject* obj)
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
SetReflectionComponent(TileMap)

template<class T>
class ReflectionScene final
{
	ReflectionScene()
	{
		AssemblyScene::assemblies.emplace(typeid(T).name(), RegisterSceneFunc);
	}

	static void RegisterSceneFunc()
	{
		SceneManager::RegisterScene<T>();
	}
public:
	static ReflectionScene& Instance()
	{
		static ReflectionScene instance;
		return instance;
	}
};


#define SetReflectionScene(className) namespace className##Reflection \
	{ \
		 inline ReflectionScene< className >& reflection = ReflectionScene< className >::Instance(); \
	}

SetReflectionScene(SampleScene)
//
//template<class T>
//class ReflectionFunction final
//{
//	ReflectionFunction()
//	{
//		ReflectionFunction::assemblies.emplace(typeid(T).name(), RegisterSceneFunc);
//	}
//
//	static void RegisterFunctionFunc()
//	{
//		
//	}
//public:
//	static ReflectionFunction& Instance()
//	{
//		static ReflectionFunction instance;
//		return instance;
//	}
//};

