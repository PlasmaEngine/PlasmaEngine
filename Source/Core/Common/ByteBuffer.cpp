// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

ByteBuffer::BlockRange::BlockRange(ByteBuffer* buffer)
  : mCurrent()
{
  mBlock = buffer->mBlocks.Data();
  mLast = buffer->mBlocks.End();
  mBlockSize = buffer->mBlockSize;
  mLastBlockSize = buffer->mCurBlockSize;
}

const ByteBuffer::Block& ByteBuffer::BlockRange::Front()
{
  ErrorIf(Empty(), "Read empty range.");
  LoadBlock();
  return mCurrent;
}

bool ByteBuffer::BlockRange::Empty()
{
  return mBlock >= mLast;
}

void ByteBuffer::BlockRange::LoadBlock()
{
  mCurrent.Size = mBlock == (mLast - 1) ? mLastBlockSize : mBlockSize;
  mCurrent.Data = *mBlock;
}

void ByteBuffer::BlockRange::PopFront()
{
  ErrorIf(Empty(), "Popped and empty range.");
  ++mBlock;
}

ByteBuffer::ByteBuffer(size_t blockSize)
{
  mTotalSize = 0;
  mCurBlockSize = 0;
  mCurBlockBuffer = nullptr;
  mBlockSize = blockSize;
}

ByteBuffer::~ByteBuffer()
{
  Deallocate();
}

ByteBuffer::BlockRange ByteBuffer::Blocks()
{
  return BlockRange(this);
}

size_t ByteBuffer::Tell()
{
  return mTotalSize;
}

void ByteBuffer::Write(const byteType* data, size_t sizeInBytes)
{
  Append(data, sizeInBytes);
}

void ByteBuffer::Append(const byteType* data, size_t sizeInBytes)
{
  if (mCurBlockBuffer == nullptr)
  {
    // Allocator more memory
    mCurBlockBuffer = (byteType*)plAllocate(mBlockSize);
    mCurBlockSize = 0;

    // Store the block
    mBlocks.PushBack(mCurBlockBuffer);
  }

  while (mCurBlockSize + sizeInBytes > mBlockSize)
  {
    // Partial copy to remaining area of block
    size_t sizeToCopy = mBlockSize - mCurBlockSize;

    // Copy into data block
    memcpy(mCurBlockBuffer + mCurBlockSize, data, sizeToCopy);

    // Allocator more memory
    mCurBlockBuffer = (byteType*)plAllocate(mBlockSize);
    mCurBlockSize = 0;

    // Store the block
    mBlocks.PushBack(mCurBlockBuffer);

    // Decrement bytes copied and try again
    sizeInBytes -= sizeToCopy;
    mTotalSize += sizeToCopy;
    data += sizeToCopy;
  }

  // Is there any more data to be copied?
  if (sizeInBytes > 0)
  {
    memcpy(mCurBlockBuffer + mCurBlockSize, data, sizeInBytes);
    mCurBlockSize += sizeInBytes;
    mTotalSize += sizeInBytes;
  }
}

void ByteBuffer::Backup(size_t sizeInBytes)
{
  ErrorIf(sizeInBytes > mTotalSize, "Attempting to back-up more than the total size of the ByteBuffer");

  mTotalSize -= sizeInBytes;

  while (sizeInBytes > mCurBlockSize)
  {
    delete mBlocks.Back();
    mBlocks.PopBack();

    sizeInBytes -= mCurBlockSize;

    mCurBlockBuffer = mBlocks.Back();
    mCurBlockSize = mBlockSize;
  }

  mCurBlockSize -= sizeInBytes;
}

::byte ByteBuffer::operator[](size_t index) const
{
  return (*(ByteBuffer*)this)[index];
}

::byte& ByteBuffer::operator[](size_t index)
{
  ErrorIf(index > mTotalSize, "ByteBuffer index out of bounds");

  size_t outerIndex = index / mBlockSize;
  size_t innerIndex = index % mBlockSize;

  return mBlocks[outerIndex][innerIndex];
}

void ByteBuffer::ExtractInto(byteType* byteBuffer, size_t bufferSizeInBytes) const
{
  ErrorIf(mTotalSize > bufferSizeInBytes, "Buffer is not large enough for data.");
  if (mTotalSize > bufferSizeInBytes)
    return; // Do nothing

  // unreferenced formal parameter in release
  (void)bufferSizeInBytes;

  // Copy over all blocks
  byteType* bufferPosition = byteBuffer;
  Array<byteType*>::range blocks = mBlocks.All();
  for (; !blocks.Empty(); blocks.PopFront())
  {
    size_t blockSize = blocks.Front() == mCurBlockBuffer ? mCurBlockSize : mBlockSize;
    memcpy(bufferPosition, blocks.Front(), blockSize);
    bufferPosition += mBlockSize;
  }
}

void ByteBuffer::ExtractInto(ByteBufferBlock& buffer) const
{
  buffer.Deallocate();
  buffer.mSize = mTotalSize;
  buffer.mData = (::byte*)plAllocate(mTotalSize);
  buffer.mCurrent = buffer.mData;
  buffer.mOwnsData = true;
  ExtractInto(buffer.mData, buffer.mSize);
};

void ByteBuffer::Deallocate()
{
  // Deallocate all blocks
  Array<byteType*>::range blocks = mBlocks.All();
  for (; !blocks.Empty(); blocks.PopFront())
  {
    plDeallocate(blocks.Front());
  }

  mCurBlockSize = 0;
  mCurBlockBuffer = nullptr;
  mBlocks.Clear();
  mTotalSize = 0;
}

ByteBufferBlock::ByteBufferBlock()
{
  mData = nullptr;
  mCurrent = nullptr;
  mSize = 0;
  mOwnsData = false;
}

ByteBufferBlock::ByteBufferBlock(ByteBufferBlock&& rhs) :
    mData(rhs.mData),
    mSize(rhs.mSize),
    mCurrent(rhs.mCurrent),
    mOwnsData(rhs.mOwnsData)
{
  rhs.mData = nullptr;
  rhs.mSize = 0;
  rhs.mCurrent = nullptr;
  rhs.mOwnsData = false;
}

ByteBufferBlock::ByteBufferBlock(MoveReference<ByteBufferBlock> rhs) :
    mData(rhs->mData),
    mSize(rhs->mSize),
    mCurrent(rhs->mCurrent),
    mOwnsData(rhs->mOwnsData)
{
  rhs->mData = nullptr;
  rhs->mSize = 0;
  rhs->mCurrent = nullptr;
  rhs->mOwnsData = false;
}

ByteBufferBlock::ByteBufferBlock(const ByteBufferBlock& rhs) : mSize(rhs.mSize), mOwnsData(rhs.mOwnsData)
{
  if (mOwnsData)
  {
    mData = (::byte*)plAllocate(rhs.mSize);
    memcpy(mData, rhs.mData, mSize);
    mCurrent = mData + (rhs.mCurrent - rhs.mData);
  }
  else
  {
    mData = rhs.mData;
    mCurrent = rhs.mCurrent;
  }
}

ByteBufferBlock::ByteBufferBlock(size_t size)
{
  mData = (::byte*)plAllocate(size);
  mCurrent = mData;
  mSize = size;
  mOwnsData = true;
}

ByteBufferBlock::ByteBufferBlock(::byte* data, size_t size, bool owned)
{
  mData = data;
  mCurrent = data;
  mSize = size;
  mOwnsData = owned;
}

void ByteBufferBlock::SetData(::byte* data, size_t size, bool owned)
{
  Deallocate();
  mData = data;
  mCurrent = data;
  mSize = size;
  mOwnsData = owned;
}

void ByteBufferBlock::SetBlock(DataBlock block)
{
  Deallocate();
  mData = block.Data;
  mCurrent = block.Data;
  mSize = block.Size;
  mOwnsData = false;
}

DataBlock ByteBufferBlock::GetBlock()
{
  return DataBlock(mData, mSize);
}

const DataBlock ByteBufferBlock::GetBlock() const
{
  return DataBlock(mData, mSize);
}

ByteBufferBlock::~ByteBufferBlock()
{
  Deallocate();
}

void ByteBufferBlock::Deallocate()
{
  if (mData && mOwnsData)
    plDeallocate(mData);

  mData = nullptr;
  mCurrent = nullptr;
  mSize = 0;
  mOwnsData = false;
}

void ByteBufferBlock::Seek(int offset, uint /*origin*/)
{
  mCurrent += offset;
}

size_t ByteBufferBlock::Read(Status& status, ::byte* data, size_t sizeInBytes)
{
  ErrorIf(mCurrent + sizeInBytes > mData + mSize, "Buffer Overflow Read");
  memcpy(data, mCurrent, sizeInBytes);
  mCurrent += sizeInBytes;
  return sizeInBytes;
}

size_t ByteBufferBlock::Write(::byte* data, size_t sizeInBytes)
{
  ErrorIf(mCurrent + sizeInBytes > mData + mSize, "Buffer Overflow Write");
  memcpy(mCurrent, data, sizeInBytes);
  mCurrent += sizeInBytes;
  return sizeInBytes;
}

size_t ByteBufferBlock::Write(::byte value)
{
  return Write(&value, 1);
}

size_t ByteBufferBlock::Size() const
{
  return mSize;
}

::byte* ByteBufferBlock::GetCurrent()
{
  return mCurrent;
}

size_t ByteBufferBlock::Tell()
{
  return mCurrent - mData;
}

::byte* ByteBufferBlock::GetBegin()
{
  return mData;
}

String ByteBuffer::ToString() const
{
  size_t bufferSize = GetSize();

  // A string nodes already Contains (and sets) the last null terminator
  StringNode* node = String::AllocateNode(bufferSize);

  // Copy data into buffer
  ExtractInto((::byte*)node->Data, bufferSize);

  return String(node);
}

String ByteBuffer::ToString(size_t subStringSizeBytes) const
{
  size_t bufferSize = GetSize();

  // A string nodes already Contains (and sets) the last null terminator
  StringNode* node = String::AllocateNode(bufferSize, subStringSizeBytes);

  // Copy data into buffer
  ExtractInto((::byte*)node->Data, bufferSize);
  node->Data[subStringSizeBytes] = '\0';

  return String(node);
}

} // namespace Plasma
