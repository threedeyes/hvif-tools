/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <cstdio>
#include <cstdlib>
#include <cctype>

#include "BMessage.h"

namespace haiku_compat {

uint32_t
BMessage::_SwapUInt32(uint32_t value)
{
	return ((value & 0xFF000000) >> 24)
		| ((value & 0x00FF0000) >> 8)
		| ((value & 0x0000FF00) << 8)
		| ((value & 0x000000FF) << 24);
}


uint16_t
BMessage::_SwapUInt16(uint16_t value)
{
	return ((value & 0xFF00) >> 8)
		| ((value & 0x00FF) << 8);
}


int32_t
BMessage::_SwapInt32(int32_t value)
{
	return (int32_t)_SwapUInt32((uint32_t)value);
}


void
BMessage::_SwapHeaderFields()
{
	if (!fNeedSwap || fHeader == NULL)
		return;

	fHeader->format = _SwapUInt32(fHeader->format);
	fHeader->flags = _SwapUInt32(fHeader->flags);
	fHeader->what = _SwapUInt32(fHeader->what);
	fHeader->unused1 = _SwapUInt32(fHeader->unused1);
	fHeader->unused2 = _SwapUInt32(fHeader->unused2);
	fHeader->unused3 = _SwapUInt32(fHeader->unused3);
	fHeader->unused4 = _SwapUInt32(fHeader->unused4);
	fHeader->current_specifier = _SwapInt32(fHeader->current_specifier);
	fHeader->message_area = _SwapInt32(fHeader->message_area);
	fHeader->data_size = _SwapUInt32(fHeader->data_size);
	fHeader->field_count = _SwapUInt32(fHeader->field_count);
	fHeader->hash_table_size = _SwapUInt32(fHeader->hash_table_size);

	for (uint32_t i = 0; i < 5; i++)
		fHeader->hash_table[i] = _SwapInt32(fHeader->hash_table[i]);
}


void
BMessage::_SwapFieldHeader(field_header* field)
{
	if (!fNeedSwap || field == NULL)
		return;

	field->flags = _SwapUInt16(field->flags);
	field->name_length = _SwapUInt16(field->name_length);
	field->type = _SwapUInt32(field->type);
	field->count = _SwapUInt32(field->count);
	field->data_size = _SwapUInt32(field->data_size);
	field->offset = _SwapUInt32(field->offset);
	field->next_field = _SwapInt32(field->next_field);
}


BMessage::BMessage()
	:
	what(0),
	fHeader(NULL),
	fFields(NULL),
	fData(NULL),
	fNeedSwap(false)
{
}


BMessage::BMessage(uint32_t whatValue)
	:
	what(whatValue),
	fHeader(NULL),
	fFields(NULL),
	fData(NULL),
	fNeedSwap(false)
{
}


BMessage::BMessage(const BMessage& other)
	:
	what(other.what),
	fHeader(NULL),
	fFields(NULL),
	fData(NULL),
	fNeedSwap(other.fNeedSwap)
{
	*this = other;
}


BMessage::~BMessage()
{
	_Clear();
}


BMessage&
BMessage::operator=(const BMessage& other)
{
	if (this == &other)
		return *this;

	_Clear();

	if (other.fHeader == NULL)
		return *this;

	fHeader = (message_header*)malloc(sizeof(message_header));
	if (fHeader == NULL)
		return *this;

	memcpy(fHeader, other.fHeader, sizeof(message_header));
	fHeader->message_area = -1;
	fNeedSwap = other.fNeedSwap;

	if (fHeader->field_count > 0) {
		size_t fieldsSize = fHeader->field_count * sizeof(field_header);
		fFields = (field_header*)malloc(fieldsSize);
		if (fFields == NULL) {
			free(fHeader);
			fHeader = NULL;
			return *this;
		}
		memcpy(fFields, other.fFields, fieldsSize);
	}

	if (fHeader->data_size > 0) {
		fData = (uint8_t*)malloc(fHeader->data_size);
		if (fData == NULL) {
			free(fFields);
			free(fHeader);
			fFields = NULL;
			fHeader = NULL;
			return *this;
		}
		memcpy(fData, other.fData, fHeader->data_size);
	}

	what = other.what;
	return *this;
}


status_t
BMessage::Unflatten(const char* flatBuffer, ssize_t size)
{
	if (flatBuffer == NULL)
		return B_BAD_VALUE;

	_Clear();

	const uint8_t* buffer = (const uint8_t*)flatBuffer;

	uint32_t format;
	memcpy(&format, buffer, 4);

	bool needSwap = false;
	if (format == MESSAGE_FORMAT_HAIKU_SWAPPED
		|| format == MESSAGE_FORMAT_R5_SWAPPED
		|| format == MESSAGE_FORMAT_DANO_SWAPPED) {
		needSwap = true;
		format = _SwapUInt32(format);
	}

	if (format == MESSAGE_FORMAT_R5) {
		fNeedSwap = needSwap;
		return _UnflattenR5Message(buffer, size);
	}

	if (format == MESSAGE_FORMAT_DANO) {
		return B_BAD_DATA;
	}

	if (format != MESSAGE_FORMAT_HAIKU) {
		return B_BAD_DATA;
	}

	fNeedSwap = needSwap;

	fHeader = (message_header*)malloc(sizeof(message_header));
	if (fHeader == NULL)
		return B_NO_MEMORY;

	memcpy(fHeader, flatBuffer, sizeof(message_header));

	if (fNeedSwap)
		_SwapHeaderFields();

	if (fHeader->field_count > 10000) {
		_Clear();
		return B_BAD_DATA;
	}

	if (size > 0 && fHeader->data_size > (uint32_t)size) {
		_Clear();
		return B_BAD_DATA;
	}

	what = fHeader->what;

	buffer = (const uint8_t*)flatBuffer + sizeof(message_header);

	if (fHeader->field_count > 0) {
		size_t fieldsSize = fHeader->field_count * sizeof(field_header);

		if (size > 0 && (size_t)size < sizeof(message_header) + fieldsSize) {
			_Clear();
			return B_BAD_DATA;
		}

		fFields = (field_header*)malloc(fieldsSize);
		if (fFields == NULL) {
			_Clear();
			return B_NO_MEMORY;
		}

		memcpy(fFields, buffer, fieldsSize);

		if (fNeedSwap) {
			for (uint32_t i = 0; i < fHeader->field_count; i++)
				_SwapFieldHeader(&fFields[i]);
		}

		buffer += fieldsSize;
	}

	if (fHeader->data_size > 0) {
		if (size > 0 && (size_t)size < sizeof(message_header)
			+ fHeader->field_count * sizeof(field_header)
			+ fHeader->data_size) {
			_Clear();
			return B_BAD_DATA;
		}

		fData = (uint8_t*)malloc(fHeader->data_size);
		if (fData == NULL) {
			_Clear();
			return B_NO_MEMORY;
		}

		memcpy(fData, buffer, fHeader->data_size);
	}

	return _ValidateMessage();
}


ssize_t
BMessage::FlattenedSize() const
{
	if (fHeader == NULL)
		return B_NO_INIT;

	return sizeof(message_header) + fHeader->field_count * sizeof(field_header)
		+ fHeader->data_size;
}


status_t
BMessage::Flatten(char* buffer, ssize_t size) const
{
	if (buffer == NULL || size < 0)
		return B_BAD_VALUE;

	if (fHeader == NULL)
		return B_NO_INIT;

	if (size < FlattenedSize())
		return B_BUFFER_OVERFLOW;

	message_header tmpHeader;
	memcpy(&tmpHeader, fHeader, sizeof(message_header));
	tmpHeader.what = what;

	memcpy(buffer, &tmpHeader, sizeof(message_header));
	buffer += sizeof(message_header);

	size_t fieldsSize = fHeader->field_count * sizeof(field_header);
	if (fieldsSize > 0) {
		memcpy(buffer, fFields, fieldsSize);
		buffer += fieldsSize;
	}

	if (fHeader->data_size > 0)
		memcpy(buffer, fData, fHeader->data_size);

	return B_OK;
}


status_t
BMessage::_UnflattenR5Message(const uint8_t* buffer, ssize_t totalSize)
{
	status_t result = _InitHeader();
	if (result != B_OK)
		return result;

	r5_message_header r5header;
	memcpy(&r5header, buffer, sizeof(r5_message_header));

	if (fNeedSwap) {
		r5header.checksum = _SwapUInt32(r5header.checksum);
		r5header.flattened_size = _SwapInt32(r5header.flattened_size);
		r5header.what = _SwapInt32(r5header.what);
	}

	what = fHeader->what = r5header.what;

	const uint8_t* pointer = buffer + sizeof(r5_message_header);
	const uint8_t* end = buffer + (totalSize > 0 ? totalSize : r5header.flattened_size);

	if (r5header.flags & R5_MESSAGE_FLAG_INCLUDE_TARGET) {
		if (pointer + sizeof(int32_t) > end)
			return B_BAD_DATA;
		pointer += sizeof(int32_t);
	}

	if (r5header.flags & R5_MESSAGE_FLAG_INCLUDE_REPLY) {
		if (pointer + sizeof(int32_t) * 3 + 4 > end)
			return B_BAD_DATA;
		pointer += sizeof(int32_t) * 3 + 4;
	}

	while (pointer < end) {
		uint8_t flags = *pointer++;
		if ((flags & R5_FIELD_FLAG_VALID) == 0)
			break;

		if (pointer + sizeof(type_code) > end)
			return B_BAD_DATA;

		type_code type;
		memcpy(&type, pointer, sizeof(type_code));
		pointer += sizeof(type_code);
		if (fNeedSwap)
			type = _SwapUInt32(type);

		int32_t itemCount = 1;
		if ((flags & R5_FIELD_FLAG_SINGLE_ITEM) == 0) {
			if (flags & R5_FIELD_FLAG_MINI_DATA) {
				if (pointer + 1 > end)
					return B_BAD_DATA;
				itemCount = *pointer++;
			} else {
				if (pointer + sizeof(int32_t) > end)
					return B_BAD_DATA;
				memcpy(&itemCount, pointer, sizeof(int32_t));
				pointer += sizeof(int32_t);
				if (fNeedSwap)
					itemCount = _SwapInt32(itemCount);
			}
		}

		int32_t dataSize;
		if (flags & R5_FIELD_FLAG_MINI_DATA) {
			if (pointer + 1 > end)
				return B_BAD_DATA;
			dataSize = *pointer++;
		} else {
			if (pointer + sizeof(int32_t) > end)
				return B_BAD_DATA;
			memcpy(&dataSize, pointer, sizeof(int32_t));
			pointer += sizeof(int32_t);
			if (fNeedSwap)
				dataSize = _SwapInt32(dataSize);
		}

		if (dataSize < 0 || dataSize > 100 * 1024 * 1024)
			return B_BAD_DATA;

		if (pointer + 1 > end)
			return B_BAD_DATA;
		uint8_t nameLength = *pointer++;

		if (pointer + nameLength > end)
			return B_BAD_DATA;

		char nameBuffer[256];
		memcpy(nameBuffer, pointer, nameLength);
		nameBuffer[nameLength] = '\0';
		pointer += nameLength;

		if (pointer + dataSize > end)
			return B_BAD_DATA;

		const uint8_t* dataPointer = pointer;
		bool fixedSize = (flags & R5_FIELD_FLAG_FIXED_SIZE) != 0;

		if (fixedSize) {
			int32_t itemSize = dataSize / itemCount;
			for (int32_t i = 0; i < itemCount; i++) {
				result = _AddR5Field(nameBuffer, type, dataPointer, itemSize,
					true, itemCount);
				if (result != B_OK)
					return result;
				dataPointer += itemSize;
			}
		} else {
			for (int32_t i = 0; i < itemCount; i++) {
				if (dataPointer + sizeof(int32_t) > pointer + dataSize)
					return B_BAD_DATA;

				int32_t itemSize;
				memcpy(&itemSize, dataPointer, sizeof(int32_t));
				if (fNeedSwap)
					itemSize = _SwapInt32(itemSize);

				dataPointer += sizeof(int32_t);

				if (dataPointer + itemSize > pointer + dataSize)
					return B_BAD_DATA;

				result = _AddR5Field(nameBuffer, type, dataPointer, itemSize,
					false, itemCount);
				if (result != B_OK)
					return result;

				dataPointer += _Pad8(itemSize + sizeof(int32_t)) - sizeof(int32_t);
			}
		}

		pointer += dataSize;
	}

	return B_OK;
}


status_t
BMessage::_AddR5Field(const char* name, type_code type, const void* data,
	ssize_t dataSize, bool fixedSize, int32_t count)
{
	if (fHeader == NULL)
		return B_NO_INIT;

	field_header* field = NULL;
	for (uint32_t i = 0; i < fHeader->field_count; i++) {
		const char* fieldName = (const char*)(fData + fFields[i].offset);
		if (strcmp(fieldName, name) == 0) {
			field = &fFields[i];
			break;
		}
	}

	if (field == NULL) {
		fHeader->field_count++;
		field_header* newFields = (field_header*)realloc(fFields,
			fHeader->field_count * sizeof(field_header));
		if (newFields == NULL) {
			fHeader->field_count--;
			return B_NO_MEMORY;
		}
		fFields = newFields;
		field = &fFields[fHeader->field_count - 1];

		field->flags = FIELD_FLAG_VALID;
		if (fixedSize)
			field->flags |= FIELD_FLAG_FIXED_SIZE;
		field->type = type;
		field->count = 0;
		field->data_size = 0;
		field->offset = fHeader->data_size;
		field->next_field = -1;
		field->name_length = strlen(name) + 1;

		uint8_t* newData = (uint8_t*)realloc(fData,
			fHeader->data_size + field->name_length);
		if (newData == NULL)
			return B_NO_MEMORY;
		fData = newData;

		memcpy(fData + fHeader->data_size, name, field->name_length);
		fHeader->data_size += field->name_length;
	}

	ssize_t sizeToAdd = dataSize;
	if (!fixedSize)
		sizeToAdd += sizeof(uint32_t);

	uint8_t* newData = (uint8_t*)realloc(fData, fHeader->data_size + sizeToAdd);
	if (newData == NULL)
		return B_NO_MEMORY;
	fData = newData;

	if (!fixedSize) {
		uint32_t size = (uint32_t)dataSize;
		memcpy(fData + fHeader->data_size, &size, sizeof(uint32_t));
		memcpy(fData + fHeader->data_size + sizeof(uint32_t), data, dataSize);
	} else {
		memcpy(fData + fHeader->data_size, data, dataSize);
	}

	fHeader->data_size += sizeToAdd;
	field->data_size += sizeToAdd;
	field->count++;

	return B_OK;
}


status_t
BMessage::_InitHeader()
{
	if (fHeader == NULL) {
		fHeader = (message_header*)malloc(sizeof(message_header));
		if (fHeader == NULL)
			return B_NO_MEMORY;
	}

	memset(fHeader, 0, sizeof(message_header));
	fHeader->format = MESSAGE_FORMAT_HAIKU;
	fHeader->flags = 0;
	fHeader->what = what;
	fHeader->current_specifier = -1;
	fHeader->message_area = -1;
	fHeader->unused1 = 0xFFFFFFFF;
	fHeader->unused2 = 0xFFFFFFFF;
	fHeader->unused3 = 0xFFFFFFFF;
	fHeader->unused4 = 0xFFFFFFFF;
	fHeader->hash_table_size = MESSAGE_BODY_HASH_TABLE_SIZE;
	memset(fHeader->hash_table, 0xFF, sizeof(fHeader->hash_table));
	fNeedSwap = false;

	return B_OK;
}


status_t
BMessage::_Clear()
{
	free(fHeader);
	fHeader = NULL;

	free(fFields);
	fFields = NULL;

	free(fData);
	fData = NULL;

	fNeedSwap = false;

	return B_OK;
}


status_t
BMessage::_ValidateMessage()
{
	if (fHeader == NULL)
		return B_NO_INIT;

	if (fHeader->field_count == 0)
		return B_OK;

	if (fFields == NULL)
		return B_NO_INIT;

	if (fData == NULL && fHeader->data_size > 0)
		return B_NO_INIT;

	for (uint32_t i = 0; i < fHeader->field_count; i++) {
		field_header* field = &fFields[i];

		if (field->next_field >= 0
			&& (uint32_t)field->next_field > fHeader->field_count) {
			_Clear();
			_InitHeader();
			return B_BAD_VALUE;
		}

		uint32_t fieldEnd = field->offset + field->name_length
			+ field->data_size;
		if (fieldEnd > fHeader->data_size) {
			_Clear();
			_InitHeader();
			return B_BAD_VALUE;
		}
	}

	return B_OK;
}


uint32_t
BMessage::_HashName(const char* name) const
{
	char ch;
	uint32_t result = 0;

	while ((ch = *name++) != 0) {
		result = (result << 7) ^ (result >> 24);
		result ^= ch;
	}

	result ^= result << 12;
	return result;
}


status_t
BMessage::_FindField(const char* name, type_code type,
	field_header** result) const
{
	if (name == NULL)
		return B_BAD_VALUE;

	if (fHeader == NULL)
		return B_NO_INIT;

	if (fHeader->field_count == 0 || fFields == NULL || fData == NULL)
		return B_NAME_NOT_FOUND;

	uint32_t hash = _HashName(name) % fHeader->hash_table_size;
	int32_t nextField = fHeader->hash_table[hash];

	while (nextField >= 0) {
		field_header* field = &fFields[nextField];
		if ((field->flags & FIELD_FLAG_VALID) == 0)
			break;

		if (strncmp((const char*)(fData + field->offset), name,
			field->name_length) == 0) {
			if (type != B_ANY_TYPE && field->type != type)
				return B_BAD_TYPE;

			*result = field;
			return B_OK;
		}

		nextField = field->next_field;
	}

	for (uint32_t i = 0; i < fHeader->field_count; i++) {
		field_header* field = &fFields[i];
		if ((field->flags & FIELD_FLAG_VALID) == 0)
			continue;

		if (strncmp((const char*)(fData + field->offset), name,
			field->name_length) == 0) {
			if (type != B_ANY_TYPE && field->type != type)
				return B_BAD_TYPE;

			*result = field;
			return B_OK;
		}
	}

	return B_NAME_NOT_FOUND;
}


void
BMessage::_UpdateOffsets(uint32_t offset, int32_t change)
{
	if (offset < fHeader->data_size) {
		field_header* field = fFields;
		for (uint32_t i = 0; i < fHeader->field_count; i++, field++) {
			if (field->offset >= offset)
				field->offset += change;
		}
	}
}


status_t
BMessage::_ResizeData(uint32_t offset, int32_t change)
{
	if (change == 0)
		return B_OK;

	if (change > 0) {
		size_t newSize = fHeader->data_size + change;
		uint8_t* newData = (uint8_t*)realloc(fData, newSize);
		if (newSize > 0 && newData == NULL)
			return B_NO_MEMORY;

		fData = newData;

		if (offset < fHeader->data_size) {
			memmove(fData + offset + change, fData + offset,
				fHeader->data_size - offset);
			fHeader->data_size += change;
			_UpdateOffsets(offset, change);
		} else {
			fHeader->data_size += change;
		}
	} else {
		ssize_t length = fHeader->data_size - offset + change;
		if (length > 0)
			memmove(fData + offset, fData + offset - change, length);

		fHeader->data_size += change;
		_UpdateOffsets(offset, change);

		if (fHeader->data_size > 0) {
			uint8_t* newData = (uint8_t*)realloc(fData, fHeader->data_size);
			if (newData != NULL)
				fData = newData;
		} else {
			free(fData);
			fData = NULL;
		}
	}

	return B_OK;
}


status_t
BMessage::_AddField(const char* name, type_code type, bool isFixedSize,
	field_header** result)
{
	if (fHeader == NULL) {
		status_t status = _InitHeader();
		if (status != B_OK)
			return status;
	}

	fHeader->field_count++;
	field_header* newFields = (field_header*)realloc(fFields,
		fHeader->field_count * sizeof(field_header));
	if (newFields == NULL) {
		fHeader->field_count--;
		return B_NO_MEMORY;
	}
	fFields = newFields;

	uint32_t hash = _HashName(name) % fHeader->hash_table_size;
	int32_t* nextField = &fHeader->hash_table[hash];
	while (*nextField >= 0)
		nextField = &fFields[*nextField].next_field;
	*nextField = fHeader->field_count - 1;

	field_header* field = &fFields[fHeader->field_count - 1];
	field->type = type;
	field->count = 0;
	field->data_size = 0;
	field->next_field = -1;
	field->offset = fHeader->data_size;
	field->name_length = strlen(name) + 1;

	status_t status = _ResizeData(field->offset, field->name_length);
	if (status != B_OK) {
		fHeader->field_count--;
		return status;
	}

	memcpy(fData + field->offset, name, field->name_length);
	field->flags = FIELD_FLAG_VALID;
	if (isFixedSize)
		field->flags |= FIELD_FLAG_FIXED_SIZE;

	*result = field;
	return B_OK;
}


status_t
BMessage::_RemoveField(field_header* field)
{
	status_t result = _ResizeData(field->offset, -(field->data_size
		+ field->name_length));
	if (result != B_OK)
		return result;

	int32_t index = ((uint8_t*)field - (uint8_t*)fFields) / sizeof(field_header);
	int32_t nextField = field->next_field;
	if (nextField > index)
		nextField--;

	int32_t* value = fHeader->hash_table;
	for (uint32_t i = 0; i < fHeader->hash_table_size; i++, value++) {
		if (*value > index)
			*value -= 1;
		else if (*value == index)
			*value = nextField;
	}

	field_header* other = fFields;
	for (uint32_t i = 0; i < fHeader->field_count; i++, other++) {
		if (other->next_field > index)
			other->next_field--;
		else if (other->next_field == index)
			other->next_field = nextField;
	}

	size_t size = (fHeader->field_count - index - 1) * sizeof(field_header);
	memmove(fFields + index, fFields + index + 1, size);
	fHeader->field_count--;

	if (fHeader->field_count > 0) {
		field_header* newFields = (field_header*)realloc(fFields,
			fHeader->field_count * sizeof(field_header));
		if (newFields != NULL)
			fFields = newFields;
	} else {
		free(fFields);
		fFields = NULL;
	}

	return B_OK;
}


status_t
BMessage::GetInfo(type_code typeRequested, int32_t index, char** nameFound,
	type_code* typeFound, int32_t* countFound) const
{
	if (fHeader == NULL)
		return B_NO_INIT;

	if (index < 0 || (uint32_t)index >= fHeader->field_count)
		return B_BAD_INDEX;

	if (typeRequested == B_ANY_TYPE) {
		if (nameFound != NULL)
			*nameFound = (char*)fData + fFields[index].offset;
		if (typeFound != NULL)
			*typeFound = fFields[index].type;
		if (countFound != NULL)
			*countFound = fFields[index].count;
		return B_OK;
	}

	int32_t counter = -1;
	field_header* field = fFields;
	for (uint32_t i = 0; i < fHeader->field_count; i++, field++) {
		if (field->type == typeRequested)
			counter++;

		if (counter == index) {
			if (nameFound != NULL)
				*nameFound = (char*)fData + field->offset;
			if (typeFound != NULL)
				*typeFound = field->type;
			if (countFound != NULL)
				*countFound = field->count;
			return B_OK;
		}
	}

	return B_BAD_INDEX;
}


status_t
BMessage::GetInfo(const char* name, type_code* typeFound,
	int32_t* countFound) const
{
	field_header* field = NULL;
	status_t result = _FindField(name, B_ANY_TYPE, &field);
	if (result != B_OK)
		return result;

	if (typeFound != NULL)
		*typeFound = field->type;
	if (countFound != NULL)
		*countFound = field->count;

	return B_OK;
}


int32_t
BMessage::CountNames(type_code type) const
{
	if (fHeader == NULL)
		return 0;

	if (type == B_ANY_TYPE)
		return fHeader->field_count;

	int32_t count = 0;
	field_header* field = fFields;
	for (uint32_t i = 0; i < fHeader->field_count; i++, field++) {
		if (field->type == type)
			count++;
	}

	return count;
}


bool
BMessage::IsEmpty() const
{
	return fHeader == NULL || fHeader->field_count == 0;
}


status_t
BMessage::MakeEmpty()
{
	_Clear();
	return _InitHeader();
}


status_t
BMessage::AddData(const char* name, type_code type, const void* data,
	ssize_t numBytes, bool isFixedSize, int32_t count)
{
	if (numBytes <= 0 || data == NULL)
		return B_BAD_VALUE;

	if (fHeader == NULL) {
		status_t status = _InitHeader();
		if (status != B_OK)
			return status;
	}

	field_header* field = NULL;
	status_t result = _FindField(name, type, &field);
	if (result == B_NAME_NOT_FOUND)
		result = _AddField(name, type, isFixedSize, &field);

	if (result != B_OK)
		return result;

	if (field == NULL)
		return B_ERROR;

	uint32_t offset = field->offset + field->name_length + field->data_size;
	if ((field->flags & FIELD_FLAG_FIXED_SIZE) != 0) {
		if (field->count) {
			ssize_t size = field->data_size / field->count;
			if (size != numBytes)
				return B_BAD_VALUE;
		}

		result = _ResizeData(offset, numBytes);
		if (result != B_OK) {
			if (field->count == 0)
				_RemoveField(field);
			return result;
		}

		memcpy(fData + offset, data, numBytes);
		field->data_size += numBytes;
	} else {
		int32_t change = numBytes + sizeof(uint32_t);
		result = _ResizeData(offset, change);
		if (result != B_OK) {
			if (field->count == 0)
				_RemoveField(field);
			return result;
		}

		uint32_t size = (uint32_t)numBytes;
		memcpy(fData + offset, &size, sizeof(uint32_t));
		memcpy(fData + offset + sizeof(uint32_t), data, size);
		field->data_size += change;
	}

	field->count++;
	return B_OK;
}


#define DEFINE_ADD_FUNCTION(type, typeName, typeCode) \
status_t \
BMessage::Add##typeName(const char* name, type value) \
{ \
	return AddData(name, typeCode, &value, sizeof(type), true); \
}


DEFINE_ADD_FUNCTION(bool, Bool, B_BOOL_TYPE)
DEFINE_ADD_FUNCTION(int8_t, Int8, B_INT8_TYPE)
DEFINE_ADD_FUNCTION(int16_t, Int16, B_INT16_TYPE)
DEFINE_ADD_FUNCTION(int32_t, Int32, B_INT32_TYPE)
DEFINE_ADD_FUNCTION(int64_t, Int64, B_INT64_TYPE)
DEFINE_ADD_FUNCTION(uint8_t, UInt8, B_UINT8_TYPE)
DEFINE_ADD_FUNCTION(uint16_t, UInt16, B_UINT16_TYPE)
DEFINE_ADD_FUNCTION(uint32_t, UInt32, B_UINT32_TYPE)
DEFINE_ADD_FUNCTION(uint64_t, UInt64, B_UINT64_TYPE)
DEFINE_ADD_FUNCTION(float, Float, B_FLOAT_TYPE)
DEFINE_ADD_FUNCTION(double, Double, B_DOUBLE_TYPE)
DEFINE_ADD_FUNCTION(BPoint, Point, B_POINT_TYPE)
DEFINE_ADD_FUNCTION(BRect, Rect, B_RECT_TYPE)
DEFINE_ADD_FUNCTION(BSize, Size, B_SIZE_TYPE)
DEFINE_ADD_FUNCTION(rgb_color, Color, B_RGB_32_BIT_TYPE)

#undef DEFINE_ADD_FUNCTION


status_t
BMessage::AddString(const char* name, const char* string)
{
	if (string == NULL)
		return B_BAD_VALUE;

	return AddData(name, B_STRING_TYPE, string, strlen(string) + 1, false);
}


status_t
BMessage::AddString(const char* name, const std::string& string)
{
	return AddData(name, B_STRING_TYPE, string.c_str(), string.length() + 1,
		false);
}


status_t
BMessage::AddPointer(const char* name, const void* pointer)
{
	return AddData(name, B_POINTER_TYPE, &pointer, sizeof(pointer), true);
}


status_t
BMessage::AddMessage(const char* name, const BMessage* message)
{
	if (message == NULL)
		return B_BAD_VALUE;

	ssize_t size = message->FlattenedSize();
	if (size < 0)
		return (status_t)size;

	char* buffer = (char*)malloc(size);
	if (buffer == NULL)
		return B_NO_MEMORY;

	status_t error = message->Flatten(buffer, size);

	if (error >= B_OK)
		error = AddData(name, B_MESSAGE_TYPE, buffer, size, false);

	free(buffer);
	return error;
}


status_t
BMessage::RemoveData(const char* name, int32_t index)
{
	if (index < 0)
		return B_BAD_INDEX;

	if (fHeader == NULL)
		return B_NO_INIT;

	field_header* field = NULL;
	status_t result = _FindField(name, B_ANY_TYPE, &field);
	if (result != B_OK)
		return result;

	if ((uint32_t)index >= field->count)
		return B_BAD_INDEX;

	if (field->count == 1)
		return _RemoveField(field);

	uint32_t offset = field->offset + field->name_length;
	if ((field->flags & FIELD_FLAG_FIXED_SIZE) != 0) {
		ssize_t size = field->data_size / field->count;
		result = _ResizeData(offset + index * size, -size);
		if (result != B_OK)
			return result;

		field->data_size -= size;
	} else {
		uint8_t* pointer = fData + offset;
		for (int32_t i = 0; i < index; i++) {
			offset += *(uint32_t*)pointer + sizeof(uint32_t);
			pointer = fData + offset;
		}

		size_t currentSize = *(uint32_t*)pointer + sizeof(uint32_t);
		result = _ResizeData(offset, -currentSize);
		if (result != B_OK)
			return result;

		field->data_size -= currentSize;
	}

	field->count--;
	return B_OK;
}


status_t
BMessage::RemoveName(const char* name)
{
	if (fHeader == NULL)
		return B_NO_INIT;

	field_header* field = NULL;
	status_t result = _FindField(name, B_ANY_TYPE, &field);
	if (result != B_OK)
		return result;

	return _RemoveField(field);
}


status_t
BMessage::FindData(const char* name, type_code type, int32_t index,
	const void** data, ssize_t* numBytes) const
{
	if (data == NULL)
		return B_BAD_VALUE;

	*data = NULL;
	field_header* field = NULL;
	status_t result = _FindField(name, type, &field);
	if (result != B_OK)
		return result;

	if (index < 0 || (uint32_t)index >= field->count)
		return B_BAD_INDEX;

	if ((field->flags & FIELD_FLAG_FIXED_SIZE) != 0) {
		size_t bytes = field->data_size / field->count;
		*data = fData + field->offset + field->name_length + index * bytes;
		if (numBytes != NULL)
			*numBytes = bytes;
	} else {
		uint8_t* pointer = fData + field->offset + field->name_length;
		for (int32_t i = 0; i < index; i++)
			pointer += *(uint32_t*)pointer + sizeof(uint32_t);

		*data = pointer + sizeof(uint32_t);
		if (numBytes != NULL)
			*numBytes = *(uint32_t*)pointer;
	}

	return B_OK;
}


status_t
BMessage::FindData(const char* name, type_code type, const void** data,
	ssize_t* numBytes) const
{
	return FindData(name, type, 0, data, numBytes);
}


#define DEFINE_FIND_FUNCTIONS(type, typeName, typeCode) \
status_t \
BMessage::Find##typeName(const char* name, type* value) const \
{ \
	return Find##typeName(name, 0, value); \
} \
\
\
status_t \
BMessage::Find##typeName(const char* name, int32_t index, type* value) const \
{ \
	if (value == NULL) \
		return B_BAD_VALUE; \
	\
	const type* ptr = NULL; \
	ssize_t bytes = 0; \
	status_t error = FindData(name, typeCode, index, \
		(const void**)&ptr, &bytes); \
	\
	if (error == B_OK && bytes == sizeof(type)) \
		*value = *ptr; \
	\
	return error; \
}


DEFINE_FIND_FUNCTIONS(bool, Bool, B_BOOL_TYPE)
DEFINE_FIND_FUNCTIONS(int8_t, Int8, B_INT8_TYPE)
DEFINE_FIND_FUNCTIONS(int16_t, Int16, B_INT16_TYPE)
DEFINE_FIND_FUNCTIONS(int32_t, Int32, B_INT32_TYPE)
DEFINE_FIND_FUNCTIONS(int64_t, Int64, B_INT64_TYPE)
DEFINE_FIND_FUNCTIONS(uint8_t, UInt8, B_UINT8_TYPE)
DEFINE_FIND_FUNCTIONS(uint16_t, UInt16, B_UINT16_TYPE)
DEFINE_FIND_FUNCTIONS(uint32_t, UInt32, B_UINT32_TYPE)
DEFINE_FIND_FUNCTIONS(uint64_t, UInt64, B_UINT64_TYPE)
DEFINE_FIND_FUNCTIONS(float, Float, B_FLOAT_TYPE)
DEFINE_FIND_FUNCTIONS(double, Double, B_DOUBLE_TYPE)
DEFINE_FIND_FUNCTIONS(BPoint, Point, B_POINT_TYPE)
DEFINE_FIND_FUNCTIONS(BRect, Rect, B_RECT_TYPE)
DEFINE_FIND_FUNCTIONS(BSize, Size, B_SIZE_TYPE)
DEFINE_FIND_FUNCTIONS(rgb_color, Color, B_RGB_32_BIT_TYPE)

#undef DEFINE_FIND_FUNCTIONS


status_t
BMessage::FindString(const char* name, const char** string) const
{
	return FindString(name, 0, string);
}


status_t
BMessage::FindString(const char* name, int32_t index,
	const char** string) const
{
	ssize_t bytes;
	return FindData(name, B_STRING_TYPE, index, (const void**)string, &bytes);
}


status_t
BMessage::FindString(const char* name, std::string* string) const
{
	return FindString(name, 0, string);
}


status_t
BMessage::FindString(const char* name, int32_t index,
	std::string* string) const
{
	if (string == NULL)
		return B_BAD_VALUE;

	const char* value;
	status_t error = FindString(name, index, &value);

	if (error == B_OK)
		*string = value;
	else
		string->clear();

	return error;
}


status_t
BMessage::FindPointer(const char* name, void** pointer) const
{
	return FindPointer(name, 0, pointer);
}


status_t
BMessage::FindPointer(const char* name, int32_t index, void** pointer) const
{
	if (pointer == NULL)
		return B_BAD_VALUE;

	void** data = NULL;
	ssize_t size = 0;
	status_t error = FindData(name, B_POINTER_TYPE, index,
		(const void**)&data, &size);

	if (error == B_OK)
		*pointer = *data;
	else
		*pointer = NULL;

	return error;
}


status_t
BMessage::FindMessage(const char* name, BMessage* message) const
{
	return FindMessage(name, 0, message);
}


status_t
BMessage::FindMessage(const char* name, int32_t index,
	BMessage* message) const
{
	if (message == NULL)
		return B_BAD_VALUE;

	void* data = NULL;
	ssize_t size = 0;
	status_t error = FindData(name, B_MESSAGE_TYPE, index,
		(const void**)&data, &size);

	if (error == B_OK)
		error = message->Unflatten((const char*)data, size);
	else
		*message = BMessage();

	return error;
}


#define DEFINE_HAS_FUNCTION(typeName, typeCode) \
bool \
BMessage::Has##typeName(const char* name, int32_t index) const \
{ \
	return HasData(name, typeCode, index); \
}


DEFINE_HAS_FUNCTION(Bool, B_BOOL_TYPE)
DEFINE_HAS_FUNCTION(Int8, B_INT8_TYPE)
DEFINE_HAS_FUNCTION(Int16, B_INT16_TYPE)
DEFINE_HAS_FUNCTION(Int32, B_INT32_TYPE)
DEFINE_HAS_FUNCTION(Int64, B_INT64_TYPE)
DEFINE_HAS_FUNCTION(UInt8, B_UINT8_TYPE)
DEFINE_HAS_FUNCTION(UInt16, B_UINT16_TYPE)
DEFINE_HAS_FUNCTION(UInt32, B_UINT32_TYPE)
DEFINE_HAS_FUNCTION(UInt64, B_UINT64_TYPE)
DEFINE_HAS_FUNCTION(Float, B_FLOAT_TYPE)
DEFINE_HAS_FUNCTION(Double, B_DOUBLE_TYPE)
DEFINE_HAS_FUNCTION(String, B_STRING_TYPE)
DEFINE_HAS_FUNCTION(Point, B_POINT_TYPE)
DEFINE_HAS_FUNCTION(Rect, B_RECT_TYPE)
DEFINE_HAS_FUNCTION(Size, B_SIZE_TYPE)
DEFINE_HAS_FUNCTION(Color, B_RGB_32_BIT_TYPE)
DEFINE_HAS_FUNCTION(Pointer, B_POINTER_TYPE)
DEFINE_HAS_FUNCTION(Message, B_MESSAGE_TYPE)

#undef DEFINE_HAS_FUNCTION


bool
BMessage::HasData(const char* name, type_code type, int32_t index) const
{
	field_header* field = NULL;
	status_t result = _FindField(name, type, &field);
	if (result != B_OK)
		return false;

	if (index < 0 || (uint32_t)index >= field->count)
		return false;

	return true;
}


bool
BMessage::GetBool(const char* name, bool defaultValue) const
{
	return GetBool(name, 0, defaultValue);
}


bool
BMessage::GetBool(const char* name, int32_t index, bool defaultValue) const
{
	bool value;
	if (FindBool(name, index, &value) == B_OK)
		return value;
	return defaultValue;
}


int32_t
BMessage::GetInt32(const char* name, int32_t defaultValue) const
{
	return GetInt32(name, 0, defaultValue);
}


int32_t
BMessage::GetInt32(const char* name, int32_t index, int32_t defaultValue) const
{
	int32_t value;
	if (FindInt32(name, index, &value) == B_OK)
		return value;
	return defaultValue;
}


float
BMessage::GetFloat(const char* name, float defaultValue) const
{
	return GetFloat(name, 0, defaultValue);
}


float
BMessage::GetFloat(const char* name, int32_t index, float defaultValue) const
{
	float value;
	if (FindFloat(name, index, &value) == B_OK)
		return value;
	return defaultValue;
}


const char*
BMessage::GetString(const char* name, const char* defaultValue) const
{
	return GetString(name, 0, defaultValue);
}


const char*
BMessage::GetString(const char* name, int32_t index,
	const char* defaultValue) const
{
	const char* value;
	if (FindString(name, index, &value) == B_OK)
		return value;
	return defaultValue;
}


#define DEFINE_SET_FUNCTION(type, typeName, typeCode) \
status_t \
BMessage::Set##typeName(const char* name, type value) \
{ \
	return SetData(name, typeCode, &value, sizeof(type), true); \
}


DEFINE_SET_FUNCTION(bool, Bool, B_BOOL_TYPE)
DEFINE_SET_FUNCTION(int8_t, Int8, B_INT8_TYPE)
DEFINE_SET_FUNCTION(int16_t, Int16, B_INT16_TYPE)
DEFINE_SET_FUNCTION(int32_t, Int32, B_INT32_TYPE)
DEFINE_SET_FUNCTION(int64_t, Int64, B_INT64_TYPE)
DEFINE_SET_FUNCTION(uint8_t, UInt8, B_UINT8_TYPE)
DEFINE_SET_FUNCTION(uint16_t, UInt16, B_UINT16_TYPE)
DEFINE_SET_FUNCTION(uint32_t, UInt32, B_UINT32_TYPE)
DEFINE_SET_FUNCTION(uint64_t, UInt64, B_UINT64_TYPE)
DEFINE_SET_FUNCTION(float, Float, B_FLOAT_TYPE)
DEFINE_SET_FUNCTION(double, Double, B_DOUBLE_TYPE)
DEFINE_SET_FUNCTION(BPoint, Point, B_POINT_TYPE)
DEFINE_SET_FUNCTION(BRect, Rect, B_RECT_TYPE)
DEFINE_SET_FUNCTION(BSize, Size, B_SIZE_TYPE)
DEFINE_SET_FUNCTION(rgb_color, Color, B_RGB_32_BIT_TYPE)

#undef DEFINE_SET_FUNCTION


status_t
BMessage::SetString(const char* name, const char* value)
{
	if (value == NULL)
		return B_BAD_VALUE;

	return SetData(name, B_STRING_TYPE, value, strlen(value) + 1, false);
}


status_t
BMessage::SetString(const char* name, const std::string& value)
{
	return SetData(name, B_STRING_TYPE, value.c_str(), value.length() + 1,
		false);
}


status_t
BMessage::SetPointer(const char* name, const void* value)
{
	return SetData(name, B_POINTER_TYPE, &value, sizeof(value), true);
}


status_t
BMessage::SetMessage(const char* name, const BMessage* value)
{
	if (value == NULL)
		return B_BAD_VALUE;

	ssize_t size = value->FlattenedSize();
	char* buffer = (char*)malloc(size);
	if (buffer == NULL)
		return B_NO_MEMORY;

	status_t error = value->Flatten(buffer, size);

	if (error >= B_OK)
		error = SetData(name, B_MESSAGE_TYPE, buffer, size, false);

	free(buffer);
	return error;
}


status_t
BMessage::SetData(const char* name, type_code type, const void* data,
	ssize_t numBytes, bool fixedSize)
{
	if (numBytes <= 0 || data == NULL)
		return B_BAD_VALUE;

	if (fHeader == NULL) {
		status_t status = _InitHeader();
		if (status != B_OK)
			return status;
	}

	RemoveName(name);
	return AddData(name, type, data, numBytes, fixedSize);
}


const char*
BMessage::_TypeCodeToString(type_code type) const
{
	static char buffer[8];

	switch (type) {
		case B_BOOL_TYPE:		return "bool";
		case B_INT8_TYPE:		return "int8";
		case B_INT16_TYPE:		return "int16";
		case B_INT32_TYPE:		return "int32";
		case B_INT64_TYPE:		return "int64";
		case B_UINT8_TYPE:		return "uint8";
		case B_UINT16_TYPE:		return "uint16";
		case B_UINT32_TYPE:		return "uint32";
		case B_UINT64_TYPE:		return "uint64";
		case B_FLOAT_TYPE:		return "float";
		case B_DOUBLE_TYPE:		return "double";
		case B_STRING_TYPE:		return "string";
		case B_POINT_TYPE:		return "point";
		case B_RECT_TYPE:		return "rect";
		case B_SIZE_TYPE:		return "size";
		case B_RGB_32_BIT_TYPE:	return "color";
		case B_POINTER_TYPE:	return "pointer";
		case B_MESSAGE_TYPE:	return "message";
		case B_REF_TYPE:		return "ref";
		case B_NODE_REF_TYPE:	return "node_ref";
		default:
			snprintf(buffer, sizeof(buffer), "'%.4s'", (char*)&type);
			return buffer;
	}
}


void
BMessage::PrintToStream() const
{
	PrintToStream(false);
}


void
BMessage::PrintToStream(bool showValues) const
{
	_PrintToStream("", showValues);
}


void
BMessage::_PrintToStream(const char* indent, bool showValues) const
{
	uint32_t value = what;
	printf("%sBMessage(what = ", indent);
	if (isprint(((char*)&value)[0]) && isprint(((char*)&value)[1])
		&& isprint(((char*)&value)[2]) && isprint(((char*)&value)[3]))
		printf("'%.4s'", (char*)&value);
	else
		printf("0x%08x", what);
	printf(") {\n");

	if (fHeader == NULL || fFields == NULL || fData == NULL) {
		printf("%s  <empty>\n", indent);
		printf("%s}\n", indent);
		return;
	}

	field_header* field = fFields;
	for (uint32_t i = 0; i < fHeader->field_count; i++, field++) {
		const char* name = (const char*)(fData + field->offset);

		if (!showValues) {
			printf("%s  %-30s  %-10s  count=%u\n",
				indent, name, _TypeCodeToString(field->type), field->count);
			continue;
		}

		uint8_t* pointer = fData + field->offset + field->name_length;
		bool isFixed = (field->flags & FIELD_FLAG_FIXED_SIZE) != 0;

		for (uint32_t j = 0; j < field->count; j++) {
			size_t size;
			if (isFixed) {
				size = field->data_size / field->count;
			} else {
				size = *(uint32_t*)pointer;
				pointer += sizeof(uint32_t);
			}

			if (field->count == 1) {
				printf("%s  %s = ", indent, name);
			} else {
				printf("%s  %s[%u] = ", indent, name, j);
			}

			switch (field->type) {
				case B_BOOL_TYPE:
					printf("bool(%s)\n",
						*(bool*)pointer ? "true" : "false");
					break;

				case B_INT8_TYPE:
					printf("int8(%d or 0x%02x)\n",
						*(int8_t*)pointer, *(uint8_t*)pointer);
					break;

				case B_INT16_TYPE:
					printf("int16(%d or 0x%04x)\n",
						*(int16_t*)pointer, *(uint16_t*)pointer);
					break;

				case B_INT32_TYPE:
					printf("int32(%d or 0x%08x)\n",
						*(int32_t*)pointer, *(uint32_t*)pointer);
					break;

				case B_INT64_TYPE:
					printf("int64(%lld or 0x%016llx)\n",
						(long long)*(int64_t*)pointer,
						(unsigned long long)*(uint64_t*)pointer);
					break;

				case B_UINT8_TYPE:
					printf("uint8(%u or 0x%02x)\n",
						*(uint8_t*)pointer, *(uint8_t*)pointer);
					break;

				case B_UINT16_TYPE:
					printf("uint16(%u or 0x%04x)\n",
						*(uint16_t*)pointer, *(uint16_t*)pointer);
					break;

				case B_UINT32_TYPE:
					printf("uint32(%u or 0x%08x)\n",
						*(uint32_t*)pointer, *(uint32_t*)pointer);
					break;

				case B_UINT64_TYPE:
					printf("uint64(%llu or 0x%016llx)\n",
						(unsigned long long)*(uint64_t*)pointer,
						(unsigned long long)*(uint64_t*)pointer);
					break;

				case B_FLOAT_TYPE:
					printf("float(%.6f)\n", *(float*)pointer);
					break;

				case B_DOUBLE_TYPE:
					printf("double(%.10f)\n", *(double*)pointer);
					break;

				case B_STRING_TYPE:
					printf("string(\"%s\", %zu bytes)\n",
						(char*)pointer, size);
					break;

				case B_POINT_TYPE: {
					BPoint* pt = (BPoint*)pointer;
					printf("BPoint(x:%.2f, y:%.2f)\n", pt->x, pt->y);
					break;
				}

				case B_RECT_TYPE: {
					BRect* rect = (BRect*)pointer;
					printf("BRect(l:%.2f, t:%.2f, r:%.2f, b:%.2f)\n",
						rect->left, rect->top, rect->right, rect->bottom);
					break;
				}

				case B_SIZE_TYPE: {
					BSize* sz = (BSize*)pointer;
					printf("BSize(w:%.2f, h:%.2f)\n", sz->width, sz->height);
					break;
				}

				case B_RGB_32_BIT_TYPE: {
					rgb_color* col = (rgb_color*)pointer;
					printf("rgb_color(r:%u, g:%u, b:%u, a:%u)\n",
						col->red, col->green, col->blue, col->alpha);
					break;
				}

				case B_POINTER_TYPE:
					printf("pointer(%p)\n", *(void**)pointer);
					break;

				case B_MESSAGE_TYPE: {
					printf("BMessage(%zu bytes) ", size);

					BMessage nested;
					status_t result = nested.Unflatten((const char*)pointer,
						size);

					if (result == B_OK) {
						char nestedIndent[256];
						snprintf(nestedIndent, sizeof(nestedIndent),
							"%s    ", indent);

						printf("\n");
						nested._PrintToStream(nestedIndent, showValues);
					} else {
						printf("<unflatten failed: %d>\n", (int)result);
					}
					break;
				}

				default:
					printf("%s(type='%.4s', %zu bytes)\n",
						_TypeCodeToString(field->type),
						(char*)&field->type, size);
					break;
			}

			pointer += size;
		}
	}

	printf("%s}\n", indent);
}

}
