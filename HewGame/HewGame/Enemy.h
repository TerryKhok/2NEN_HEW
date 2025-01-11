#pragma once

class Enemy :public Component
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
		if constexpr (3 <= 7) {
			::cereal::make_optional_nvp(ar, "searchAngle", searchAngle) ; ::cereal::make_optional_nvp(ar, "searchDis", searchDis); ::cereal::make_optional_nvp(ar, "searchRenge", searchRenge);
			::cereal::make_optional_nvp(ar, "speed", speed);
		}
		else {
			ar(searchAngle, searchDis, searchRenge);
		}
	} void Deserialize(cereal::JSONInputArchive& ar) override {
		if constexpr (3 <= 7) {
			::cereal::make_optional_nvp(ar, "searchAngle", searchAngle) ; ::cereal::make_optional_nvp(ar, "searchDis", searchDis); ::cereal::make_optional_nvp(ar, "searchRenge", searchRenge);
			::cereal::make_optional_nvp(ar, "speed", speed);
		}
		else {
			ar(searchAngle, searchDis, searchRenge);
		}
	};
};

SetReflectionComponent(Enemy);