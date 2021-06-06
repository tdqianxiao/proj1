#ifndef __TADPOLE_BYTEARRAY_H__
#define __TADPOLE_BYTEARRAY_H__

#include <memory>
#include <vector>
#include <sys/types.h>

#include "src/bswap.hpp"

/**
 * @ 如果有大小端差异，按小端序列化，如果在大端机器解会自动变为小端
 */

namespace tadpole{

	/**
	* @brief 字节数组类
	*/
	class ByteArray{
	public:
		/**
		* @brief 智能指针定义
		*/
		typedef std::shared_ptr<ByteArray> ptr;
	public:
		/**
		*@brief 节点类
		*/
		struct Node{
			/**
			* @brief 构造一个节点
			* @param[in] size 节点大小
			*/
			Node(size_t size)
				:next(nullptr){
				buf = new char[size];
			}
			
			/**
			* @brief 析构函数，释放节点
			*/
			~Node(){
				delete[] buf ; 
			}
			//buff指针
			char * buf;
			//指向下一个节点
			Node * next ; 
		};

		/**
		* @brief 构造函数
		* @param[in] size 该bytearray创建节点的大小
		*/
		ByteArray(size_t size = 4096);
		
		/**
		* @brief 析构函数
		*/
		~ByteArray();
		
		/**
		* @brief 判断是否为小端
		* @return true 小端
		*/
		bool isLittleEndian(){
			return TADPOLE_BYTE_ORDER == TADPOLE_LITTLE_ENDIAN;
		}
		
		/**
		* @brief 获得该字节数组的容量
		*/
		size_t getCapcity()const {return m_capcity;}

		/**
		* @brief 获得大小，为实际存储数据的大小
		*/
		size_t getSize()const {return m_size;}

		/**
		* @brief 获得可读指针当前位置
		*/
		const char * getPosition()const {return m_pos;}

		/**
		* @brief 获得node节点的大小
		*/
		size_t getBasesize()const {return m_basesize;}
		
		/**
		* @brief 获得当前块还有多少内存,写buf用
		*/
		size_t useCapcity(){return m_basesize - (m_pos - m_curWriteNode->buf);}
		
		/**
		* @brief 读缓冲区时获得一个buf还有多少可读字节读buf用
		*/
		size_t readLastSize(){return m_basesize - (m_cur - m_root->buf);}

		/**
		* @brief 往字节数组里面写元素
		* @param[in] buf 所需写元素的地址
		* @param[in] size 所需写元素的大小
		*/
		void write(const char * buf , size_t size);

		/**
		* @brief 在字节数组里面读元素
		* @param[out] buf 需要赋值的buf
		* @param[in] size 需要赋值元素的大小
		*/
		bool read(char * buf , size_t size);

		/**
		* @brief 写固定长度的int
		*/
		void writeFInt8(int8_t val);
		void writeFInt16(int16_t val);
		void writeFInt32(int32_t val);
		void writeFInt64(int64_t val);
		void writeFUint8(uint8_t val);
		void writeFUint16(uint16_t val);
		void writeFUint32(uint32_t val);
		void writeFUint64(uint64_t val);

		/**
		* @brief 写压缩长度的int
		*/
		void writeInt32(int32_t val);
		void writeInt64(int64_t val);
		void writeUint32(uint32_t val);
		void writeUint64(uint64_t val);

		/**
		* @brief 读固定长度的int
		*/
		void readFInt8(int8_t &val);
		void readFInt16(int16_t &val);
		void readFInt32(int32_t &val);
		void readFInt64(int64_t &val);
		void readFUint8(uint8_t &val);
		void readFUint16(uint16_t &val);
		void readFUint32(uint32_t &val);
		void readFUint64(uint64_t &val);

		/**
		* @brief 读压缩长度的int
		*/
		void readInt32(int32_t &val);
		void readInt64(int64_t &val);
		void readUint32(uint32_t &val);
		void readUint64(uint64_t &val);

		/**
		* @brief 写字符串，最大长度为4字节，也就是字符串最大长度为4g
		* @param[in] str 字符串
		*/
		void writeString(const std::string & str);

		/**
		* @brief 读字符串，最大读4g
		* @param[out] str 需要写的字符串
		*/
		void readString(std::string & str);

		/** 
		* @brief 写固定长度的浮点数
		*/
		void writeFloat(const float& val);
		void writeDouble(const double& val);

		/**
		* @brief 读固定长度的浮点数
		*/
		void readFloat(float &val);
		void readDouble(double &val);
	private:
		//当前写位置
		char * m_pos ;
		//当前读位置
		char * m_cur;
		//bytearray里元素大小
		size_t m_size;
		//node 大小
		size_t m_basesize;
		//容量
		size_t m_capcity;
		//第一个节点
		Node * m_root;
		//当前读节点
		Node * m_curWriteNode; 
	};

}

#endif 
