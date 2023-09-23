// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "WString.hpp"

namespace Plasma
{

wchar_t WString::mEmptyReturn(L'\0');

WString::WString(const wchar_t* str) : mSize(0)
{
  if (!str)
    return;
  // wcslen does not include the null terminating character in its count
  mSize = wcslen(str) + 1;
  wchar_t* wstr = (wchar_t*)plAllocate(mSize * sizeof(wchar_t));
  wcscpy(wstr, str);
  mData.SetData((::byte*)wstr, mSize * sizeof(wchar_t), true);
}

WString::WString(const wchar_t* str, size_t lengthInWChars) :
    // +1 for the null terminator (unlike String, WString actually adds to the
    // size for null)
    mSize(lengthInWChars + 1)
{
  if (!str)
  {
    mSize = 0;
    return;
  }

  wchar_t* wstr = (wchar_t*)plAllocate(mSize * sizeof(wchar_t));
  memcpy(wstr, str, lengthInWChars * sizeof(wchar_t));
  wstr[lengthInWChars] = 0;
  mData.SetData((::byte*)wstr, mSize * sizeof(wchar_t), true);
}

WString::WString(StringParam str) : mSize(0)
{
  if (str.Empty())
    return;
  // first read the number of wide characters we need to allocate for our
  // wide character buffer going from UTF8 encoded chars -> UTF16 (windows
  // wchar) this step is necessary when using MultiByteToWideChar
  mSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
  wchar_t* wstr = (wchar_t*)plAllocate(mSize * sizeof(wchar_t));
  // using the acquired information needed allocate a destination buffer and
  // covert the utf8 encoded character string to a wide string
  MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, mSize);
  mData.SetData((::byte*)wstr, mSize * sizeof(wchar_t), true);
}

WString::WString(const WString& rhs)
{
  InternalDeepCopy(rhs);
}

WString::WString() : mSize(0)
{
}

WString::~WString()
{
}

WString& WString::operator=(const WString& rhs)
{
  // Deal with self assignment
  if (&rhs == this)
    return *this;

  if (mSize > 0)
    mData.Deallocate();

  InternalDeepCopy(rhs);

  return *this;
}

const wchar_t* WString::c_str() const
{
  if (mData.Size())
    return (wchar_t*)mData.GetBegin();
  return &mEmptyReturn;
}

size_t WString::Size() const
{
  return mSize;
}

size_t WString::SizeInBytes() const
{
  return mSize * sizeof(wchar_t);
}

::byte* WString::Data()
{
  return mData.GetBegin();
}

bool WString::IsEmpty() const
{
  if (mSize)
    return false;
  return true;
}

void WString::InternalDeepCopy(const WString& rhs)
{
  mSize = rhs.mSize;

  Status status;
  size_t sizeInBytes = rhs.SizeInBytes();
  ::byte* data = (::byte*)plAllocate(sizeInBytes);
  rhs.mData.Read(status, data, sizeInBytes);

  mData.SetData(data, sizeInBytes, true);
}

String Narrow(const wchar_t* wstr)
{
  // first read the number of wide characters we need to allocate for our
  // character buffer going from UTF16 (windows wchar) -> UTF8 encoded chars
  // this step is necessary when using WideCharToMultiByte
  size_t size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
  char* str = new char[size];
  // using the acquired information needed allocate a destination buffer and
  // covert the wide string to a utf8 encoded character string
  WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, size, NULL, NULL);
  String ret(str);
  delete[] str;
  return ret;
}

String Narrow(const WString& wstr)
{
  if (wstr.IsEmpty())
    return String();

  return Narrow(wstr.c_str());
}

WString Widen(const char* str)
{
  if (!str)
    return WString();
  // first read the number of wide characters we need to allocate for our
  // wide character buffer going from UTF8 encoded chars -> UTF16 (windows
  // wchar) this step is necessary when using MultiByteToWideChar
  size_t size = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
  wchar_t* wstr = new wchar_t[size];
  // using the acquired information needed allocate a destination buffer and
  // covert the utf8 encoded character string to a wide string
  MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, size);
  WString ret(wstr);
  delete[] wstr;
  return ret;
}

WString Widen(const String& str)
{
  return WString(str);
}

int Utf16ToUtf8(int utf16)
{
  // standard ascii character, no need to convert, just return it.
  if (utf16 < 128)
    return utf16;

  // take the straight code point value and put it in a wchar
  wchar_t inputUTF16 = (wchar_t)utf16;

  size_t size = WideCharToMultiByte(CP_UTF8, 0, &inputUTF16, 1, NULL, 0, NULL, NULL);

  // output buffer is exact required size, no null terminator processed
  char outputUTF8[4] = {0};
  WideCharToMultiByte(CP_UTF8, 0, &inputUTF16, 1, outputUTF8, size, NULL, NULL);

  // after converting to UTF8 pack the code points back into an int
  int key = 0;
  for (size_t i = 0; i < size; ++i)
  {
    key <<= 8;
    key += (uchar)outputUTF8[i];
  }

  return key;
}

} // namespace Plasma
