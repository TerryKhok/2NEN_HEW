#pragma once


class FunctionRegistry {

	friend class Button;

public:
	using FunctionType = std::function<void()>;
private:
	static inline std::unordered_map<std::string, FunctionType> functions;

	// Private constructor to prevent multiple instances
	FunctionRegistry() = default;
	FunctionRegistry(const FunctionRegistry&) = delete;
	FunctionRegistry& operator=(const FunctionRegistry&) = delete;
public:
	static FunctionRegistry& Get() {
		static FunctionRegistry instance;
		return instance;
	}

	void registerFunction(const std::string& name, FunctionType func) {
		if (functions.find(name) != functions.end()) {
			LOG_WARNING("Function with name '%s' already exists.", name.c_str());
			return;
		}
		functions[name] = func;
	}

	void callFunction(const std::string& name) const {
		auto it = functions.find(name);
		if (it == functions.end()) {
			LOG_WARNING("Function with name '%s' not found.", name.c_str());
			return;
		}
		it->second();
	}

	FunctionType GetRegisterFunction(std::string& name) const{
		auto it = functions.find(name);
		if (it == functions.end()) {
			LOG_WARNING("Function with name '%s' not found.", name.c_str());
			return nullptr;
		}
		it->second;
	}

	//登録された関数を選ぶアイコンを表示する
	static void DrawPickFunction(const char* _label, std::string& _funcName);
};

class Button : public Component
{
	friend class GameObject;

public:
	enum BUTTON_ACTION
	{
		MOUSE_TRIGGER,
		MOUSE_PRESS,
		MOUSE_RELEASE,
		ACTION_MAX
	};

	void SetEvent(std::function<void()>&& _func) {
		m_event = _func;
	}

	void SetEvent(std::string _funcName);

	void SetAction(BUTTON_ACTION _action);

	void Serialize(SERIALIZE_OUTPUT& ar) override;
	void Deserialize(SERIALIZE_INPUT& ar) override;
	void DrawImGui(ImGuiApp::HandleUI& _handle) override;
private:
	Button() 
	{
		SetEvent("ButtonLog");
	}

	void Update() {
		(this->*pUpdate)();
	}
	void PauseUpdate() override{
		(this->*pUpdate)();
	}

	void MouseTrigger();
	void MousePress();
	void MouseRelease();
private:
	void(Button::* pUpdate)() = &Button::MouseTrigger;
private:
	FunctionRegistry::FunctionType m_event = {};
	std::string_view m_funcName = "";
	BUTTON_ACTION m_action = MOUSE_TRIGGER;
};

// Macro to automatically register a function
#define REGISTER_FUNCTION(func)                                      \
    namespace {                                                           \
        struct AutoRegister_##func {                                      \
            AutoRegister_##func() {                                       \
                FunctionRegistry::Get().registerFunction(#func, func); \
            }                                                             \
        } autoRegister_##func;                                            \
    }

void ButtonLog();

REGISTER_FUNCTION(ButtonLog)