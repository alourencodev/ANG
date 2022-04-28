#pragma once

#include <Core/Attributes.hpp>
#include <Core/Meta.hpp>
#include <Core/Log/Assert.hpp>
#include <Core/Log/Log.h>
#include <Core/Math/Math.hpp>


namespace age
{

template<typename t_type>
class DefaultHeapAllocator
{
	constexpr static char k_tag[] = "DefaultHeapAllocator";

public:
	static t_type *alloc(size_t count) { return reinterpret_cast<t_type *>(std::malloc(sizeof(t_type) * count)); }

	static void dealloc(t_type *ptr) { std::free(ptr); }

	static bool realloc(t_type **outPtr, size_t count) 
	{ 
		if (count == 0) {
			// This is explicitly done since we don't know what the current realloc implementation does.
			dealloc(*outPtr);
			*outPtr = nullptr;
		} 
		else {
			t_type *newPtr = reinterpret_cast<t_type *>(std::realloc(*outPtr, sizeof(t_type) * count)); 
			if (newPtr == nullptr) {
				age_warning(k_tag, "Failed to reallocate memory with size %d", count);
				return false;
			}

			*outPtr = newPtr;
		}

		return true;
	}
};

namespace
{

class Header
{
public:
	Header *next = nullptr;
	Header *prev = nullptr;

	_force_inline const u64 size() const { u64 size = _size & ~1; return size; }

	_force_inline bool isSet() const { return (_size & 1) > 0; }

	_force_inline u64 *footer()
	{ 
		byte *ptr = reinterpret_cast<byte *>(this);
		return reinterpret_cast<u64 *>(ptr + size() - sizeof(u64));
	}

	_force_inline void *ptr()
	{ 
		return reinterpret_cast<void *>(this + sizeof(Header));
	}

	_force_inline void setSize(const u64 size) 
	{ 
		// This overwrites the used flag
		_size = size; 
		*footer() = _size;
	}
	
	_force_inline void set() 
	{ 
		_size |= 1; 
		*footer() = _size;
	}

	_force_inline void clear()
	{ 
		_size &= ~1; 
		*footer() = _size;
	}

private:
	u64 _size = 0;	// This will always be a multiplier of a power of 2, so we can always use the lower bit as a used flag.
};

}	// namespace anon

// TODO:	Possible memory optimization. For an allocated chunk it is useless to have a previous and next link.
//			Thus, we might save 16 bytes per allocated chunk if we dont use it.
template<class t_tag>
class HeapAllocator
{
	constexpr static char k_tag[] = "HeapAllocator";

	// Align to 64 bits
	constexpr static u64 k_alignmentBytes = 8;	// Every allocated chunk will be aligned to these amount of bytes
	constexpr static u64 k_metaDataSize = sizeof(Header) + sizeof(u64);	// Header + Footer. Footer is just a size
	
public:
	static void preAllocate(u64 bytes)
	{
		age_assertFatal(_inst.start == nullptr, "Allocator has already been pre allocated.");
		age_assertFatal(bytes > 0, "Unable to pre allocate allocator with 0 bytes.");

		_inst.start = _aligned_malloc(bytes, k_alignmentBytes);
		_inst.end = reinterpret_cast<byte *>(_inst.start) + bytes;
		_inst.firstFree = reinterpret_cast<Header *>(_inst.start);
		_inst.firstFree->setSize(bytes);
		_inst.firstFree->next = nullptr;
		_inst.firstFree->prev = nullptr;
	}

	static void freePreAllocatedMemory()
	{
		_aligned_free(_inst.start);
	}
	
	template<typename t_type>
	_nodiscard static t_type* alloc(u64 bytes)
	{
		// Includes header and footer 
		const u64 necessarySize = k_metaDataSize + bytes;			
		
		// The size that is actually being allocated. It will be >= than necessarySize
		const u64 allocationSize = calcAlignedChunkSizeForBytes(necessarySize);	

		// Find next fit
		Header *current = _inst.firstFree;
		while (current->size() < necessarySize) {
			if (current->next == nullptr) {
				age_error(k_tag, "Heap ran out of memory.");
				return nullptr;
			}

			current = current->next;
		}

		const u64 currentSize = current->size();

		age_assertFatal(necessarySize <= currentSize, "Necessary size should be greater or equal than the current size. This might mean an error in size calculation");

		// Check if chunk is larger than what we need.
		// If that is the case, then split.
		if (allocationSize < currentSize) {
			byte *currentBytePtr = reinterpret_cast<byte *>(current);
			Header *newChunk = reinterpret_cast<Header *>(currentBytePtr + allocationSize);
			newChunk->setSize(currentSize - allocationSize);
			newChunk->next = current->next;
			newChunk->prev = current->prev;

			if (current->next != nullptr)
				current->next->prev = newChunk;
			
			if (current->prev != nullptr)
				current->prev->next = newChunk;
			else
				_inst.firstFree = newChunk; 

		} else {
			if (current->next != nullptr)
				current->next->prev = current->prev;

			if (current->prev != nullptr)
				current->prev->next = current->next;
			else
				_inst.firstFree = current->next;
		}

		current->setSize(allocationSize);
		current->set();

		return reinterpret_cast<t_type *>(current->ptr());
	}

	template<typename t_type>
	static void free(t_type *&ptr)
	{
		Header *current = getHeader(ptr);
		bool mergedWithNext = false;

		{	// Check next 
			Header *next = reinterpret_cast<Header *>(current->footer() + 1);
			if (next < _inst.end) {
				if (!next->isSet()) {
					// Link to next
					current->next = next->next;
					if (next->next != nullptr)
						next->next->prev = current;

					// Link to previous
					current->prev = next->prev;
					if (next->prev != nullptr)
						next->prev->next = current;

					current->setSize(current->size() + next->size());

					mergedWithNext = true;
				}

				// Let's not clear the header here already, let's do that after we check the previous chunk

			} else {
				// Current is the last chunk in the heap
				current->next = nullptr;
			}
		}

		{	// Check previous
			if (current != _inst.start) {
				const byte *prevFooter = reinterpret_cast<byte *>(current) - sizeof(u64);
				const u64 prevFooterValue = *reinterpret_cast<const u64 *>(prevFooter);
				const bool isPreviousSet = (prevFooterValue & 1) > 0;

				if (!isPreviousSet) {
					byte *prevPtr = reinterpret_cast<byte *>(current) - prevFooterValue;
					Header *prev = reinterpret_cast<Header *>(prevPtr);

					if (mergedWithNext) {
						// We have already taken the position of the next one in the list,
						// so we need to remove prev from the list before merging with it

						if (prev->prev != nullptr)
							prev->prev->next = prev->next;
						else
							_inst.firstFree = prev->next;

						if (prev->next != nullptr)
							prev->next->prev = prev->prev;
					}

					// TODO: We are calculating the size multiple times so we can possibly improve on that

					// Merge
					prev->setSize(prev->size() + current->size());
					prev->next = current->next;
					prev->prev = current->prev;
					current = prev;

					// Update links
					if (current->prev != nullptr)
						current->prev->next = current;
					
					if (current->next != nullptr)
						current->next->prev = current;

				} else {
					current->clear();
				}

			} else {
				// If current is the first chunk in the heap
				current->prev = nullptr;
				current->next = _inst.firstFree;
				_inst.firstFree->prev = current;
			}
		}

		ptr = nullptr;
	}

private:
	static HeapAllocator<t_tag> _inst;

	static _force_inline u64 calcAlignedChunkSizeForBytes(u64 bytes)
	{
		// It is expected that bytes also include the size of metadata

		// Number of bits to shift when rounding to the alignment bytes
		// E.g. to align for 8 (1000) we want to shift 3 bits
		constexpr u8 k_roundShiftAmount = (k_alignmentBytes / 2) - 1;	

		bytes += k_alignmentBytes;
		bytes = bytes >> k_roundShiftAmount;
		bytes = bytes << k_roundShiftAmount;

		return bytes;
	}

	template<typename t_type>
	static _force_inline Header *getHeader(t_type *ptr) 
	{ 
		byte *bytePtr = reinterpret_cast<byte *>(ptr);
		return reinterpret_cast<Header *>(bytePtr - sizeof(Header)); 
	}

	void *start = nullptr;
	void *end = nullptr;
	Header *firstFree = nullptr;
};


#define DECLARE_HEAP_ALLOCATOR(Name)					\
struct Name ## _allocatorTag {};						\
using Name = age::HeapAllocator<Name ## _allocatorTag>

#define IMPLEMENT_HEAP_ALLOCATOR(Name)					\
Name Name ## ::_inst = Name()

}	// namespace age
