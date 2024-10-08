#pragma once

//�f�o�b�O�t���O
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
//�Z�[�t�|�C���^�̍쐬
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
    //�����֎~
    PointerRegistryManager() {}

    //SafePointer���쐬(�g���ĂȂ�)
    template <typename T>
    SafePointer<T> createLoggingPointer(T* rawPtr, const std::string& name) {
        return SafePointer<T>(rawPtr, name, getRegistry<T>());
    }

    //�����̃|�C���^���Q�Ƃ��Ă���|�C���^�[�̍폜
    template <typename T>
    static void deletePointer(T* ptr) {
        getRegistry<T>().notifyDeletion(ptr);
    }
private:
    // �قȂ����^�̃��X�g��ۑ����Ď擾����
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

    //�|�C���^��o�^
    void registerPointer(SafePointer<T>* p) {
        pointers.push_back(p);
    }

    //�|�C���^�̓o�^����
    void unregisterPointer(SafePointer<T>* p) {
        pointers.remove(p);
    }

    //�Ή������|�C���^�̎Q�Ƃ��Ȃ���
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

    //����SafePointer���m���
    SafePointer<T>& operator=(SafePointer<T> _p)
    {
        ptr = _p.get();
        return *this;
    }
    //�|�C���^�̑��
    SafePointer<T>& operator=(T* _p)
    {
        ptr = _p;
        return *this;
    }
    //T�^�ւ̕ϊ�
    operator T* ()const
    {
        return ptr;
    }
    //�|�C���^������
    T& operator*() {
        if (ptr == nullptr) {
            std::string log = "Dereferencing a deleted pointer : " + ptrName;
            throw std::runtime_error(log.c_str());
        }
        return *ptr;
    }
    //�|�C���^�Q��
    T* operator->() {
        if (ptr == nullptr) {
            std::string log = "Dereferencing a deleted pointer : " + ptrName;
            throw std::runtime_error(log.c_str());
        }
        return ptr;
    }

    //�ێ����Ă���A�h���X�̎擾
    T* get() const { return ptr; }

    //���L���Ă���A�h���X���ׂĖ����ɂ���
    void reset() { registry.notifyDeletion(ptr); }

    //�ێ����Ă���A�h���X�̖�����
    void invalidate() { ptr = nullptr; }

    //�ێ����Ă���|�C���^���g���Ă�����
    bool isValid() const { return ptr != nullptr; }

    //�|�C���^�̍폜
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

