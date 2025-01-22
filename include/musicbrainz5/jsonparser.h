// (c) Mike Moreton 2024.  This code probably has bugs. If that matters to you, fix them before use.
//
// This code parses a JSON document, and returns a tree of C++ objects that represent the JSON
// document.  JSON documents are a tree of JSON Values which can be things like numbers or
// strings, or an array of more values.  But a JSON Value can also be a JSON Object, which is
// a map from names to more JSON values.  A JSON document has a single top-level JSON value, which
// must be of type Object.
//
// We represent this by providing a tree of JsonValue C++ objects, with an internal type field
// that specifies exactly what sort of JSON Value this is - number, string, array or map (i.e.
// JSON object). 
//
// The implementation is not designed to be fast.  It should be safe to copy trees of JsonValues
// as free and delete are not used in the implementation.
//
// It might be possible to have C++ sub-classes for the different sorts of JSON Value, but this
// is probably more trouble than it's worth.
//

#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <string>
#include <exception>
#include <map>
#include <vector>
#include <string>
#include <list>

class JsonValue {
public:
	//
	// This is the parent class for all exceptions thrown by the parser.
	class JsonValueException : public std::exception {
		public:
			JsonValueException(const char *msg) { msg_f = msg; };
			virtual std::string getMsg();

		protected:
			const char *exceptionClass_f = "UNKNOWN ERROR";
			std::string msg_f = "<NO MESSAGE>";
	};

	//
	// This exception is thrown when there is an error in the JSON document being parsed.
	class JsonParseException : public JsonValueException {
		public:
			JsonParseException(const char *msg, const char *obj);
			virtual std::string getMsg();

		protected:

		private:
			std::string object_f = "<NO OBJECT>";
	};

	//
	// This exception is thrown when an index provided is out of range for this JsonValue.
	class NotFoundException:public JsonValueException {
		public:
			NotFoundException(const char *msg) : JsonValueException(msg)
					{exceptionClass_f = "NOT FOUND";};
	};

	//
	// This exception is thrown when you try and do something invalid for this type of
	// JsonValue, like trying to read the numeric value of a string, or indexing something
	// that isn't a JSON Object or JSON Array.
	class AccessTypeException:public JsonValueException {
		public:
			AccessTypeException(const char *msg) : JsonValueException(msg)
					{exceptionClass_f = "ACCESS TYPE";};
	};

	//
	// This exception is thrown when you try to access the value of a JsonValue and that
	// JsonValue is a JSON null.
	class NullTypeException:public JsonValueException {
		public:
			NullTypeException(const char *msg) : JsonValueException(msg)
					{exceptionClass_f = "NULL TYPE";};
	};

	//
	// Thrown for internal data consistency errors.  Hopefully this should never happen!
	class InternalErrorException:public JsonValueException {
		public:
			InternalErrorException(const char *msg) : JsonValueException(msg)
					{exceptionClass_f = "INTERNAL ERROR";};
	};

	//
	// The different types of JSON Value.  Note that we also add a JSON float for a number that
	// was parsed as a float, because trying to access a float as an integer is probably an error.
	enum VAL_TYPE {JSON_OBJECT, JSON_ARRAY, JSON_STRING, JSON_NUMBER, JSON_BOOL, JSON_NULL,
			JSON_FLOAT};

	~JsonValue();

	//
	// Read the JSON document from the supplied filename.
	void read(const char *filename);

	//
	// Read the JSON document from the supplied string.
	void parse(const std::string &jsonText);

	//
	// If the JsonValue is a JSON object, look up the JsonValue mapped to the supplied name.
	const JsonValue &operator[](std::string &name) const;
	const JsonValue &operator[](const char *name) const;

	//
	// If the JsonValue is a JSON object, return true if there is a JsonValue mapped to the
	// supplied name.
	bool has(std::string &name) const;
	bool has(const char * name) const;

	//
	// If the JsonValue is a JSON array, return the JsonValue at the indicated index.
	const JsonValue &operator[](unsigned idx) const;
	const JsonValue &operator[](int idx) const;


	//
	// If the JsonValue is a JSON array, return the length of the array.
	unsigned length() const;

	//
	// Return the type of JSON object that the JsonValue is.
	enum VAL_TYPE getType() const { return(type_f); };

	//
	// If the JsonValue is a JSON object, return the map of names to values.
	const std::map<std::string, JsonValue> &getMap() const;

	//
	// If the JsonValue is a JSON array, return the vector of JsonValues.
	const std::vector<JsonValue> &getArray() const;

	//
	// If the JsonValue is a JSON number, which was not specified as a float, return the integer.
	long getNumber() const;
	operator long() const { return(getNumber()); }

	//
	// If the JsonValue is a JSON number, return the number whether it was specified as an integer
	// or a float.
	double getFloat() const;
	operator double() const { return(getFloat()); }

	//
	// If the JsonValue is a JSON bool, return the value of the bool.
	bool getBool() const;
	operator bool() const { return(getBool()); }

	//
	// If the JsonValue is a JSON string, return the string.
	const std::string &getString() const;
	operator const std::string() const { return(getString()); }

	//
	// Output the contents of the JsonValue (and all objects in the tree) as JSON text.
	void prettyPrint(std::string &output, std::string indent = std::string("")) const;

private:
	void skipWhitespace(const char **buffer);
	const char *parseObject(const char *buffer);
	const char *parseValue(const char *buffer);
	const char *parseArray(const char *buffer);
	std::string parseString(const char **buffer);
	void checkForNull() const;

	enum VAL_TYPE type_f = JSON_NULL;

	std::map<std::string, JsonValue> nameValPairs_f;	// The map for a JSON Object
	std::list<std::pair<std::string, JsonValue>> nameValsInOrder_f; // As above - used to keep order
	std::string theString_f;			// The string for a JSON String
	bool theBool_f;						// The bool for a JSON Bool
	long theInteger_f;					// The value of a JSON Number specified as an integer
	double theFloat_f;					// The value of a JSON Number specified as a float
	std::vector<JsonValue> theArray_f;	// The array for a JSON Array
};

#endif // JSONPARSER_H
