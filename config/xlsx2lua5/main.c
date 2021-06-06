#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdio.h>

int luaopen_zip(lua_State *L);

int build_file_md5 (char *file_path,char*dest);
int build_buffer_md5 (unsigned char *buffer,size_t n,char*dest);

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

struct fbuf_t {
	char * buf;
	size_t cap;
	size_t cur;	
};

int fbuf_create(lua_State * vm) {
	struct fbuf_t *c = lua_newuserdata(vm, sizeof(*c));
	memset(c,0,sizeof(*c));
	return 1;
}

#if defined(_WIN32)


#else
void mkdirs(const char *pathname, mode_t mode) {
	int i,len;
    char str[1024];    
    strncpy(str,pathname, 1024);
    len=strlen(str);
    for( i=0; i <= len; i++ ) {
    	if ((str[i] == '/' && i != 0)  || i == len) {
    		str[i] = '\0';
    		if (access(str, F_OK) != 0) {
				mkdir(str,mode);
    		} 
    		str[i] = '/';
    	}
    }
    return;
}
#endif

int fbuf_write(lua_State * vm) {
	struct fbuf_t *c = lua_touserdata(vm, 1);
	size_t len = 0;
	const char * str = luaL_checklstring(vm,2,&len);
	if (len + c->cur > c->cap) {
		size_t total = len+(c->cur+10240)/10240*10240;
		assert(total > c->cap);
		char * buf = realloc(c->buf,total);
		assert(buf);
		c->buf = buf;
		c->cap = total;
	}
	memcpy(c->buf+c->cur,str,len);
	c->cur += len;
	return 0;
}

int fbuf_flush(lua_State * vm) {
	struct fbuf_t *c = lua_touserdata(vm, 1);
	if (c->cur) {
		const char * fpath = luaL_checkstring(vm,2);
		assert(fpath);
		const char * fpdir = luaL_checkstring(vm,3);
		assert(fpdir);
		if (access(fpdir, F_OK) != 0) {
			#if defined(_WIN32)
			mkdir(fpdir);
			#else
			mkdirs(fpdir, 0777);
			#endif
		}
		FILE * fd = fopen(fpath, "wb");
		assert(fd);
		size_t wn = fwrite(c->buf, 1, c->cur, fd);
		assert(wn == c->cur);
		fclose(fd);
		c->cur = 0;//reset
	}
	return 0;
}

int fbuf_reset(lua_State * vm) {
	struct fbuf_t *c = lua_touserdata(vm, 1);
	c->cur = 0;
	return 0;
}

int fbuf_md5(lua_State * vm) {
	struct fbuf_t *c = lua_touserdata(vm, 1);
	if (c->cur > 0) {
		char md5[33];
		memset(md5,'\0',sizeof(md5));
		build_buffer_md5(c->buf,c->cur,md5);
		assert(md5[sizeof(md5)-1]=='\0');
		lua_pushstring(vm,md5);
		return 1;
	}
	return 0;
}

int file_md5(lua_State * vm) {
	const char * full = luaL_checkstring(vm,1);
	char md5[33];
	memset(md5,'\0',sizeof(md5));
	if (build_file_md5(full,md5))
		lua_pushstring(vm,md5);
	else
		lua_pushnil(vm);
	return 1;
}

#define job_limit 1024
int job_amt = 0;
int job_cur = 0;
char ** jobs[job_limit] = {NULL};
char ** md5s[job_limit] = {NULL};

int jobs_set(lua_State*vm) {
	size_t len = 0;
	const char * buf = luaL_checklstring(vm,-1,&len);
	char * job = malloc(len + 1);
	strcpy(job,buf);
	job[len] = 0;
	md5s[job_amt] = NULL;
	jobs[job_amt++] = job;
	assert(job_amt < job_limit);
	lua_pushinteger(vm,job_amt-1);
	return 1;
}

int jobs_get(lua_State*vm) {
	int idx = luaL_checkinteger(vm,1);
	assert(idx<job_limit);
	const char * job = jobs[idx];
	const char * md5 = md5s[idx];
	lua_pushstring(vm,job);
	if (md5)
		lua_pushstring(vm,md5);
	else
		lua_pushnil(vm);
	return 2;
}

uint64_t time_real_ms() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int ltime_real_ms(lua_State * vm) {
	double tm = (double)time_real_ms();
	lua_pushnumber(vm,tm);
	return 1;
}

/*int _ecb(lua_State * L) {
    const char *msg = lua_tostring(L, 1);
	if (msg)
		luaL_traceback(L, L, msg, 1);
	else {
		lua_pushliteral(L, "(no error message)");
	}
	return 1;
}*/
int _ecb(lua_State * lvm) {
    lua_getglobal(lvm,"debug");
    lua_getfield(lvm, -1, "traceback");
    lua_pushvalue(lvm, 1);
    lua_pushinteger(lvm, 2);
    lua_call(lvm, 2, 1);
    const char * errmsg = lua_tostring(lvm, -1);
    if (errmsg) {
		printf("%s\n",errmsg);
	}
	return 1;
}

static void * _worker(void * p) {
	int id = *((int*)p);
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	luaL_requiref(L, "zip", luaopen_zip, 0);
	lua_pushcfunction(L,fbuf_create);
	lua_setglobal(L,"fbuf_create");
	lua_pushcfunction(L,fbuf_write);
	lua_setglobal(L,"fbuf_write");
	lua_pushcfunction(L,fbuf_flush);
	lua_setglobal(L,"fbuf_flush");
	lua_pushcfunction(L,fbuf_reset);
	lua_setglobal(L,"fbuf_reset");
	lua_pushcfunction(L,fbuf_md5);
	lua_setglobal(L,"fbuf_md5");
	if (luaL_dofile(L, "c.lua") != 0) {
		const char * errmsg = lua_tostring(L, -1);
		if (errmsg) {
			printf("%s\n", errmsg);
		}
		assert(0);
	}
	
	for(;;) {
		int job_idx = __sync_fetch_and_add(&job_cur,1);
		if (job_idx >= job_amt)
			break;
		
		char * job = (char*)jobs[job_idx];
		//printf("aaa %d\n",job_idx);
		int t = lua_gettop(L);
		lua_pushcfunction(L,_ecb);
		#if defined(_WIN32)
		lua_getglobal(L,"_workWin");
		#else
		lua_getglobal(L,"_workUnix");
		#endif
		lua_pushstring(L,job);
		assert(job);
		lua_pushinteger(L,id);
		int err = lua_pcall(L, 2, 1, -4);
		if (err) {
			printf(lua_tostring(L,-1));
		} else {
			size_t len = 0;
			const char * md5 = luaL_checklstring(L,-1,&len);
			if (!md5) {
				md5s[job_idx]=NULL;
			} else {
				assert(len > 0 && len <= 32);
				char * buf = malloc(len+1);
				memcpy(buf,md5,len);
				buf[len]='\0';
				md5s[job_idx] = buf;
			}
		}
		assert(err == 0);
		//free(job);
		lua_settop(L,t);
	}
	lua_close(L);
	return NULL;
}

int
main(int argc, const char *argv[]) {
	//__sync_fetch_and_add(&id,1);
	assert(argc >= 3);
	int worker_amt = atoi(argv[1]);
	if (worker_amt < 0)
		worker_amt = 1;
	else if(worker_amt > 64)
		worker_amt = 64;
	/*
	char cmd[1024];
	FILE * fd = NULL;
	#if defined(_WIN32)
	sprintf(cmd,"c.bat %s\\xlsx",argv[2]);
	fd = popen(cmd, "rb");
	#else
	sprintf(cmd,"./c.sh %s",argv[2]);
	fd = popen(cmd, "r");
	#endif

	int blen = 1024*100;
	char * ftxt = malloc(blen);
	int rlen = fread(ftxt,1,blen,fd);
	assert(blen >= rlen);
	pclose(fd);
	
	int fpos = 0,tpos=0;
	for (;tpos < rlen; tpos++) {
		if (ftxt[tpos] == '\n') {
			if (tpos > fpos) {
				int len = tpos - fpos;
				char * job = (char*)malloc(len+1);
				memcpy(job,ftxt+fpos,len);
				job[len] = 0;
				md5s[job_amt] = NULL;
				jobs[job_amt++] = job;
				assert(job_amt < job_limit);
			}
			fpos = tpos + 1;
		}
	}
	*/
	uint64_t stm = time_real_ms();
	//准备工作开始
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	
	#if defined(_WIN32)
	lua_pushboolean(L,1);
	lua_setglobal(L,"_WIN32");
	#endif

	if (luaL_dofile(L, "controller.lua") != 0) {
		const char * errmsg = lua_tostring(L, -1);
		if (errmsg) {
			printf("%s\n", errmsg);
		}
		assert(0);
	}

	lua_pushcfunction(L,jobs_set);
	lua_setglobal(L,"jobs_set");
	lua_pushcfunction(L,jobs_get);
	lua_setglobal(L,"jobs_get");
	lua_pushcfunction(L,file_md5);
	lua_setglobal(L,"file_md5");
	lua_pushcfunction(L,ltime_real_ms);
	lua_setglobal(L,"ltime_real_ms");

	lua_pushcfunction(L,_ecb);
	lua_getglobal(L,"jobs_begin");
	lua_pushstring(L,argv[2]);
	int err = lua_pcall(L, 1, 0, -3);
	if (err) {
		printf(lua_tostring(L,-1));
		assert(0);
	}
	//准备工作结束
	//开始转换
	printf("\ninit thread number:%d\n\n",worker_amt);
	pthread_t worker_handles[worker_amt];
	int worker_ids[worker_amt];
	int i = 0;
	for (;i<worker_amt;i++) {
		worker_ids[i] = i + 1;
		pthread_create(&worker_handles[i], NULL, _worker, &worker_ids[i]);
	}
	
	for (i = 0;i<worker_amt;i++) {
		pthread_join(worker_handles[i], NULL);
	}
	//转换结束
	//开始收尾
	lua_pushcfunction(L,_ecb);
	lua_getglobal(L,"jobs_end");
	lua_pushstring(L,argv[2]);
	err = lua_pcall(L, 1, 0, -3);
	if (err) {
		printf(lua_tostring(L,-1));
		assert(0);
	}
	//收尾结束
	uint64_t etm = time_real_ms();
	printf("\ndone! cost %f second; worker thread number:%d\n",(etm-stm)/1000.0,worker_amt);
	return 1;
}

