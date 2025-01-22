// JSON parser.
// This code is designed to parse a Json object tree, and parse it into a tree of C++ JsonValue
// objects.  It's not designed to be fast, and it's not strict - it will allow some things through
// that aren't genuine JSON (like floating point numbers starting with '.').  It also has
// a distressing habit of ignoring lines starting with '#' as a comment.
//
// A JSON document is a single JSON object, and hence a single JsonObject, but within that object
// there may be a tree of sub-objects.
//
// (c) Mike Moreton 2024.  This code probably has bugs. If that matters to you, fix them before use.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <climits>
#include <math.h>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "musicbrainz5/jsonparser.h"

static uint16_t getUtf16(const char **current)
{
	const char *ptr = *current;
	if ((*ptr++ != '\\') || (*ptr++ != 'u')) {
		throw JsonValue::JsonParseException("Missing \\u", "");
	}

	uint16_t code = 0;
	for (int i = 0; i < 4; i++) {
		char inch = *ptr++; 
		uint8_t digit;
		if ((inch >= '0') && (inch <= '9')) {
			digit = inch - '0';
		} else if ((inch >= 'A') && (inch <= 'F')) {
			digit = inch - 'A' + 10;
		} else if ((inch >= 'a') && (inch <= 'f')) {
			digit = inch - 'a' + 10;
		} else {
			throw JsonValue::JsonParseException("\\u not followed by four hex digits", "");
		}

		code = (code << 4) + digit;
	}
	*current = ptr;
	return(code);
}
			
void JsonValue::read(const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		throw JsonParseException("Couldn't open file for reading", filename);
	}

	//
	// Work out how long the file is.
	if (0 != fseek(fp, 0, SEEK_END)) {
		throw JsonParseException("Couldn't seek to end of file", filename);
	}
	long fileLength = ftell(fp);
	if (0 != fseek(fp, 0, SEEK_SET)) {
		throw JsonParseException("Couldn't seek to beginning of file", filename);
	}

	//
	// Read the entire file into a buffer 
	char *buffer = (char *)malloc(fileLength + 1);
	if (!buffer) {
		throw JsonParseException("Couldn't malloc file buffer", filename);
	}

	if (fread(buffer, fileLength, 1, fp) != 1) {
		throw JsonParseException("Couldn't read file into buffer", filename);
	}
	fclose(fp);
	buffer[fileLength] = '\0';

	parse(buffer);
}

void JsonValue::parse(const std::string &jsonText)
{
	//
	// The file must contain a single JSON object, which means it must start with '{'.
	const char *current = jsonText.c_str();
	skipWhitespace(&current);

	//
	// An object must start with an opening curly bracket.
	if (*current != '{') {
		throw JsonParseException("Top level object does not start with curly brace", "");
	}
	current++;

	current = parseObject(current);
	const char *objectEnd = current;

	//
	// And there must be nothing after the closing bracket.
	skipWhitespace(&current);
	if (*current != '\0') {
		throw JsonParseException("Unexpected text after end of top level object", objectEnd);
	}
}

//
// Skip over whitespace in the input.
void JsonValue::skipWhitespace(const char **buffer)
{
	const char *ptr = *buffer;
	while(true) {
		while (isspace(*ptr) && (*ptr != '\0')) {
			ptr++;
		}


		//
		// We also skip comments - not legal JSON, but very useful.
		if (*ptr == '#') {
			while (*ptr != '\n') {
				ptr++;
			}
		} else {
			break;
		}
	}
	*buffer = ptr;
}

//
// A JSON object is a list of string:value pairs.
const char *JsonValue::parseObject(const char *current)
{
	const char *objStart = current;
	type_f = JSON_OBJECT;

	while (true) {
		skipWhitespace(&current);

		if (*current == '}') {
			//
			// This is the end of the object.
			current++;
			return(current);
		} else if (*current == '\0') {
			throw JsonParseException("Unexpected end of file while reading object", objStart);
		}

		//
		// The next thing must be the name string.
		objStart = current;
		std::string nameString = parseString(&current);

		//
		// Then must be a colon
		skipWhitespace(&current);
		if (*current != ':') {
				throw JsonParseException("Missing : between name and value while reading object",
						objStart);
		}
		current++;

		//
		// Then comes a value.  
		JsonValue value;
		current = value.parseValue(current);
		nameValPairs_f[nameString] = value;
		nameValsInOrder_f.emplace_back(std::pair<std::string, JsonValue>(nameString, value));
		
		//
		// Must be followed by a } or ,
		skipWhitespace(&current);
		if (*current == ',') {
			current++;
			continue;	// On to next name value pair.
		} else if (*current == '}') {
			continue; // Handled in top of loop.
		} else {
			throw JsonParseException("Unexpected character after parsing name/value while reading",
					objStart);
		}
	}
}

//
// Parse a JSON String into a JsonValue.
std::string JsonValue::parseString(const char **buffer)
{
	const char *current = *buffer;
	std::string theString;

	skipWhitespace(&current);
	if (*current != '"') {
			throw JsonParseException("No opening quote when reading string", *buffer);
	}
	current++;

	while(*current != '"') {
		switch(*current) {
			case '\0':
				throw JsonParseException("Unexpected EOF during string read.", *buffer);

			case '\\':
				current++;
				switch(*current) {
				case '\0':
					throw JsonParseException("Unexpected EOF during string escape read.", *buffer);

				case '"':
				case '\\':
				case '/':
					theString += *current;
					break;

				case 'b':
					theString += '\b';
					break;

				case 'f':
					theString += '\f';
					break;

				case 'n':
					theString += '\n';
					break;

				case 'r':
					theString += '\r';
					break;

				case 't':
					theString += '\t';
					break;

				case 'u':
					//
					// Nowadays JSON is almost always UTF-8 encoded, which makes the ability
					// to specify UTF-16 encodings a little strange.  But, whatever...
					// We change the UTF-16 encoding into a UTF-8 encoding.
					current -= 1; 	// Step back to the beginning of the \u
					uint16_t utf16;
					uint32_t unicode;
					utf16 = getUtf16(&current);
					unicode	= utf16;	// default and error condition is to use it unchanged

					//
					// 32 bit unicodes are encoded by a UTF16 high surrogate containing the high
					// order 10 bits followed by a UTF16 low surrogate containing the low order 10
					// bits.  So if we have a high surrogate, we try and read a following low
					// surrogate and create a 20 bit character to which we add 0x10000.
					// However, unpaired surrogates are not unknown.  In this case we step back
					// to the point where we tried to read the low surrogate, and just encode the
					// high surrogate as if it were a valid UTF16 character.
					if ((utf16 >= 0xd800) && (utf16 <= 0xdbff)) {
						//
						// This is a high surrogate.
						uint32_t highBits = (utf16 - 0xd800) << 10;
						const char *startLow = current;
						try {
							uint16_t lowUtf16 = getUtf16(&current);
							if ((lowUtf16 >= 0xdc00) && (lowUtf16 <= 0xdfff)) {
								//
								// Found a low surrogate
								unicode =  highBits + (lowUtf16 - 0xdc00) + 0x10000;
							} else {
								current = startLow;
							}
						} catch (JsonParseException &e) {
							//
							// Failed to find the low surrogate.  Reset the current pointer.
							current = startLow;
						}
					}

					//
					// So we now have a unicode code point, which we need to encode in UTF-8.
					if (unicode >= 0x10000) {
						theString += char(0xf0 + ((unicode & 0x1c0000) >> 18));
						theString += char(0x80 + ((unicode & 0x3f000) >> 12));
						theString += char(0x80 + ((unicode & 0xfc0) >> 6));
						theString += char(0x80 + (unicode & 0x3f));
					} else if (unicode >= 0x800) {
						theString += char(0xe0 + ((unicode & 0xf000) >> 12));
						theString += char(0x80 + ((unicode & 0xfc0) >> 6));
						theString += char(0x80 + (unicode & 0x3f));
					} else if (unicode >= 0x80) {
						theString += char(0xc0 + ((unicode & 0x3c0) >> 6));
						theString += char(0x80 + (unicode & 0x3f));
					} else {
						theString += char(unicode & 0x3f);
					}

					//
					// We'll have stepped on one character too far.
					current--;

					break;

				default:
					throw JsonParseException("Unsupported escape character", *buffer);
				}
				break;

			default:
				theString += *current;
				break;
		}

		current++;
	}

	//
	// Must have reached the end of the string.
	*buffer = ++current;
	return(theString);
}

//
// This parses a JSON value into an existing JSON object.
const char *JsonValue::parseValue(const char *current)
{
	skipWhitespace(&current);

	switch(*current) {
		case '\0':
			throw JsonParseException("Unexpected EOF while looking for value", current);

		case '"':
			type_f = JSON_STRING;
			theString_f = parseString(&current);
			break;

		case '{':
			current = parseObject(++current);
			break;

		case '[':
			current = parseArray(++current);
			break;

		default:
			if (0 == strncmp(current, "true", strlen("true"))) {
				type_f = JSON_BOOL;
				theBool_f = true;
				current += strlen("true");
			} else if (0 == strncmp(current, "false", strlen("false"))) {
				type_f = JSON_BOOL;
				theBool_f = false;
				current += strlen("false");
			} else if (0 == strncmp(current, "null", strlen("null"))) {
				type_f = JSON_NULL;
				current += strlen("null");
			} else {
				//
				// The only possibility is a number. We're more permissive, allowing
				// hex numbers "0x".
				type_f = JSON_NUMBER;
				const char *endPtr;
				theInteger_f = strtol(current, (char **)&endPtr, 0);
				if ((theInteger_f == LONG_MIN) || (theInteger_f == LONG_MAX)) {
					throw JsonParseException("Integer out of range", current);
				}

				if ((endPtr == current) && (*current != '.')) {
					throw JsonParseException("Unexpected character in integer value", current);
				}

				if ((*endPtr == 'E') || (*endPtr == 'e') || (*endPtr == '.')) {
					//
					// Oops - it was actually a floating point number.  Go back and reparse
					// it on this basis.
					type_f = JSON_FLOAT;
					theFloat_f = strtod(current, (char **)&endPtr);
					if ((theFloat_f == HUGE_VAL) || (theFloat_f == -HUGE_VAL)) {
						throw JsonParseException("Float out of range", current);
					}

					if (endPtr == current) {
						throw JsonParseException("Unexpected character in float value", current);
					}
				}
				current = endPtr;
			}
			break;
	}
	return(current);
}

//
// Parse a JSON Value into a JsonValue object.
const char *JsonValue::parseArray(const char *current)
{
	const char *objStart = current;

	//
	// A JSON array consists of a comma separated list of JSON values, terminated by a ']'.
	type_f = JSON_ARRAY;
	while (true) {
		skipWhitespace(&current);
		switch(*current) {
			case '\0':
				throw JsonParseException("Unexpected EOF while looking for array", objStart);
	
			case ']':
				// End of array
				return(++current);
	
			case ',':
				//
				// We don't check the commas, so you could have one, zero, or many between objects
				// and we wouldn't care.
				current++;
				break;

			default:
				//
				// Start of a value.
				JsonValue newVal;
				current = newVal.parseValue(current);
				theArray_f.push_back(newVal);
				break;
		}
	}
}

//
// Check whether the current object is a null object, and if so throw an exception.
void JsonValue::checkForNull() const
{
	if (type_f == JSON_NULL) {
		throw NullTypeException("Element is null");
	}
}

//
// Find the JsonValue keyed by the given name in the current JSON object.
const JsonValue &JsonValue::operator[](const char *name) const
{
	std::string nameString(name);
	return ((*this)[nameString]);
}

//
// Find the JsonValue keyed by the given name in the current JSON object.
const JsonValue &JsonValue::operator[](std::string &name) const
{
	checkForNull();
	if (type_f != JSON_OBJECT) {
		throw AccessTypeException("Can't index non-object with string");
	}

	std::map<std::string, JsonValue>::const_iterator theEntry = nameValPairs_f.find(name);
	if (theEntry == nameValPairs_f.end()) {
		throw NotFoundException(name.c_str());
	} else {
		return(theEntry->second);
	}
}

//
// If the JsonValue is a JSON object, return true if there is a JsonValue mapped to the
// supplied name.
bool JsonValue::has(const char *name) const
{
	std::string nameString(name);
	return(has(nameString));
}

bool JsonValue::has(std::string &name) const
{
	checkForNull();
	if (type_f != JSON_OBJECT) {
		throw AccessTypeException("Can't index non-object with string");
	}

	std::map<std::string, JsonValue>::const_iterator theEntry = nameValPairs_f.find(name);
	return(theEntry != nameValPairs_f.end());
}

//
// Find the JsonValue at the given index in the current JSON array.
const JsonValue &JsonValue::operator[](int idx) const
{
	if (idx < 0) {
		throw NotFoundException(std::to_string(idx).c_str());
	} else {
		return((*this)[(unsigned)idx]);
	}
}

//
// Find the JsonValue at the given index in the current JSON array.
const JsonValue &JsonValue::operator[](unsigned idx) const
{
	checkForNull();
	if (type_f != JSON_ARRAY) {
		throw AccessTypeException("Can't index non-array with integer");
	}

	if (idx >= theArray_f.size()) {
		throw NotFoundException(std::to_string(idx).c_str());
	} else {
		return(theArray_f[idx]);
	}
}

//
// Return the number of elements.
unsigned JsonValue::length() const
{
	checkForNull();
	if (type_f == JSON_ARRAY) {
		return(theArray_f.size());
	} else {
		throw AccessTypeException("Can't get length of non-array");
	}
}

//
// For a JSON object, return the map of name value pairs.
const std::map<std::string, JsonValue> &JsonValue::getMap() const
{
	checkForNull();
	if (type_f == JSON_OBJECT) {
		return(nameValPairs_f);
	} else {
		throw AccessTypeException("Can't get object map for non-object");
	}
}

//
// For a JSON array, return the array of objects
const std::vector<JsonValue> &JsonValue::getArray() const
{
	checkForNull();
	if (type_f == JSON_ARRAY) {
		return(theArray_f);
	} else {
		throw AccessTypeException("Can't get array for non-array");
	}
}


//
// Return the string from a value.
const std::string &JsonValue::getString() const
{
	checkForNull();
	if (type_f == JSON_STRING) {
		return(theString_f);
	}
		throw AccessTypeException("Value is not a string");
}

//
// Return the integer from a value.
long JsonValue::getNumber() const
{
	checkForNull();
	if (type_f == JSON_NUMBER) {
		return(theInteger_f);
	}
	throw AccessTypeException("Value is not an integer");
}

//
// Return the float from a value.
double JsonValue::getFloat() const
{
	checkForNull();
	if (type_f == JSON_FLOAT) {
		return(theFloat_f);
	} else if (type_f == JSON_NUMBER) {
		//
		// JSON doesn't distinguish between floating point numbers and integers, so allow
		// the caller not to care the exact format of number used in the JSON document.
		return(theInteger_f);
	}
	throw AccessTypeException("Value is not a float");
}

//
// Return the bool from a value.
bool JsonValue::getBool() const
{
	checkForNull();
	if (type_f == JSON_BOOL) {
		return(theBool_f);
	}
	throw AccessTypeException("Value is not a bool");
}

//
// Add a pretty printed version of the tree to the supplied string.
void JsonValue::prettyPrint(std::string &output, std::string indent) const
{
	std::string newIndent = indent + "    ";
	bool first = true;

	switch (type_f) {
	case JSON_OBJECT:
		output += "{";
		for (auto thisNameVal : nameValsInOrder_f) {
			if (first) {
				first = false;
			} else {
				output += ",";
			}

			output += "\n" + newIndent;
			output += "\"" + thisNameVal.first + "\" : ";
			thisNameVal.second.prettyPrint(output, newIndent);
		}

		output += "\n" + indent + "}";
		break;

	case JSON_ARRAY:
		output += "[";
		for (auto it = theArray_f.begin(); it != theArray_f.end(); it++) {
			if (first) {
				first = false;
			} else {
				output += ",";
			}
			output += "\n" + newIndent;
			it->prettyPrint(output, newIndent);
		}
		output += "\n" + indent + "]";
		break;

	case JSON_STRING:
		output += "\"" + theString_f + "\"";
		break;

	case JSON_NUMBER:
		output += std::to_string(theInteger_f);
		break;

	case JSON_FLOAT:
		{
			std::stringstream ss;
			ss << std::scientific;
			ss << std::setprecision(std::numeric_limits<double>::digits10 + 1);
			ss << theFloat_f;
			output += ss.str();
		}
		break;

	case JSON_BOOL:
		output += theBool_f ? "true" : "false";
		break;

	case JSON_NULL:
		output += "null";
		break;

	default:
		throw JsonValue::InternalErrorException("unknown object type in prettyPrint");
		break;
	}
}

//
// Destruct the obect. All we do is set the type to NULL so hopefully we can catch any uses
// after it's destructed.
JsonValue::~JsonValue()
{
	type_f = JSON_NULL;
}

std::string JsonValue::JsonValueException::getMsg()
{
	return(exceptionClass_f + std::string(":") + msg_f);
}

JsonValue::JsonParseException::JsonParseException(const char *msg, const char *objStart)
	: JsonValueException(msg)
{
	object_f = objStart;
	exceptionClass_f = "PARSE ERROR";
}

std::string JsonValue::JsonParseException::getMsg()
{
	char buffer[81];

	snprintf(buffer, sizeof(buffer), " '%s'", object_f.c_str());

	return(exceptionClass_f + std::string(":") + msg_f + buffer);
}
