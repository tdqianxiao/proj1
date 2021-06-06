#ifndef __TADPOLE_BSWAP_HPP__
#define __TADPOLE_BSWAP_HPP__

#define TADPOLE_LITTLE_ENDIAN  1
#define TADPOLE_BIG_ENDIAN 2

#include <byteswap.h>
#include <stdint.h>

namespace tadpole{
/**
 * @brief 8字节类型的字节序转化
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
byteswap(T value) {
    return (T)bswap_64((uint64_t)value);
}

/**
 * @brief 4字节类型的字节序转化
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
byteswap(T value) {
    return (T)bswap_32((uint32_t)value);
}

/**
 * @brief 2字节类型的字节序转化
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
byteswap(T value) {
    return (T)bswap_16((uint16_t)value);
}

/**
 * @brief 让代码在不同大小端机器上表现一样
 */
#if BYTE_ORDER == BIG_ENDIAN
#define TADPOLE_BYTE_ORDER TADPOLE_BIG_ENDIAN
#else 
#define TADPOLE_BYTE_ORDER TADPOLE_LITTLE_ENDIAN
#endif 

#if TADPOLE_BYTE_ORDER == TADPOLE_LITTLE_ENDIAN
template <class T>
T byteswapOnBigEndian(T val){
	return val ;
}

template <class T>
T byteswapOnLittleEndian(T val){
	return byteswap(val);
}

#else 
template <class T>
T byteswapOnBigEndian(T val){
	return byteswap(val);
}

template <class T>
T byteswapOnLittleEndian(T val){
	return val ; 
}
#endif 

}

#endif 
