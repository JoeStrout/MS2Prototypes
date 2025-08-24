/*
 *  SimpleString.cpp
 *
 */
#include "SimpleString.h"
#include "UnicodeUtil.h"
#include "UnitTest.h"
#include <stdio.h>
#include <cmath>

namespace MiniScript {

#if(DEBUG)
	long RefCountedStorage::instanceCount = 0;
	long StringStorage::instanceCount = 0;
	StringStorage* StringStorage::head = nullptr;

	long _stringInstanceCount() { return StringStorage::instanceCount; }

	void StringStorage::DumpStrings() {
		StringStorage *ss = head;
		while (ss) {
			printf("%s\n", ss->data);
			ss = ss->_next;
		}
	}
#endif

	
	using std::fabs;
	
	// at
	//
	//	Get the UTF-8 character at the given character index.
	int String::at(size_t pos) const {
		if (!ss) return 0;
		if (ss->charCount < 0) ss->analyzeChars();
		if (ss->isASCII) {
			// Easy case: this String is all ASCII, so we can grab the requested character directly.
			return (pos < ss->dataSize ? ss->data[pos] : 0);
		}
		// Harder case: we have some multi-byte characters, so we have to iterate.
		unsigned char *c = (unsigned char*)(ss->data);
		unsigned char *maxc = c + ss->dataSize;
		AdvanceUTF8(&c, maxc, (int)pos);
		return (int)UTF8Decode(c);
	}

	size_t String::bytePosOfCharPos(size_t pos) const {
		if (pos <= 0 || !ss) return 0;
		if (ss->charCount < 0) ss->analyzeChars();
		if (ss->isASCII) return pos;
		unsigned char *c = (unsigned char*)ss->data;
		unsigned char *maxc = c + ss->dataSize;
		AdvanceUTF8(&c, maxc, (int)pos);
		return (size_t)(c - (unsigned char*)ss->data);
	}
	
	size_t String::charPosOfBytePos(size_t posB) const {
		if (posB <= 0 || !ss) return 0;
		if (ss->charCount < 0) ss->analyzeChars();
		if (posB > ss->dataSize) return ss->charCount;
		if (ss->isASCII) return posB;
		int count = 0;
		for (size_t i=0; i<posB; i++) {
			if (!IsUTF8IntraChar(ss->data[i])) count++;
		}
		return count;
	}

	String String::Substring(long pos, long numChars) const {
		if (!ss) return *this;
		long posB = bytePosOfCharPos(pos);	// (also ensures ss->isASCII is known)
		if (ss->isASCII) return SubstringB(pos, numChars);
		unsigned char *startPtr = (unsigned char*)ss->data + posB;
		unsigned char *endPtr = startPtr;
		unsigned char *max = (unsigned char*)ss->data + ss->dataSize;
		if (numChars < 0) endPtr = max;
		else AdvanceUTF8(&endPtr, max, (int)numChars);
		return SubstringB(posB, endPtr - startPtr);
	}
	
	bool String::StartsWith(const String& s) const {
		if (s.empty()) return true;
		long byteCount = s.ss->dataSize - 1;  // (ignoring terminating null)
		if (!ss or ss->dataSize < byteCount) return false;
		char *p1 = ss->data;
		char *p2 = s.ss->data;
		for (int i=0; i<byteCount; i++) if (*p1++ != *p2++) return false;
		return true;
	}
	
	bool String::EndsWith(const String& s) const {
		if (s.empty()) return true;
		long byteCount = s.ss->dataSize - 1;  // (ignoring terminating null)
		if (!ss or ss->dataSize < byteCount) return false;
		char *p1 = ss->data + ss->dataSize - byteCount - 1;
		char *p2 = s.ss->data;
		for (int i=0; i<byteCount; i++) if (*p1++ != *p2++) return false;
		return true;
	}

	String String::Format(int num, const char* formatSpec) {
		char buf[32];
		snprintf(buf, 32, formatSpec, num);
		return buf;
	}

	String String::Format(long num, const char* formatSpec) {
		char buf[32];
		snprintf(buf, 32, formatSpec, num);
		return buf;
	}

	String String::Format(float num, const char* formatSpec) {
		char buf[32];
		snprintf(buf, 32, formatSpec, num);
		return buf;
	}

	String String::Format(double num, const char* formatSpec) {
		char buf[32];
		snprintf(buf, 32, formatSpec, num);
		return buf;
	}

	String String::Format(bool value, const char* trueString, const char* falseString) {
		return value ? trueString : falseString;
	}

	int String::IntValue(const char* formatSpec) const {
		int retval = 0;
		sscanf(c_str(), formatSpec, &retval);
		return retval;
	}

	long String::LongValue(const char* formatSpec) const {
		long retval = 0;
		sscanf(c_str(), formatSpec, &retval);
		return retval;
	}

	float String::FloatValue(const char* formatSpec) const {
		float retval = 0;
		sscanf(c_str(), formatSpec, &retval);
		return retval;
	}

	double String::DoubleValue(const char* formatSpec) const {
		double retval = 0;
		sscanf(c_str(), formatSpec, &retval);
		return retval;
	}

	bool String::BooleanValue() const {
		String lower = this->ToLower();
		if (0 == lower.Compare("true") or 0 == lower.Compare("yes")
			or 0 == lower.Compare("t") or 0 == lower.Compare("y")) {
			return true;
		}
		
		// Check if the String has a float value but don't call FloatValue() since it calls this method
		float floatVal = 0;
		sscanf(c_str(), "%f", &floatVal);
		return (fabs(floatVal) > 0.0001);
	}

	// Comparision between cstrings to strings
	bool operator==(const char *cstring, const String &str) {
		return str == cstring;
	}
	bool operator!=(const char *cstring, const String &str) {
		return str != cstring;
	}
	bool operator<(const char *cstring, const String &str) {
		return str > cstring;
	}
	bool operator<=(const char *cstring, const String &str) {
		return str >= cstring;
	}
	bool operator>(const char *cstring, const String &str) {
		return str < cstring;
	}
	bool operator>=(const char *cstring, const String &str) {
		return str <= cstring;
	}

	// UTF-8-savvy character-oriented code
	void StringStorage::analyzeChars() {
		charCount = 0;
		isASCII = true;
		for (const unsigned char *c = (const unsigned char*)data; *c; c++) {
			if (IsUTF8IntraChar(*c)) isASCII = false;
			else charCount++;
		}
	}


	//--------------------------------------------------------------------------------
	// Unit Tests (commented out)

	/*
	class TestString : public UnitTest
	{
	public:
		TestString() : UnitTest("String") {}
		virtual void Run();
	};
	
	void TestString::Run()
	{
		// ... test code commented out ...
	}

	RegisterUnitTest(TestString);
	*/
}

