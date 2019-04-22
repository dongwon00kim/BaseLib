#ifndef SINGLETON_H_
#define SINGLETON_H_

#include <memory>
#define USE_MEYERS_SINGLETON
#ifndef USE_MEYERS_SINGLETON
#include <mutex>
#endif

namespace baseutils {

template<typename T>
class Singleton
{
public:
    virtual ~Singleton() = default;

    static T& GetInstance() {
#ifdef USE_MEYERS_SINGLETON
        static T sInstance;   
        return sInstance;
#else   
        std::call_once(mOnceFlag, [] {
            mInstance.reset(new T);
        });
        return *mInstance.get();
#endif
    }

private:
#ifndef USE_MEYERS_SINGLETON
    static std::unique_ptr<T> mInstance;

    static std::once_flag mOnceFlag;
#endif
    Singleton() = default;

    Singleton(const Singleton&) = delete;

    Singleton& operator=(const Singleton&) = delete;
};

} // namespace baseutils

#endif /* SINGLETON_H_ */
