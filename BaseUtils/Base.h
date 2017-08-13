/** @file Base.h
 *
 *  Brief description.
 *
 *  @author            Dongwon, Kim (dongwon00.kim@gmail.com)
 *  @version           1.0
 *  @date              2016.05.11
 *  @note
 *  @see
 */

#ifndef _BASE_H_
#define _BASE_H_

#define DISALLOW_EVIL_CONSTRUCTORS(name) \
    name(const name &) = delete; \
    name &operator=(const name &) = delete;

#endif

