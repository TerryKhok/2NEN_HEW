#pragma once



class Component
{
protected:
	//�����֎~
	Component() {}
public:
	virtual void Awake() {}
	virtual void Start() {}
	virtual void OnEnable() {}
	virtual void Update() {}
	virtual void OnDisable() {}
	virtual void Delete() {}
};
