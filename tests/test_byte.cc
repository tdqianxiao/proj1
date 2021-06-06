#include <iostream>
#include <assert.h>
#include "src/byteArray.h"
#include <string.h>
#include "src/utils.h"

int main (int argv,char ** argc){
	tadpole::ByteArray::ptr ba(new tadpole::ByteArray(1));
    auto printer = tadpole::GetConsumeTimePrinter();
    for(int j = 0 ; j < 1000; ++j){
        for(int i = 0 ; i < 1000;++i){
            ba->writeFInt8(1);
            ba->writeFInt16(2);
            ba->writeFInt32(3);
            ba->writeFInt64(4);
            ba->writeFUint8(5);
            ba->writeFUint16(6);
            ba->writeFUint32(7);
            ba->writeFUint64(8);
            
            std::string name("111");
            ba->writeString(name);

            ba->writeFloat(1.1f);

            ba->writeDouble(1.1);

            char buf[32] = {};
            memset(buf,30,1);
            ba->write(buf,30);
        }

        for(int i = 0 ; i < 1000;++i){
            int8_t a1 = 0;
            int16_t a2 = 0;
            int32_t a3 = 0;
            int64_t a4 = 0;

            uint8_t b1 = 0;
            uint16_t b2 = 0;
            uint32_t b3 = 0;
            uint64_t b4 = 0;

            ba->readFInt8(a1);assert(a1 == 1);
            ba->readFInt16(a2);assert(a2 == 2);
            ba->readFInt32(a3);assert(a3 == 3);
            ba->readFInt64(a4);assert(a4 == 4);
            ba->readFUint8(b1);assert(b1 == 5);
            ba->readFUint16(b2);assert(b2 == 6);
            ba->readFUint32(b3);assert(b3 == 7);
            ba->readFUint64(b4);assert(b4 == 8);
            std::string name;
            ba->readString(name);
            assert(name == "111");

            float c = 0;
            ba->readFloat(c);assert(c == 1.1f);

            double c2 = 0;
            ba->readDouble(c2);assert(c2 == 1.1);

            char buf[32] = {};
            ba->read(buf,30);
        }
    }
	return 0;	
}


