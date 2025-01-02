
template<>
bool CreateComponent<Renderer>(GameObject* obj, SERIALIZE_INPUT& ar)
{
	obj->LoadComponent<Renderer>(ar);
	return true;
}

template<>
bool CreateComponent<Box2DBody>(GameObject* obj, SERIALIZE_INPUT& ar)
{
	obj->LoadComponent<Box2DBody>(ar);
	return true;
}


template<>
bool CreateComponent<Animator>(GameObject* obj, SERIALIZE_INPUT& ar)
{
	obj->LoadComponent<Animator>(ar);
	return true;
}

template<>
bool CreateComponent<TileMap>(GameObject* obj, SERIALIZE_INPUT& ar)
{
	obj->LoadComponent<TileMap>(ar);
	return true;
}