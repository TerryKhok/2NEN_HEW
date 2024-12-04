#pragma once


const char* relativePath(const char* _fullPath);
void setConsoleTextColor(unsigned int _color);

#ifdef DEBUG_TRUE

#define LOG(format, ...) printf_s("\n<file:%s,line:%d>",relativePath(__FILE__),__LINE__);printf_s(format,__VA_ARGS__)
#define LOG_ERROR(format, ...) setConsoleTextColor(4);printf_s("\n-ERROR!!-\n<file:%s,line:%d>",relativePath(__FILE__),__LINE__);printf_s(format,__VA_ARGS__);setConsoleTextColor(7)
#define LOG_WARNING(format, ...) setConsoleTextColor(6);printf_s("\n-WARNING!-\n<file:%s,line:%d>",relativePath(__FILE__),__LINE__);printf_s(format,__VA_ARGS__);setConsoleTextColor(7)

#define LOG_NL printf_s("\n")

#else
#define LOG(format, ...)  
#define LOG_ERROR(format, ...)  
#define LOG_WARNING(format, ...)  

#define LOG_NL
#endif

// Forward declarations
class PointerRegistry;
template <typename T> class SafePointer;


#ifdef DEBUG_TRUE

//セーフポインタの作成
#define SAFE_POINTER(type,var) SafePointer<type> var = SafePointer<type>(nullptr, #var);

#define SAFE_TYPE(type) SafePointer<type>

#else
#define SAFE_POINTER(type,var) type* var;

#define SAFE_TYPE(type) type*
#endif


#ifdef DEBUG_TRUE

#define TRY_CATCH_LOG(process) \
    try { \
        process; \
    } catch (const std::exception& e) { \
        LOG_ERROR(e.what()); \
    } catch (...) { \
    }\

#else

#define TRY_CATCH_LOG(process) \
    try { \
        process; \
    }catch (...) { \
    }\

#endif

class SafePointerBase {
public:
    virtual ~SafePointerBase() = default;

    //ポインタの無効化（SafePointer<T>で実装予定）
    virtual void invalidate() = 0;

    //レジスタで比較するためにVoid*の生ポインタを取得
    virtual void* getRawPointer() const = 0;
};

class PointerRegistry {

    friend class PointerRegistryManager;
    template <typename T>
    friend class SafePointer;

private:
    PointerRegistry() {}

private:
    // ポインターを登録する
    void registerPointer(SafePointerBase* p) {
        pointers.push_back(p);
    }

    // ポインタの登録を解除する
    void unregisterPointer(SafePointerBase* p) {
        pointers.remove(p);
    }

    void notifyDeletion(void* ptr) {
        for (auto& p : pointers) {
            if (p->getRawPointer() == ptr) {
                p->invalidate();
            }
        }
    }

private:
    std::list<SafePointerBase*> pointers;
};

class PointerRegistryManager {
    template <typename T>
    friend class SafePointer;

    friend class Window;
    friend class Scene;
    friend class GameObject;
    friend class ObjectManager;
    friend class Component;
    friend class ImGuiApp;
private:
    //生成禁止
    PointerRegistryManager() {}

    //レジスタ取得
    static PointerRegistry& getRegistry() {
        static PointerRegistry registry;  // Single registry for all types
        return registry;
    }

    //レジスタに受け取ったポインタをVoid*に変換して通知を送る
    template <typename T>
    static void deletePointer(T* _ptr) {
        getRegistry().notifyDeletion(static_cast<void*>(_ptr));
    }
private:
    static PointerRegistryManager manager;
};

template <typename T>
class SafePointer : public SafePointerBase{
 
public:
    void* operator new(size_t) = delete;

    SafePointer(T* _p = nullptr, const std::string& _name = typeid(T).name(), PointerRegistry& _reg = PointerRegistryManager::getRegistry())
        : ptr(_p), ptrName(_name), registry(_reg) {
        registry.registerPointer(this);
    }

    ~SafePointer() {
        registry.unregisterPointer(this);
    }

    //同じSafePointer同士代入
    SafePointer<T>& operator=(SafePointer<T> _p)
    {
        ptr = _p.get();
        return *this;
    }
    //ポインタの代入
    SafePointer<T>& operator=(T* _p)
    {
        ptr = _p;
        return *this;
    }
    //T*型への変換
    operator const T* () const
    {
        if (ptr == nullptr) {
            std::string log = "Dereferencing a deleted pointer : " + ptrName;
            throw std::runtime_error(log.c_str());
        }

        return ptr;
    }
  
    T** operator&() {
        return &ptr;
    }

    //ポインタ剥がし
    T& operator*() {
        if (ptr == nullptr) {
            std::string log = "Dereferencing a deleted pointer : " + ptrName;
            throw std::runtime_error(log.c_str());
        }
        return *ptr;
    }
    //ポインタ参照
    T* operator->() {
        if (ptr == nullptr) {
            std::string log = "Dereferencing a deleted pointer : " + ptrName;
            throw std::runtime_error(log.c_str());
        }
        return ptr;
    }

    bool operator==(const T* _value) {
        return ptr == _value;
    }

    bool operator!=(const T* _value) {
        return ptr != _value;
    }

private:
    //保持しているアドレスの無効化
    void invalidate() override {
        ptr = nullptr;
    }
    //比較用のVoid*の生ポインタ取得
    void* getRawPointer() const override {
        return static_cast<void*>(ptr);
    }

    //保持しているアドレスの取得
    T* get() const { return ptr; }

    //共有しているアドレスすべて無効にする
    void reset() { registry.notifyDeletion(ptr); }

    //保持しているポインタを使っていいか
    bool isValid() const { return ptr != nullptr; }

    //ポインタの削除
    void Delete() {
        auto p = ptr;
        registry.notifyDeletion(ptr);
        delete p;
    }
private:
    T* ptr;
    std::string ptrName;
    PointerRegistry& registry;
};
