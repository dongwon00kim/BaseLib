/** @file SendableObject.h
 *
 *  Brief description.
 *
 *  @author            Dongwon, Kim (dongwon00.kim@gmail.com)
 *  @version           1.0
 *  @date              2016.06.17
 *  @note
 *  @see
 */

#ifndef _SENDABLE_OBJECT_H_
#define _SENDABLE_OBJECT_H_

#include <memory>

#include <Base.h>

using namespace std;

namespace utils {
namespace baseutils {

/**
 *  @class Sendable
 *  @brief Sendable object for Message
  */
class SendableObject {
public:
	SendableObject() = default;

    virtual ~SendableObject() = default;
};

}; // namespace baseutils
}; // namespace utils

#endif  // _SENDABLE_OBJECT_H_
