#pragma once

class Enemy_type1 :public Component
{
private:

	float speed = 1.0f;
	Vector3 goalPos;
	std::vector<Vector2> checkPos;
	float searchAngle = 45.0f;
	float searchDis = 20.0f;
	float searchRenge = 100.0f;
	int searchCount = 0;

	SAFE_POINTER(Box2DBody, rb);

	void Start();
	void Update();
	void Search(Vector3 targetPos);
	void OnColliderEnter(GameObject* _other) override;
	void DrawImGui(ImGuiApp::HandleUI& _handle);
	
	void Serialize(cereal::JSONOutputArchive& ar) override {
		ar(CEREAL_NVP(speed), CEREAL_NVP(searchAngle), CEREAL_NVP(searchDis), CEREAL_NVP(searchRenge));
	} 
	void Deserialize(cereal::JSONInputArchive& ar) override {
		ar(CEREAL_NVP(speed), CEREAL_NVP(searchAngle), CEREAL_NVP(searchDis), CEREAL_NVP(searchRenge));
	}
};

SetReflectionComponent(Enemy_type1);