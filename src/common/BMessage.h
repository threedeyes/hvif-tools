/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef BMESSAGE_H
#define BMESSAGE_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <cstring>

typedef int32_t status_t;
typedef int64_t bigtime_t;

#ifndef __HAIKU__
enum {
	B_OK				= 0,
	B_ERROR				= -1,
	B_NO_MEMORY			= -2,
	B_BAD_VALUE			= -3,
	B_NAME_NOT_FOUND	= -4,
	B_BAD_INDEX			= -5,
	B_BAD_TYPE			= -6,
	B_NO_INIT			= -7,
	B_BAD_DATA			= -8
};
#endif

enum {
	B_ANY_TYPE			= 'ANYT',
	B_BOOL_TYPE			= 'BOOL',
	B_INT8_TYPE			= 'BYTE',
	B_INT16_TYPE		= 'SHRT',
	B_INT32_TYPE		= 'LONG',
	B_INT64_TYPE		= 'LLNG',
	B_UINT8_TYPE		= 'UBYT',
	B_UINT16_TYPE		= 'USHT',
	B_UINT32_TYPE		= 'ULNG',
	B_UINT64_TYPE		= 'ULLG',
	B_FLOAT_TYPE		= 'FLOT',
	B_DOUBLE_TYPE		= 'DBLE',
	B_STRING_TYPE		= 'CSTR',
	B_POINT_TYPE		= 'BPNT',
	B_RECT_TYPE			= 'RECT',
	B_SIZE_TYPE			= 'SIZE',
	B_COLOR_TYPE		= 'RGBC',
	B_POINTER_TYPE		= 'PNTR',
	B_MESSAGE_TYPE		= 'MSGG',
	B_REF_TYPE			= 'RREF',
	B_NODE_REF_TYPE		= 'NREF',
	B_RGB_32_BIT_TYPE	= 'RGBB',
	B_ALIGNMENT_TYPE	= 'ALGN',
	B_MESSENGER_TYPE	= 'MSNG'
};

typedef uint32_t type_code;

struct BPoint {
	float x;
	float y;
	
	BPoint() : x(0), y(0) {}
	BPoint(float x, float y) : x(x), y(y) {}
};

struct BRect {
	float left;
	float top;
	float right;
	float bottom;
	
	BRect() : left(0), top(0), right(0), bottom(0) {}
	BRect(float l, float t, float r, float b)
		: left(l), top(t), right(r), bottom(b) {}
};

struct BSize {
	float width;
	float height;
	
	BSize() : width(0), height(0) {}
	BSize(float w, float h) : width(w), height(h) {}
};

struct rgb_color {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
};

struct entry_ref {
	int32_t		device;
	int64_t		directory;
	char*		name;
	
	entry_ref() : device(0), directory(0), name(NULL) {}
	~entry_ref() { delete[] name; }
};

struct node_ref {
	int32_t		device;
	int64_t		node;
	
	node_ref() : device(0), node(0) {}
};


class BMessage {
public:
								BMessage();
								BMessage(uint32_t what);
								BMessage(const BMessage& other);
	virtual						~BMessage();

			BMessage&			operator=(const BMessage& other);

			// Unflattening
			status_t			Unflatten(const char* flatBuffer,
									ssize_t size = -1);

			// Field info
			status_t			GetInfo(type_code typeRequested, int32_t index,
									char** nameFound, type_code* typeFound,
									int32_t* countFound = NULL) const;
			status_t			GetInfo(const char* name, type_code* typeFound,
									int32_t* countFound = NULL) const;

			int32_t				CountNames(type_code type) const;
			bool				IsEmpty() const;

			// Finding data
			status_t			FindBool(const char* name, bool* value) const;
			status_t			FindBool(const char* name, int32_t index,
									bool* value) const;

			status_t			FindInt8(const char* name, int8_t* value) const;
			status_t			FindInt8(const char* name, int32_t index,
									int8_t* value) const;

			status_t			FindInt16(const char* name,
									int16_t* value) const;
			status_t			FindInt16(const char* name, int32_t index,
									int16_t* value) const;

			status_t			FindInt32(const char* name,
									int32_t* value) const;
			status_t			FindInt32(const char* name, int32_t index,
									int32_t* value) const;

			status_t			FindInt64(const char* name,
									int64_t* value) const;
			status_t			FindInt64(const char* name, int32_t index,
									int64_t* value) const;

			status_t			FindUInt8(const char* name,
									uint8_t* value) const;
			status_t			FindUInt8(const char* name, int32_t index,
									uint8_t* value) const;

			status_t			FindUInt16(const char* name,
									uint16_t* value) const;
			status_t			FindUInt16(const char* name, int32_t index,
									uint16_t* value) const;

			status_t			FindUInt32(const char* name,
									uint32_t* value) const;
			status_t			FindUInt32(const char* name, int32_t index,
									uint32_t* value) const;

			status_t			FindUInt64(const char* name,
									uint64_t* value) const;
			status_t			FindUInt64(const char* name, int32_t index,
									uint64_t* value) const;

			status_t			FindFloat(const char* name, float* value) const;
			status_t			FindFloat(const char* name, int32_t index,
									float* value) const;

			status_t			FindDouble(const char* name,
									double* value) const;
			status_t			FindDouble(const char* name, int32_t index,
									double* value) const;

			status_t			FindString(const char* name,
									const char** string) const;
			status_t			FindString(const char* name, int32_t index,
									const char** string) const;
			status_t			FindString(const char* name,
									std::string* string) const;
			status_t			FindString(const char* name, int32_t index,
									std::string* string) const;

			status_t			FindPoint(const char* name,
									BPoint* point) const;
			status_t			FindPoint(const char* name, int32_t index,
									BPoint* point) const;

			status_t			FindRect(const char* name, BRect* rect) const;
			status_t			FindRect(const char* name, int32_t index,
									BRect* rect) const;

			status_t			FindSize(const char* name, BSize* size) const;
			status_t			FindSize(const char* name, int32_t index,
									BSize* size) const;

			status_t			FindColor(const char* name,
									rgb_color* color) const;
			status_t			FindColor(const char* name, int32_t index,
									rgb_color* color) const;

			status_t			FindPointer(const char* name,
									void** pointer) const;
			status_t			FindPointer(const char* name, int32_t index,
									void** pointer) const;

			status_t			FindMessage(const char* name,
									BMessage* message) const;
			status_t			FindMessage(const char* name, int32_t index,
									BMessage* message) const;

			status_t			FindData(const char* name, type_code type,
									const void** data,
									ssize_t* numBytes) const;
			status_t			FindData(const char* name, type_code type,
									int32_t index, const void** data,
									ssize_t* numBytes) const;

			// Checking for data
			bool				HasBool(const char* name,
									int32_t index = 0) const;
			bool				HasInt8(const char* name,
									int32_t index = 0) const;
			bool				HasInt16(const char* name,
									int32_t index = 0) const;
			bool				HasInt32(const char* name,
									int32_t index = 0) const;
			bool				HasInt64(const char* name,
									int32_t index = 0) const;
			bool				HasUInt8(const char* name,
									int32_t index = 0) const;
			bool				HasUInt16(const char* name,
									int32_t index = 0) const;
			bool				HasUInt32(const char* name,
									int32_t index = 0) const;
			bool				HasUInt64(const char* name,
									int32_t index = 0) const;
			bool				HasFloat(const char* name,
									int32_t index = 0) const;
			bool				HasDouble(const char* name,
									int32_t index = 0) const;
			bool				HasString(const char* name,
									int32_t index = 0) const;
			bool				HasPoint(const char* name,
									int32_t index = 0) const;
			bool				HasRect(const char* name,
									int32_t index = 0) const;
			bool				HasSize(const char* name,
									int32_t index = 0) const;
			bool				HasColor(const char* name,
									int32_t index = 0) const;
			bool				HasPointer(const char* name,
									int32_t index = 0) const;
			bool				HasMessage(const char* name,
									int32_t index = 0) const;
			bool				HasData(const char* name, type_code type,
									int32_t index = 0) const;

			// Convenience getters with defaults
			bool				GetBool(const char* name,
									bool defaultValue = false) const;
			bool				GetBool(const char* name, int32_t index,
									bool defaultValue) const;

			int32_t				GetInt32(const char* name,
									int32_t defaultValue = 0) const;
			int32_t				GetInt32(const char* name, int32_t index,
									int32_t defaultValue) const;

			float				GetFloat(const char* name,
									float defaultValue = 0.0f) const;
			float				GetFloat(const char* name, int32_t index,
									float defaultValue) const;

			const char*			GetString(const char* name,
									const char* defaultValue = NULL) const;
			const char*			GetString(const char* name, int32_t index,
									const char* defaultValue) const;

			// Debugging
			void				PrintToStream() const;
			void				PrintToStream(bool showValues) const;

			uint32_t			what;

private:
			enum {
				MESSAGE_FORMAT_R5				= 'FOB1',
				MESSAGE_FORMAT_R5_SWAPPED		= '1BOF',
				MESSAGE_FORMAT_DANO				= 'FOB2',
				MESSAGE_FORMAT_DANO_SWAPPED		= '2BOF',
				MESSAGE_FORMAT_HAIKU			= '1FMH',
				MESSAGE_FORMAT_HAIKU_SWAPPED	= 'HMF1',
				FIELD_FLAG_VALID				= 0x0001,
				FIELD_FLAG_FIXED_SIZE			= 0x0002,
				MESSAGE_BODY_HASH_TABLE_SIZE	= 5,
				// R5 specific flags
				R5_MESSAGE_FLAG_VALID			= 0x01,
				R5_MESSAGE_FLAG_INCLUDE_TARGET	= 0x02,
				R5_MESSAGE_FLAG_INCLUDE_REPLY	= 0x04,
				R5_MESSAGE_FLAG_SCRIPT_MESSAGE	= 0x08,
				R5_FIELD_FLAG_VALID				= 0x01,
				R5_FIELD_FLAG_MINI_DATA			= 0x02,
				R5_FIELD_FLAG_FIXED_SIZE		= 0x04,
				R5_FIELD_FLAG_SINGLE_ITEM		= 0x08
			};

#pragma pack(push, 1)
			struct message_header {
				uint32_t	format;
				uint32_t	flags;
				uint32_t	what;
				uint32_t	unused1;
				uint32_t	unused2;
				uint32_t	unused3;
				uint32_t	unused4;
				int32_t		current_specifier;
				int32_t		message_area;
				uint32_t	data_size;
				uint32_t	field_count;
				uint32_t	hash_table_size;
				int32_t		hash_table[5];
			};

			struct field_header {
				uint16_t	flags;
				uint16_t	name_length;
				uint32_t	type;
				uint32_t	count;
				uint32_t	data_size;
				uint32_t	offset;
				int32_t		next_field;
			};

			// R5 format header (completely different!)
			struct r5_message_header {
				uint32_t	magic;
				uint32_t	checksum;
				int32_t		flattened_size;
				int32_t		what;
				uint8_t		flags;
			};
#pragma pack(pop)

			message_header*		fHeader;
			field_header*		fFields;
			uint8_t*			fData;
			bool				fNeedSwap;

			status_t			_InitHeader();
			status_t			_Clear();
			status_t			_ValidateMessage();

			uint32_t			_HashName(const char* name) const;
			status_t			_FindField(const char* name, type_code type,
									field_header** result) const;

			void				_PrintToStream(const char* indent,
									bool showValues) const;
			const char*			_TypeCodeToString(type_code type) const;

			// R5 format support
			status_t			_UnflattenR5Message(const uint8_t* buffer,
									ssize_t size);
			status_t			_AddR5Field(const char* name, type_code type,
									const void* data, ssize_t dataSize,
									bool fixedSize, int32_t count);

	static	uint32_t			_SwapUInt32(uint32_t value);
	static	uint16_t			_SwapUInt16(uint16_t value);
	static	int32_t				_SwapInt32(int32_t value);

			void				_SwapHeaderFields();
			void				_SwapFieldHeader(field_header* field);

	static	int32_t				_Pad8(int32_t value) {
									return (value + 7) & ~7;
								}
};

#endif
