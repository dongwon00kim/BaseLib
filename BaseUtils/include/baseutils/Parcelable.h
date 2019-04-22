#ifndef PARCELABLE_H_
#define PARCELABLE_H_

#include <memory>

namespace baseutils {

/**
 *  @class Parcelable
 *  @brief Parcelable object for Message
  */
class Parcelable {
public:
	Parcelable() = default;

    virtual ~Parcelable() = default;
};

} // namespace baseutils

#endif  // PARCELABLE_H_
