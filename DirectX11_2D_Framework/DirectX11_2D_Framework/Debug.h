#pragma once

//デバッグフラグ
//================================================================
#define DEBUG_TRUE
//================================================================


const char* relativePath(const char* fullPath);
void setConsoleTextColor(unsigned int color);

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
template <typename T> class SafePointer;
template <typename T> class PointerRegistry;

#ifdef DEBUG_TRUE
//セーフポインタの作成
#define SAFE_POINTER(type,var) SafePointer<type> var = SafePointer<type>(nullptr, #var)
#else
#define SAFE_POINTER(type,var) type* var;
#endif

// The PointerRegistryManager can manage multiple types of registries
class PointerRegistryManager {
    template <typename T>
    friend class SafePointer;

    friend class Scene;
    friend class Component;
private:
    //生成禁止
    PointerRegistryManager() {}

    //SafePointerを作成(使ってない)
    template <typename T>
    SafePointer<T> createLoggingPointer(T* rawPtr, const std::string& name) {
        return SafePointer<T>(rawPtr, name, getRegistry<T>());
    }

    //引数のポインタを参照しているポインターの削除
    template <typename T>
    static void deletePointer(T* ptr) {
        getRegistry<T>().notifyDeletion(ptr);
    }
private:
    // 異なった型のリストを保存して取得する
    template <typename T>
    static PointerRegistry<T>& getRegistry() {
        static PointerRegistry<T> registry;
        return registry;
    }
private:
    static PointerRegistryManager manager;
};


template <typename T>
class PointerRegistry {
    
    friend class PointerRegistryManager;
    template <typename T>
    friend class SafePointer;

private:
    PointerRegistry() {}

    //ポインタを登録
    void registerPointer(SafePointer<T>* p) {
        pointers.push_back(p);
    }

    //ポインタの登録解除
    void unregisterPointer(SafePointer<T>* p) {
        pointers.remove(p);
    }

    //対応したポインタの参照をなくす
    void notifyDeletion(T* ptr) {
        for (auto& p : pointers) {
            if (p->get() == ptr) {
                p->invalidate();
            }
        }
    }
private:
    std::list<SafePointer<T>*> pointers;
};

template <typename T>
class SafePointer {
 
public:   
    SafePointer(T* p = nullptr, const std::string& name = typeid(T).name(), PointerRegistry<T>& reg = PointerRegistryManager::manager.getRegistry<T>())
        : ptr(p), ptrName(name), registry(reg) {
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
    //T型への変換
    operator T* ()const
    {
        return ptr;
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

    //保持しているアドレスの取得
    T* get() const { return ptr; }

    //共有しているアドレスすべて無効にする
    void reset() { registry.notifyDeletion(ptr); }

    //保持しているアドレスの無効化
    void invalidate() { ptr = nullptr; }

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
    PointerRegistry<T>& registry;
};

