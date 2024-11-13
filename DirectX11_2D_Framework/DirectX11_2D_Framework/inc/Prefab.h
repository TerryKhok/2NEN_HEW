#pragma once

#include<nlohmann/json.hpp>
#include<boost/pfr.hpp>

// Factory for dynamic component creation, serialization, and deserialization
class ComponentFactory {
public:
    using CreateFunction = std::function<Component*()>;
    using SerializeFunction = std::function<void(nlohmann::json&, const Component&)>;
    using DeserializeFunction = std::function<void(const nlohmann::json&, Component&)>;

    static ComponentFactory& getInstance() {
        static ComponentFactory instance;
        return instance;
    }

    void registerType(const std::string& type, CreateFunction createFunc,
        SerializeFunction serializeFunc, DeserializeFunction deserializeFunc) {
        creators[type] = std::move(createFunc);
        serializers[type] = std::move(serializeFunc);
        deserializers[type] = std::move(deserializeFunc);
    }

    std::unique_ptr<Component, void(*)(Component*)> create(const std::string& type) const {
        auto it = creators.find(type);
        if (it != creators.end()) {
            return std::unique_ptr<Component, void(*)(Component*)>((it->second)(), [](Component* p) {delete p; });
        }
        return std::unique_ptr<Component, void(*)(Component*)>(nullptr, [](Component* p) {delete p; });
    }

    void serialize(const std::unique_ptr<Component, void(*)(Component*)>& component, nlohmann::json& j) const {
        auto it = serializers.find(component->getType());
        if (it != serializers.end()) {
            it->second(j, *component);
        }
    }

    void deserialize(const std::string& type, const nlohmann::json& j, std::unique_ptr<Component, void(*)(Component*)>& component) const {
        component = create(type);
        if (!component) throw std::runtime_error("Unknown component type: " + type);

        auto it = deserializers.find(type);
        if (it != deserializers.end()) {
            it->second(j, *component);
        }
    }

private:
    std::unordered_map<std::string, CreateFunction> creators;
    std::unordered_map<std::string, SerializeFunction> serializers;
    std::unordered_map<std::string, DeserializeFunction> deserializers;
};

// Register macro for components to avoid manual registration
#define REGISTER_COMPONENT_TYPE(TYPE) \
    static bool reg_##TYPE = []() { \
        ComponentFactory::getInstance().registerType(#TYPE, \
            []() { return new TYPE(); }, \
            [](nlohmann::json& j, const Component& comp) { \
                const auto& derived = static_cast<const TYPE&>(comp); \
                j["type"] = derived.getType(); \
                boost::pfr::for_each_field(derived, [&](const auto& field, auto index) { \
                    j["field" + std::to_string(index)] = field; \
                }); \
            }, \
            [](const nlohmann::json& j, Component& comp) { \
                auto& derived = static_cast<TYPE&>(comp); \
                boost::pfr::for_each_field(derived, [&](auto& field, auto index) { \
                    field = j.at("field" + std::to_string(index)).get<decltype(field)>(); \
                }); \
            } \
        ); \
        return true; \
    }();