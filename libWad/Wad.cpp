#include <stdio.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <sstream>

struct WadNode {
	int offset;
	int length;
	std::string name;
	std::vector<struct WadNode> children;
};

class Wad{
	public:
		static char buffer[10000000];
		static std::string filePath;
		static int fileSize;
		static int numDesc;
		static int offDesc;
		static std::vector<struct WadNode> elements;

		static Wad* loadWad(const std::string &path){
			filePath = path;
			std::ifstream input(path, std::ios::binary | std::ios::ate);
			fileSize = input.tellg();
			input.seekg(0);
			input.read(buffer, fileSize);

			char a[4] = {buffer[4],buffer[5],buffer[6],buffer[7]};
			char b[4] = {buffer[8],buffer[9],buffer[10],buffer[11]};
			memcpy(&numDesc, a, sizeof(int));
			memcpy(&offDesc, b, sizeof(int));

			int buffLoc = offDesc;
			for(int i = 0; i < numDesc; i ++){
				char offArr[4] = {buffer[buffLoc++], buffer[buffLoc++], buffer[buffLoc++], buffer[buffLoc++]};
				char lenArr[4] = {buffer[buffLoc++], buffer[buffLoc++], buffer[buffLoc++], buffer[buffLoc++]};
				char nodeName[8] = {buffer[buffLoc++], buffer[buffLoc++], buffer[buffLoc++], buffer[buffLoc++],
						    buffer[buffLoc++], buffer[buffLoc++], buffer[buffLoc++], buffer[buffLoc++]};
				std::vector<struct WadNode> nodeChildren;
				int os, len;
				memcpy(&os, offArr, sizeof(int));
				memcpy(&len, lenArr, sizeof(int));
				std::string name(nodeName);
				struct WadNode node = {os, len, name, nodeChildren};
				elements.push_back(node);
			}

			struct WadNode root = {0,0,"/",{}};
			elements.emplace(elements.begin(), root);
			makeTree(&elements[0], 1, "");
		}

		static int makeTree(struct WadNode *currNode, int i, std::string exit){
			int iter = i;
			while(iter < elements.size()){
				struct WadNode wn = elements[iter];
				int nameLen = wn.name.length();

				if(wn.name == exit){
					elements.erase(elements.begin() + iter);
					return iter;
				}

				//HANDLE MAP MARKERS
				if(nameLen == 4 && wn.name.at(0) == 'E' && wn.name.at(2) == 'M'){
					int tempIter = iter + 1;
					for(int i = 0; i < 10; i ++){
						elements[iter].children.push_back(elements[tempIter + i]);
					}
					currNode->children.push_back(elements[iter]);
					iter += 10;
				}

				//HANDLE NAMESPACE MARKERS
				else if(nameLen > 6 && wn.name.substr(nameLen - 6, nameLen) == "_START"){
					std::string dirName = wn.name.substr(0, nameLen - 6);
					int dirOS = wn.offset;
					int dirLN = wn.length;
					std::vector<struct WadNode> dirChildren;
					std::string dirEnd = dirName + "_END";

					elements.erase(elements.begin() + iter);
                        	       	struct WadNode dirNode = {dirOS, dirLN, dirName, dirChildren};
					elements.emplace(elements.begin() + iter, dirNode);
					currNode->children.push_back(dirNode);
					iter = makeTree(&currNode->children.back(), iter + 1, dirEnd) - 1;
				}
				else
					currNode->children.push_back(elements[iter]);
				iter++;
			}
		}

		std::string getMagic(){
			std::string b(buffer);
			std::string mag = b.substr(0,4);
			return mag;
		}

		struct WadNode readPath(const std::string &path){
			if(path == "/")
				return elements[0];

			std::string nPath = path.substr(1, path.length());
			std::stringstream pathStream;
			pathStream << nPath;
			std::vector<std::string> nodeNames;
			std::string newNode;

			while(std::getline(pathStream, newNode, '/'))
				nodeNames.push_back(newNode);

			struct WadNode selNode = elements[0];
			for(std::string n : nodeNames){
				for(struct WadNode wn : selNode.children){
					if(wn.name == n){
						selNode = wn;
						break;
					}
				}
			}
			return selNode;
		}

		bool isContent(const std::string &path){
			return (getSize(path) != 0);
		}

		bool isDirectory(const std::string &path){
			return (getSize(path) == 0);
		}

		int getSize(const std::string &path){
			struct WadNode n = readPath(path);
			return n.length;
		}

		int getContents(const std::string &path, char *cbuffer, int length, int offset = 0){
			if(!isContent(path)) return -1;
			struct WadNode file = readPath(path);

			memcpy(cbuffer, buffer+file.offset+offset, length);
			cbuffer[length] = '\0';
			/*
			FILE* fh = fopen(filePath.c_str(), "r");
			fseek(fh, file.offset + offset, SEEK_SET);
			fread(cbuffer, 1, length, fh);
			fclose(fh);*/
			return (length - offset);
		}

		int getDirectory(const std::string &path, std::vector<std::string> *directory){
			if(this->isDirectory(path) == false) return -1;

			int count = 0;
			for(struct WadNode wn : readPath(path).children){
				count++;
				directory->push_back(wn.name);
			}

			return count;
		}

		int testPath(const std::string &path){
			return 0;
		}
};
