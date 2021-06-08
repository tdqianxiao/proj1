#include "src/byteArray.h"
#include <string.h>
#include "src/log.h"

using namespace tadpole;
int loadFile(const std::string & path){
	ByteArray::ptr ba(new ByteArray(4096));
	FILE* file = fopen(path.c_str(),"r+b"); 
	fseek(file,0,SEEK_END);
	int size = ftell(file);
	std::cout<<"size:"<<size<<std::endl;
	fseek(file,0,SEEK_SET);

	char buf[4096] = {};
	while(1){
		memset(buf,0,4096);
		int ret = fread(buf,1,4096,file);
		if(ret == 0){
			//TADPOLE_LOG_DEBUG(TADPOLE_FIND_LOGGER("root")) << "ret : "<<ret;	
			break; 
		}	
		ba->write(buf,4096);
		if(ftell(file) >= size){
			break;
		}
	}
	fclose(file);	
	return size;
}

int main(){
    loadFile("./test/xxx.jpeg");
    return 0;
}