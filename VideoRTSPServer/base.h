#pragma once
#include<string>
#ifndef BYTE
typedef unsigned char BYTE;
#endif // !BYTE

class MyBuffer :public std::string {
public:
	MyBuffer(size_t size = 0) :std::string() {//调用string的构造函数
		if (size > 0) {
			resize(size);
			memset((void*)this->c_str(), 0, size);
		}
	}
	MyBuffer(void* buffer, size_t size) :std::string() {
		memcpy((void*)c_str(), buffer, size);
	}
	/*MyBuffer(const MyBuffer& buffer) {
		resize(buffer.size());
		memcpy((void*)c_str(),buffer.c_str(),buffer.size());
	}
	MyBuffer& operator=(const MyBuffer& buffer) {
		if (&buffer != this) {
			resize(buffer.size());
			memcpy((void*)c_str(), (void*)buffer.c_str(), buffer.size());
		}
		return *this;
	}*/
	MyBuffer(const char* str) {
		resize(strlen(str));
		memcpy((void*)c_str(), str, strlen(str));
	}
	~MyBuffer() {
		std::string::~basic_string();
	}
	operator char* () const {
		return (char*)c_str();
	}
	operator const char* () const {
		return c_str();
	}
	operator BYTE* () const {
		return (BYTE*)c_str();
	}
	operator void* () const {
		return (void*)c_str();
	}
	void Update(void* buffer, size_t size) {
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}
	void Zero() {
		if (size() > 0) {
			memset((char*)c_str(), 0, size());
		}
	}
	MyBuffer& operator<<(const MyBuffer& buffer) {
		if (&buffer != this) {
			*this += buffer;
		}
		else {
			MyBuffer tmp = buffer;
			*this += tmp;
		}
		return *this;
	}
	MyBuffer& operator<<(int data) {
		char s[16] = "";
		snprintf(s, sizeof(s), "%d", data);
		*this += s;
		return *this;
	}
	const MyBuffer& operator>>(int& data) const {
		data = atoi(*this);
		return *this;
	}
	const MyBuffer& operator>>(short& data) const {
		data = (short)atoi(*this);
		return *this;
	}
	void ResetSize() {
		size_t size = strlen(c_str());
		resize(size);
	}
};
