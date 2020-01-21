#pragma once
#include "Wad.cpp"
#include <stdlib.h>

char Wad::buffer[10000000];
std::string Wad::filePath = "";
int Wad::fileSize = 0;
int Wad::numDesc = 0;
int Wad::offDesc = 0;
std::vector<struct WadNode> Wad::elements = {};

int testPath(char* filePath);
static Wad* loadWad(const std::string &path);
std::string getMagic();
bool isContent(const std::string &path);
bool isDirectory(const std::string &path);
int getContents(const std::string &path, char *buffer, int length, int offset = 0);
int getDirectory(const std::string &path, std::vector<std::string> *directory);
