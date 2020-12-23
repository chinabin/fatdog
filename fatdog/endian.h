#ifndef __FATDOG_ENDIAN_H__
#define __FATDOG_ENDIAN_H__

#define FATDOG_LITTLE_ENDIAN 1
#define FATDOG_BIG_ENDIAN 2

#include <type_traits>
#include <byteswap.h>
#include <stdint.h>

namespace fatdog
{
    /*
     * check https://man7.org/linux/man-pages/man3/bswap.3.html and https://ftp.samba.org/pub/unpacked/ntdb/lib/ccan/endian/endian.h
     * for bswap_XX explanation, bswap_XX 以 8 字节为单位，进行首尾调换。
    */
    template <class T>
    typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type byteswap(T value)
    {
        return (T)bswap_64((uint64_t)value);
    }

    template <class T>
    typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type byteswap(T value)
    {
        return (T)bswap_32((uint32_t)value);
    }

    template <class T>
    typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type byteswap(T value)
    {
        return (T)bswap_16((uint16_t)value);
    }

#if BYTE_ORDER == BIG_ENDIAN
#define FATDOG_BYTE_ORDER FATDOG_BIG_ENDIAN
#else
#define FATDOG_BYTE_ORDER FATDOG_LITTLE_ENDIAN
#endif

#if FATDOG_BYTE_ORDER == FATDOG_BIG_ENDIAN
    template <class T>
    T byteswapOnLittleEndian(T t)
    {
        return t;
    }

    template <class T>
    T byteswapOnBigEndian(T t)
    {
        return byteswap(t);
    }

#else

    template <class T>
    T byteswapOnLittleEndian(T t)
    {
        return byteswap(t);
    }

    template <class T>
    T byteswapOnBigEndian(T t)
    {
        return t;
    }
#endif
} // namespace fatdog

#endif