
#include <baseutils/Handler.h>
#include "LooperRoster.h"

namespace baseutils {

std::shared_ptr<Looper>
Handler::looper() {
    return LooperRoster::getInstance()->findLooper(id());
}

} // namespace baseutils
