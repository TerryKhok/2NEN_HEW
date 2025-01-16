
template<>
bool CreateComponentFunc<Renderer>(GameObject* obj, SERIALIZE_INPUT& ar)
{
	obj->LoadComponent<Renderer>(ar);
	return true;
}

template<>
bool CreateComponentFunc<Box2DBody>(GameObject* obj, SERIALIZE_INPUT& ar)
{
	obj->LoadComponent<Box2DBody>(ar);
	return true;
}

template<>
bool CreateComponentFunc<Animator>(GameObject* obj, SERIALIZE_INPUT& ar)
{
	obj->LoadComponent<Animator>(ar);
	return true;
}

template<>
bool CreateComponentFunc<TileMap>(GameObject* obj, SERIALIZE_INPUT& ar)
{
	obj->LoadComponent<TileMap>(ar);
	return true;
}

template<>
bool CreateComponentFunc<SubWindow>(GameObject* obj, SERIALIZE_INPUT& ar)
{
	obj->AddComponent<SubWindow>();
	return true;
}

template<>
bool CreateComponentFunc<SFText>(GameObject* obj, SERIALIZE_INPUT& ar)
{
	obj->AddComponent<SFText>();
	return true;
}