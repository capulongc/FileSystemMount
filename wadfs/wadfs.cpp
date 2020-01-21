#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <errno.h>
#include <stdlib.h>
#include "/home/reptilian/libWad/Wad.h"

Wad *myWad;
static int wadGetAttr(const char *path, struct stat *stbuff){
	if(myWad->isDirectory(path)){
		stbuff->st_mode = S_IFDIR | 0755;
		stbuff->st_nlink = 2;
		return 0;
	}

	if(myWad->isContent(path)){
		stbuff->st_mode = S_IFREG | 0777;
		stbuff->st_nlink = 1;
		stbuff->st_size = myWad->getSize(path);
		return 0;
	}

	return -ENOENT;
}

static int wadOpenFile(const char *path, struct fuse_file_info *fi){ return 0; }

static int wadReadFile(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi){
	int dataSize = myWad->getSize(path);
	char *myData = new char[dataSize + 1];
	myData[dataSize] = 0;
	return myWad->getContents(path, buffer, dataSize, 0);
}

static int wadRelFile(const char *path, struct fuse_file_info *fi){ return 0; }

static int wadOpenDir(const char *path, struct fuse_file_info *fi){ return 0; }

static int wadReadDir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
	if(offset == 0) filler(buffer, ".", NULL, 0);
	if(offset < 2) filler(buffer, "..", NULL, 0);

	std::vector<std::string> entries;
	int numEntries = myWad->getDirectory(path, &entries);
	for(int i = offset; i < entries.size(); i ++){
		filler(buffer, entries[i].c_str(), NULL, 0);
	}
	return 0;
}

static int wadRelDir(const char *path, struct fuse_file_info *fi){
	return 0;
}


static struct fuse_operations op = {
};

int main(int argc, char *argv[]){
	myWad = Wad::loadWad(argv[1]);

	op.getattr = wadGetAttr;
	op.open = wadOpenFile;
        op.read = wadReadFile;
	op.release = wadRelFile;
	op.opendir = wadOpenDir;
	op.readdir = wadReadDir;
	op.releasedir = wadRelDir;

	int fuseArgc = argc - 1;
	if(argc == 3){
		char *fuseArgv[fuseArgc] = {argv[0], argv[2]};
		int result = fuse_main(fuseArgc, fuseArgv, &op, NULL);
	}
	if(argc == 4){
		char *fuseArgv[fuseArgc] = {argv[0], argv[2], argv[3]};
		int result = fuse_main(fuseArgc, fuseArgv, &op, NULL);
	}

	delete myWad;
	return 0;
}
