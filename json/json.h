/*
 * json.h
 *
 * Copyright (c) 2016 Foghorn Systems, Inc.  All rights reserved.
 *
 */

#pragma once

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <iomanip>
#include <istream>
#include <map>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

// A JSON instance, which will be exactly one of:
//    - null:    nothing;
//    - array:   an array of other JSON instances;
//    - boolean: either true or false;
//    - number:  a double-precision float;
//    - object:  a map of strings to other JSON instances; or
//    - string:  a string of bytes.
//
// Each kind of JSON instance is represented by a member of the json_t::kind_t
// enumeration, which are null, array, boolean, number, object, and string.
// Calling get_kind() on a JSON instance will return one of these enumerated
// values.
//
// Each kind of JSON instance also has a type which provides it's specific
// behaviors.  Each of these types is named after its corresponding enumerated
// value, with the "_t" ending appended.  The types are therefore null_t,
// array_t, boolean_t, number_t, object_t, and string_t.  These are called the
// elementary types of json_t.
//
// The json_t class uses a union internally to combine the behaviors of all
// these elementary types.  The union allows the class to completely avoid heap
// allocations for JSON instances of a fixed size (null, boolean, and number).
// For dynamically sized JSON instances (string, array, and object), the
// union reduces the number of heap operations to just those required to
// maintain the indirect contents.
//
// Calling as() on a JSON instance gives you access to its elementary type,
// but you must know what kind of JSON instance it is.  If you call as() with
// the wrong elementary type, the function will throw.  Calling get_kind()
// before as() can help you to pick the right elementary type.  For example:
//
//    void some_function(std::ostream &strm, const json_t &json) {
//      if (json.get_kind() == json_t::string) {
//        const auto &string = json.as<json_t::string_>();
//        strm << "the string is " << string << std::endl;
//      } else if (json.get_kind() == json_t::number) {
//        const auto &number = json.as<json_t::number_>();
//        strm << "the number is " << number << std::endl;
//      } else {
//        strm << "the JSON object isn't a string or a number" << std::endl;
//      }
//    }
//
// Calling try_as() is like calling as(), except that it will return a pointer
// instead of a reference and, if the elementary type isn't the one you were
// expecting, the pointer returned will be null.  For example:
//
//    void some_function(std::ostream &strm, const json_t &json) {
//      const auto *string = json.try_as<json_t::string_t>();
//      if (string) {
//        strm << "the string is " << *string << std::endl;
//        return;
//      }
//      const auto *number = json.try_as<json_t::number_t>();
//        strm << "the number is " << *number << std::endl;
//        return;
//      }
//      strm << "the JSON object isn't a string or a number" << std::endl;
//    }
//
// This class supports encoding to and decoding from JSON wire format, as
// specified in RFC-7159 (available here: https://tools.ietf.org/html/rfc7159).
// If you insert an instance of this class onto a stream or extract one from
// a stream, the formatting will be as per the RFC.
class json_t final {
public:

  // The generic version of this template remains intentionally undefined.
  // It is specialized only on types from which json_t may construct.
  // These types include:
  //    - the basic element types (null_t, array_t, boolean_t, number_t,
  //      string_t, and object_t), each of which constructs its own kind of
  //      JSON instance;
  //    - the native integer types (signed and unsigned, 8- thru 64-bits),
  //      each of which constructs a JSON instance which is a number;
  //    - the native float type, which constructs a JSON instance which is
  //      a number; and
  //    - the c-string type (const char *), which constructs a JSON instance
  //      which is a string.
  template <typename arg_t>
  struct lookup_t;

  // The base class for errors thrown by functions of json_t.  It is
  // templatized so that it may inherit whichever standard C++ exception class
  // is appropriate for its purpose.
  template <typename super_t>
  class error_t;

  // Inherits from error_t<std::runtime_error>.  This is the error thrown by
  // functions which read from or write to streams, indicating the stream
  // contains a problem.  Most commonly, this will be caused by trying to read
  // badly formatted JSON.
  class stream_error_t;

  // Inherits from error_t<std::logic_error>.  This is the error thrown by
  // accessor and updater functions when they cannot comply with a request.
  // For example, attempting to subscript into a non-array JSON instance will
  // throw this sort of error.
  class usage_error_t;

  // An RAII class to preserve the IO flags in a standard stream.  The flags
  // are copied out by the constructor and then restored by the destructor.
  // This makes it safe to apply the various stream-modifying functions of
  // <iomanip> without worrying about altering the behavior of the stream non-
  // locally.
  class io_flags_saver_t;

  // The native type used to represent the null JSON instance.
  struct null_t {};

  // The native types used to represent the non-null kinds of JSON instances.
  // The union combines all of these, as well as null_t, into a single storage
  // area.
  using array_t   = std::vector<json_t>;
  using boolean_t = bool;
  using number_t  = double;
  using object_t  = std::map<std::string, json_t>;
  using string_t  = std::string;

  // Describes the kind of data stored in a JSON instance.  See get_kind()
  // and get_static_kind().
  enum kind_t { null, array, boolean, number, object, string };

  // Constructs an empty JSON instance of the given kind.  This is particularly
  // useful for constructing an empty object which you can then populate.
  // For example:
  //    json_t a = json_t::object;
  //    a["id"]   = 1001;
  //    a["name"] = "alice";
  json_t(kind_t new_kind = null) noexcept;

  // Move-construct, leaving the donor null.
  json_t(json_t &&that) noexcept;

  // Copy-construct.
  json_t(const json_t &that);

  // Destroy.
  ~json_t();

  // Construct from any type for which there exists a specialization of
  // lookup_t.  See lookup_t for a list of valid types.  Note that this
  // constructor takes its argument by value.  This is intentional.  You have
  // the option of forwarding or copying your argument into place at the
  // application site.  The constructor will then forward the argument into
  // the new object's internal storage.
  template <
      typename arg_t,
      typename = typename lookup_t<std::decay_t<arg_t>>::elem_t>
  json_t(arg_t arg) noexcept;

  // Construct an array from a braced list of JSON instances.  This is useful
  // for whipping up short arrays quickly.  For example:
  //    json_t a = { 101, "foo", true };
  //
  // For more complicated arrays, consider constructing the array_t instance
  // yourself, then forwarding it to the constructor above.  For example:
  //    json_t::array_t temp;
  //    for (int i = 0; i < 100; ++i) {
  //      temp.push_back(i);
  //    }
  //    json_t a = std::move(temp);
  json_t(std::initializer_list<json_t> that);

  // Move-assign, leaving the donor null.
  json_t &operator=(json_t &&that) noexcept;

  // Copy-assign.
  json_t &operator=(const json_t &that);

  // Subscript into an array.  The access is range-checked and will throw
  // if out of bounds.  Also throws if this JSON instance is not an array.
  // See also try_lookup().
  json_t &operator[](size_t idx);

  // As operator[], above, but as a constant.  See also try_lookup().
  const json_t &operator[](size_t idx) const;

  // Looks up the value associated with a key.  If the key is not in the
  // object, it is created with a null value, which is returned.  Throws if
  // this JSON instance is not an object.
  json_t &operator[](const string_t &key);

  // Looks up the value associated with a key.  If the key is not in the
  // object, this throws.  Also throws if this JSON instance is not an object.
  const json_t &operator[](const string_t &key) const;

  // The same as compare(that) == 0, q.v.
  bool operator==(const json_t &that) const noexcept;

  // The same as compare(that) != 0, q.v.
  bool operator!=(const json_t &that) const noexcept;

  // The same as compare(that) < 0, q.v.
  bool operator<(const json_t &that) const noexcept;

  // The same as compare(that) <= 0, q.v.
  bool operator<=(const json_t &that) const noexcept;

  // The same as compare(that) > 0, q.v.
  bool operator>(const json_t &that) const noexcept;

  // The same as compare(that) >= 0, q.v.
  bool operator>=(const json_t &that) const noexcept;

  // Like try_as(), q.v., except that this will never return a null pointer.
  // If try_as() would return a null pointer, this throws instead.
  template <typename elem_t>
  elem_t &as();

  // Like try_as(), q.v., except that this will never return a null pointer.
  // If try_as() would return a null pointer, this throws instead.
  template <typename elem_t>
  const elem_t &as() const;

  // Compares this JSON instance with that one and returns:
  //    <  0: this instance sorts before that one;
  //    >  0: this instance sorts after that one; or
  //    == 0: this instance and that one are identical.
  //
  // NB: DO NOT assume the non-zero return values are exactly -1 or 1.  They
  // can be anything.
  //
  // Ordering is based first on kind, such that:
  //    null < array < boolean < number < object < string.
  //
  // Between instances of the same kind, the comparision method is based
  // determined by kind:
  //    null:    any two nulls are identical;
  //    array:   lexicographic;
  //    boolean: false orders before true;
  //    number:  by increasing value;
  //    object:  lexicographic by key, then by value; or
  //    string:  lexicographic.
  // single-valued
  int compare(const json_t &that) const noexcept;

  // True iff. this object contains the given key.  Throws if this JSON
  // instance is not an object.
  bool contains(const string_t &key) const;

  // Convert this JSON instance to its wire format and return the resulting
  // bytes as a string.  This is equivalent to streaming the object out and
  // capturing the result as a string.  See operator>>, below.
  std::string encode() const;

  // Return a copy of the contents of this JSON instance, converted to the
  // requested return type.  The return type must be among the types for
  // which a specialization of lookup_t exists.  If the requested conversion
  // is incompatible with the JSON instance's kind, this throws.
  template <typename ret_t>
  ret_t get() const;

  // As get(), above, but with the return type deduced and returned via
  // out-parameter.
  template <typename ret_t>
  void get(ret_t &ret) const;

  // The kind of JSON instance this is.  See kind_t, above.
  kind_t get_kind() const noexcept;

  // The meaning of this accessor depends on the kind of JSON instance this
  // is:
  //    null:    throws;
  //    array:   the number of elements in the array;
  //    boolean: throws;
  //    number:  throws;
  //    object:  the number of key-value pairs in the object; or
  //    string:  the number of bytes in the string.
  size_t get_size() const;

  // The meaning of this accessor depends on the kind of JSON instance this
  // is:
  //    null:    throws;
  //    array:   true iff. the array contains no elements;
  //    boolean: throws;
  //    number:  throws;
  //    object:  true iff. the object contains no key-value pairs; or
  //    string:  true iff. the string contains no bytes.
  bool is_empty() const;

  // Return this object the default-constructed state appropriate for the
  // given kind.
  json_t &reset(kind_t new_kind = null) noexcept;

  // Similar to get(), q.v., except that the value is moved out instead of
  // copied out, so this JSON instance is left null.  The return type must be
  // among the types for which a specialization of lookup_t exists.  If the
  // requested conversion is incompatible with the JSON instance's kind, this
  // throws (and leaves this JSON instance intact).
  template <typename ret_t>
  ret_t take() noexcept;

  // As take(), above, but with the return type deduced and returned via
  // out-parameter.
  template <typename ret_t>
  void take(ret_t &ret) noexcept;

  // If this JSON instance contains an element of the given type, return a
  // pointer to it; otherwise, return a null pointer.  The type must be one of
  // of the six element types (null_t, array_t, boolean_t, number_t, object_t,
  // or string_t).
  // NB: This function is meant to provide access to the contents of the JSON
  // instance as they are actually stored.  It CANNOT cast the contents to any
  // other type.  For that functionality, use get() or take(), qq.v.
  template <typename elem_t>
  elem_t *try_as() noexcept;

  // As try_as(), above, but as a constant.
  template <typename elem_t>
  const elem_t *try_as() const noexcept;

  // Send back, via out-parameter, a copy of the contents of this JSON
  // instance, converted to the requested return type, and return true.  If the
  // requested conversion is incompatible with the JSON instance's kind, leave
  // the out-parameter unchanged and return false.  The return type must be
  // among the types for which a specialization of lookup_t exists.
  template <typename ret_t>
  bool try_get(ret_t &ret) const;

  // Look up the value associated with a key and return a pointer to it.  If
  // the key is not in the object, return a null pointer.  Throw if this JSON
  // instance is not an object.
  json_t *try_lookup(const string_t &key);

  // As try_lookup(), above, but as a constant.
  const json_t *try_lookup(const string_t &key) const;

  // Similar to try_get(), q.v., except that the value is moved out instead of
  // copied out, so this JSON instance is left null.  If this function returns
  // false, then the JSON instance remains intact.
  template <typename ret_t>
  bool try_take(ret_t &ret) noexcept;

  // Extract a new JSON instance from the given stream by decoding the JSON
  // wire format.  See also operator<<, below.
  static json_t decode(std::istream &strm);

  // Extract a new JSON instance from the given string by decoding the JSON
  // wire format.  See also operator<<, below.
  static json_t decode(const std::string str);

  // The enumerated value which describes the given type.  The type must be one
  // of the six element types (null_t, array_t, boolean_t, number_t, object_t,
  // or string_t).
  template <typename elem_t>
  static constexpr kind_t get_static_kind() noexcept;

  // Insert a JSON instance onto an output stream by encoding the instance in
  // JSON wire format.
  friend std::istream &operator>>(std::istream &strm, json_t &that);

  // Extract a JSON instance from an input stream by decoding the JSON wire
  // format.
  friend std::ostream &operator<<(std::ostream &strm, const json_t &that);

private:

  // The type of pair stored by object_t.  Used by compare_range_items().
  using field_t = object_t::value_type;

  // Construct in our union storage space an instance of elem_t using the
  // given args_t.  This is called by the various constructors.  It ignores
  // and overwrites any previous state.
  template <typename elem_t, typename... args_t>
  void construct(args_t &&... args);

  // If the given pointer is non-null, this returns it as a reference;
  // otherwise, this throws an error like 'expected <x> but found <y>'.  This
  // is used by as() to check the result of try_as().
  template <typename elem_t>
  elem_t &throw_if_null(elem_t *elem) const;

  // Compares two instances of array_t or object_t lexicographically.  Uses
  // compare_range_items() to compare individual elements of the collections.
  // Used by compare().
  template <typename range_t>
  static int compare_ranges(const range_t &lhs, const range_t &rhs) noexcept;

  // A pass-thru to compare().  Used by compare_ranges() when comparing two
  // instances of array_t.
  static bool compare_range_items(
      const json_t &lhs, const json_t &rhs) noexcept;

  // Compares to fields, first by their keys and then by their values.  Used
  // by compare_ranges() when comparing two instances of object_t.
  static bool compare_range_items(
      const field_t &lhs, const field_t &rhs) noexcept;

  // Compare two values for which operator< is defined.  This is used by
  // compare() when comparing booleans or numbers.
  template <typename value_t>
  static int compare_values(const value_t &lhs, const value_t &rhs) noexcept;

  // Skips whitespace, then compares the next non-whitespace character to
  // the expected character.  If they match, this advances the stream past
  // the expected character and returns.  If they don't match, this leaves
  // the stream positioned at the unexpected character and throws.
  static void match(std::istream &strm, int expected);

  // Exctract one or more digits from the input stream and insert them on
  // the output stream.  Stop at the first non-digit.  There must be at least
  // one digit or this throws.
  static void pump_digits(std::ostream &out, std::istream &in);

  // Extracts an array from a stream.  This must start with '[' and end with
  // ']' and contain a comma-separated list of JSON instances.  This skips
  // leading whitespace and any irrelevant internal whitespace.  If the syntax
  // is not as expected, this throws.
  static array_t read_array(std::istream &strm);

  // Extracts a number from a stream.  This follows the JSON standard for
  // a number rather than that given by the locale imbued in the stream.
  // This skips leading whitespace and any irrelevant internal whitespace.  If
  // the syntax is not as expected, this throws.
  static number_t read_number(std::istream &strm);

  // Extracts an array from a stream.  This must start with '{' and end with
  // '}' and contain a comma-separated list of pairs.  Each pair must consist
  // of a JSON string and a JSON instance, separated by ':'.  This skips
  // leading whitespace and any irrelevant internal whitespace.  If the syntax
  // is not as expected, this throws.
  static object_t read_object(std::istream &strm);

  // Extracts a number from a stream.  This follows the JSON standard for
  // a string rather than that given by the locale imbued in the stream.  In
  // particular, this must start and end with '"' and contain only whitespace,
  // printable characters, or JSON-compliant escape sequences.  This skips
  // leading whitespace and any irrelevant internal whitespace.  If the syntax
  // is not as expected, this throws.
  static string_t read_string(std::istream &strm);

  // Extracts a contiguous series of letters from a stream.  (There are no
  // quotes involved.)  This skips leading whitespace and stops at the next
  // occurrence of whitespace or any other non-letter.  If the syntax is not as
  // expected, this throws.
  static std::string read_word(std::istream &strm);

  // Skips whitespace, then compares the next non-whitespace character to
  // the expected character.  If they match, this advances the stream past
  // the expected character and returns true.  If they don't match, this leaves
  // the stream positioned at the unexpected character and returns false.
  static bool try_match(std::istream &strm, int expected);

  // Insert the word "null" on a stream.
  static void write(std::ostream &strm, const null_t &that);

  // Insert an array on a stream such that it can be extracted by
  // read_array(), q.v.
  static void write(std::ostream &strm, const array_t &that);

  // Insert the word "true" or the word "false" on a stream.
  static void write(std::ostream &strm, const boolean_t &that);

  // Insert an number on a stream such that it can be extracted by
  // read_number(), q.v.
  static void write(std::ostream &strm, const number_t &that);

  // Insert an object on a stream such that it can be extracted by
  // read_object(), q.v.
  static void write(std::ostream &strm, const object_t &that);

  // Insert a string on a stream such that it can be extracted by
  // read_string(), q.v.
  static void write(std::ostream &strm, const string_t &that);

  // Insert a character code onto a stream such that it will be easily
  // understood by a human.  If the character is printable, this inserts it
  // as a single-quoted example (like 'x').  If the character is the eof marker
  // (-1), this inserts 'eof'.  If the character is one of the common escape
  // codes, this inserts it as a single-quoted escape code (like '\n').
  // Otherwise, this inserts the character as hex number with leading base
  // marker (like 0xff).
  static void write_char(std::ostream &strm, int c);

  // The union of our possible elements.
  union {
    null_t    null_elem;
    array_t   array_elem;
    boolean_t boolean_elem;
    number_t  number_elem;
    object_t  object_elem;
    string_t  string_elem;
  };

  // Determines which memeber of the union has actually been constructed.
  kind_t kind;

};  // json_t

// See the forward declaration in json_t.
template <typename super_t>
class json_t::error_t
    : public super_t {
public:

  // A function we can call to stream out a description of what went wrong.
  using cb_t = std::function<void (std::ostream &)>;

  // Takes ownership of the callback closure.
  explicit error_t(cb_t cb);

  // Overridden to construct the error message by calling write() followed
  // by the cached callback.  The resulting string is itself cached and any
  // subsequent call to what() will return the cached value.
  virtual const char *what() const noexcept;

protected:

  // Override to provide a general description of the kind of error this is.
  virtual void write(std::ostream &msg) const = 0;

private:

  // Set at construction time, then called the first time what() is called.
  // After what() uses this callback, it sets the callback null, releasing
  // any resources it was holding.
  mutable cb_t cb;

  // Initially empty, this is set by the first call to what().  Thereafter,
  // this contains our cached message.
  mutable std::string msg;

};  // json_t::error_t

// See the forward declaration in json_t.
class json_t::stream_error_t
    : public error_t<std::runtime_error> {
public:

  // Caches the position within the stream at which the error occurred
  // and takes ownership of the callback closure.
  stream_error_t(std::streampos pos, cb_t cb);

  // The position within the stream at which the error occurred.
  std::streampos get_pos() const noexcept;

private:

  // Overridden to indicate that this is a stream-related error and the
  // position at which the error occurred.
  virtual void write(std::ostream &msg) const override;

  // See accessor.
  std::streampos pos;

};  // json_t::stream_error_t

// See the forward declaration in json_t.
class json_t::usage_error_t
    : public error_t<std::logic_error> {
public:

  // Use the constructor from the base class.
  using error_t::error_t;

private:

  // Overridden to indicate that this is a usage-related error.
  virtual void write(std::ostream &msg) const override;

};  // json_t::usage_error_t

// See the forward declaration in json_t.
class json_t::io_flags_saver_t final {
public:

  // Caches the io flags from the stream.
  explicit io_flags_saver_t(std::ios_base &strm);

  // Restores the cached io flags to the stream.
  ~io_flags_saver_t();

  // No copying.
  io_flags_saver_t(const io_flags_saver_t &) = delete;
  io_flags_saver_t &operator=(const io_flags_saver_t &) = delete;

private:

  // The stream to which we will restore flags.
  std::ios_base &strm;

  // The flags we will restore.
  std::ios::fmtflags saved_flags;

};  // json_t::io_flags_saver_t

// Inserts a json_t::kind_t enumerated value in a human-readable form.
std::ostream &operator<<(std::ostream &strm, json_t::kind_t that);

///////////////////////////////////////////////////////////////////////////////

// Definitions for basic JSON element types.
#define LOOKUP(elem)                              \
    template <>                                   \
    struct json_t::lookup_t<json_t::elem##_t> {   \
      using elem_t = json_t::elem##_t;            \
      static constexpr auto kind = elem;          \
    };
LOOKUP(null   )
LOOKUP(array  )
LOOKUP(boolean)
LOOKUP(number )
LOOKUP(object )
LOOKUP(string )
#undef LOOKUP

// Definitions for types which can convert to/from JSON numbers.
#define LOOKUP(ret)                 \
    template <>                     \
    struct json_t::lookup_t<ret> {  \
      using elem_t = number_t;      \
    };
LOOKUP(int8_t  )
LOOKUP(int16_t )
LOOKUP(int32_t )
LOOKUP(int64_t )
LOOKUP(uint8_t )
LOOKUP(uint16_t)
LOOKUP(uint32_t)
LOOKUP(uint64_t)
LOOKUP(float   )
#undef LOOKUP

// Definition to convert from a c-string to a JSON string.
template <>
struct json_t::lookup_t<const char *> {
  using elem_t = string_t;
};

///////////////////////////////////////////////////////////////////////////////

template <typename super_t>
json_t::error_t<super_t>::
    error_t(cb_t cb_)
    : super_t({}), cb(std::move(cb_)) {}

template <typename super_t>
const char *
    json_t::error_t<super_t>::
    what() const noexcept {
  // If the callback is non-null, we haven't cached the message yet.
  if (cb) {
    // Write the message.
    std::ostringstream strm;
    write(strm);
    strm << "; ";
    cb(strm);
    // Cache the message and forget the callback.
    msg = strm.str();
    cb = cb_t {};
  }
  // Return the cached message.
  return msg.c_str();
}

///////////////////////////////////////////////////////////////////////////////

inline json_t::stream_error_t::
    stream_error_t(std::streampos pos_, cb_t cb)
    : error_t(std::move(cb)), pos(pos_) {}

inline std::streampos
    json_t::stream_error_t::
    get_pos() const noexcept {
  return pos;
}

inline void
    json_t::stream_error_t::
    write(std::ostream &msg) const {
  msg << "JSON parsing error at position " << pos;
}

///////////////////////////////////////////////////////////////////////////////

inline void
    json_t::usage_error_t::
    write(std::ostream &msg) const {
  msg << "JSON usage error";
}

///////////////////////////////////////////////////////////////////////////////

inline json_t::io_flags_saver_t::
    io_flags_saver_t(std::ios_base &strm_)
    : strm(strm_), saved_flags(strm_.flags()) {}

inline json_t::io_flags_saver_t::
    ~io_flags_saver_t() {
  strm.flags(saved_flags);
}

///////////////////////////////////////////////////////////////////////////////

inline json_t::
    json_t(kind_t new_kind) noexcept {
  switch (new_kind) {
    case null:    construct<null_t   >(); break;
    case array:   construct<array_t  >(); break;
    case boolean: construct<boolean_t>(); break;
    case number:  construct<number_t >(); break;
    case object:  construct<object_t >(); break;
    case string:  construct<string_t >(); break;
  }  // switch
}

inline json_t::
    json_t(json_t &&that) noexcept {
  switch (that.kind) {
    case null:    construct<null_t   >(std::move(that.null_elem   )); break;
    case array:   construct<array_t  >(std::move(that.array_elem  )); break;
    case boolean: construct<boolean_t>(std::move(that.boolean_elem)); break;
    case number:  construct<number_t >(std::move(that.number_elem )); break;
    case object:  construct<object_t >(std::move(that.object_elem )); break;
    case string:  construct<string_t >(std::move(that.string_elem )); break;
  }  // switch
  that.reset();
}

inline json_t::
    json_t(const json_t &that) {
  switch (that.kind) {
    case null:    construct<null_t   >(that.null_elem   ); break;
    case array:   construct<array_t  >(that.array_elem  ); break;
    case boolean: construct<boolean_t>(that.boolean_elem); break;
    case number:  construct<number_t >(that.number_elem ); break;
    case object:  construct<object_t >(that.object_elem ); break;
    case string:  construct<string_t >(that.string_elem ); break;
  }  // switch
}

inline json_t::
    ~json_t() {
  switch (kind) {
    case null:    null_elem   .~null_t   (); break;
    case array:   array_elem  .~array_t  (); break;
    case boolean: boolean_elem.~boolean_t(); break;
    case number:  number_elem .~number_t (); break;
    case object:  object_elem .~object_t (); break;
    case string:  string_elem .~string_t (); break;
  }  // kind
}

template <typename arg_t, typename>
json_t::
    json_t(arg_t arg) noexcept {
  using elem_t = typename lookup_t<std::decay_t<arg_t>>::elem_t;
  construct<elem_t>(std::forward<arg_t>(arg));
}

inline json_t::
    json_t(std::initializer_list<json_t> that) {
  construct<array_t>(that);
}

inline json_t &
    json_t::
    operator=(json_t &&that) noexcept {
  if (this != &that) {
    this->~json_t();
    new (this) json_t(std::move(that));
  }
  return *this;
}

inline json_t &
    json_t::
    operator=(const json_t &that) {
  return *this = json_t { that };
}

inline json_t &
    json_t::
    operator[](size_t idx) {
  switch (kind) {
    case array: {
      if (idx >= array_elem.size()) {
        throw usage_error_t {
          [idx, size = array_elem.size()](std::ostream &msg) {
            msg
                << "cannot access element " << idx
                << " in array of size " << size;
          }
        };
      }
      return array_elem[idx];
    }
    case null: case boolean: case number: case object: case string: {
      throw usage_error_t {
        [kind = this->kind](std::ostream &msg) {
          msg << "cannot subscript into " << kind;
        }
      };
    }
  }  // switch
}

inline const json_t &
    json_t::
    operator[](size_t idx) const {
  switch (kind) {
    case array: {
      return array_elem[idx];
    }
    case null: case boolean: case number: case object: case string: {
      throw usage_error_t {
        [kind = this->kind](std::ostream &msg) {
          msg << "cannot subscript into constant " << kind;
        }
      };
    }
  }  // switch
}

inline json_t &
    json_t::
    operator[](const string_t &key) {
  switch (kind) {
    case object: {
      return object_elem[key];
    }
    case null: case array: case boolean: case number: case string: {
      throw usage_error_t {
        [key, kind = this->kind](std::ostream &msg) {
          msg << "cannot lookup key " << std::quoted(key) << " in " << kind;
        }
      };
    }
  }  // switch
}

inline const json_t &
    json_t::
    operator[](const string_t &key) const {
  switch (kind) {
    case object: {
      auto iter = object_elem.find(key);
      if (iter == object_elem.end()) {
        throw usage_error_t {
          [key](std::ostream &msg) {
            msg << "object does not contain key " << std::quoted(key);
          }
        };
      }
      return iter->second;
    }
    case null: case array: case boolean: case number: case string: {
      throw usage_error_t {
        [key, kind = this->kind](std::ostream &msg) {
          msg
              << "cannot lookup key " << std::quoted(key)
              << " in constant " << kind;
        }
      };
    }
  }  // switch
}

inline bool
    json_t::
    operator==(const json_t &that) const noexcept {
  return compare(that) == 0;
}

inline bool
    json_t::
    operator!=(const json_t &that) const noexcept {
  return compare(that) != 0;
}

inline bool
    json_t::
    operator<(const json_t &that) const noexcept {
  return compare(that) < 0;
}

inline bool
    json_t::
    operator<=(const json_t &that) const noexcept {
  return compare(that) <= 0;
}

inline bool
    json_t::
    operator>(const json_t &that) const noexcept {
  return compare(that) > 0;
}

inline bool
    json_t::
    operator>=(const json_t &that) const noexcept {
  return compare(that) >= 0;
}

template <typename elem_t>
elem_t &
    json_t::
    as() {
  return throw_if_null(try_as<elem_t>());
}

template <typename elem_t>
const elem_t &
    json_t::
    as() const {
  return throw_if_null(try_as<elem_t>());
}

inline int
    json_t::
    compare(const json_t &that) const noexcept {
  int result = kind - that.kind;
  if (!result) {
    switch (kind) {
      case null:    result = 0; break;
      case array:   result = compare_ranges(array_elem,   that.array_elem  ); break;
      case boolean: result = compare_values(boolean_elem, that.boolean_elem); break;
      case number:  result = compare_values(number_elem,  that.number_elem ); break;
      case object:  result = compare_ranges(object_elem,  that.object_elem ); break;
      case string:  result = string_elem.compare(that.string_elem);           break;
    }  // switch
  }
  return result;
}

inline bool
    json_t::
    contains(const string_t &key) const {
  switch (kind) {
    case object: {
      return object_elem.find(key) != object_elem.end();
    }
    case null: case array: case boolean: case number: case string: {
      throw usage_error_t {
        [key, kind = this->kind](std::ostream &msg) {
          msg
              << "cannot test for presence of key " << std::quoted(key)
              << " in " << kind;
        }
      };
    }
  }  // switch
}

inline std::string
    json_t::
    encode() const {
  std::ostringstream strm;
  strm << *this;
  return strm.str();
}

template <typename ret_t>
ret_t
    json_t::
    get() const {
  using elem_t = typename lookup_t<std::decay_t<ret_t>>::elem_t;
  return static_cast<ret_t>(as<elem_t>());
}

template <typename ret_t>
void
    json_t::
    get(ret_t &ret) const {
  ret = get<ret_t>();
}

inline json_t::kind_t
    json_t::
    get_kind() const noexcept {
  return kind;
}

inline size_t
    json_t::
    get_size() const {
  switch (kind) {
    case array:  return array_elem .size();
    case object: return object_elem.size();
    case string: return string_elem.size();
    case null: case boolean: case number: {
      throw usage_error_t {
        [kind = this->kind](std::ostream &msg) {
          msg << "cannot get size of " << kind;
        }
      };
    }
  }  // switch
}

inline bool
    json_t::
    is_empty() const {
  switch (kind) {
    case array:  return array_elem .empty();
    case object: return object_elem.empty();
    case string: return string_elem.empty();
    case null: case boolean: case number: {
      throw usage_error_t {
        [kind = this->kind](std::ostream &msg) {
          msg << "cannot test for emptiness of " << kind;
        }
      };
    }
  }  // switch
}

inline json_t &
    json_t::
    reset(kind_t new_kind) noexcept {
  this->~json_t();
  new (this) json_t(new_kind);
  return *this;
}

template <typename ret_t>
ret_t
    json_t::
    take() noexcept {
  using elem_t = typename lookup_t<std::decay_t<ret_t>>::elem_t;
  auto ret = static_cast<ret_t>(std::forward<elem_t>(as<elem_t>()));
  reset();
  return ret;
}

template <typename ret_t>
void
    json_t::
    take(ret_t &ret) noexcept {
  ret = take<ret_t>();
}

template <typename elem_t>
elem_t *
    json_t::
    try_as() noexcept {
  return (kind == lookup_t<elem_t>::kind)
      ? reinterpret_cast<elem_t *>(&null_elem)
      : nullptr;
}

template <typename elem_t>
const elem_t *
    json_t::
    try_as() const noexcept {
  return (kind == lookup_t<elem_t>::kind)
      ? reinterpret_cast<const elem_t *>(&null_elem)
      : nullptr;
}

template <typename ret_t>
bool
    json_t::
    try_get(ret_t &ret) const {
  using elem_t = typename lookup_t<std::decay_t<ret_t>>::elem_t;
  const auto *elem = try_as<elem_t>();
  bool success = (elem != nullptr);
  if (success) {
    ret = static_cast<ret_t>(*elem);
  }
  return success;
}

inline json_t *
    json_t::
    try_lookup(const string_t &key) {
  switch (kind) {
    case object: {
      auto iter = object_elem.find(key);
      return (iter != object_elem.end()) ? &(iter->second) : nullptr;
    }
    case null: case array: case boolean: case number: case string: {
      throw usage_error_t {
        [key, kind = this->kind](std::ostream &msg) {
          msg << "cannot lookup key " << std::quoted(key) << " in " << kind;
        }
      };
    }
  }  // switch
}

inline const json_t *
    json_t::
    try_lookup(const string_t &key) const {
  switch (kind) {
    case object: {
      auto iter = object_elem.find(key);
      return (iter != object_elem.end()) ? &(iter->second) : nullptr;
    }
    case null: case array: case boolean: case number: case string: {
      throw usage_error_t {
        [key, kind = this->kind](std::ostream &msg) {
          msg
              << "cannot lookup key " << std::quoted(key)
              << " in constant " << kind;
        }
      };
    }
  }  // switch
}

template <typename ret_t>
bool
    json_t::
    try_take(ret_t &ret) noexcept {
  using elem_t = typename lookup_t<std::decay_t<ret_t>>::elem_t;
  auto *elem = try_as<elem_t>();
  bool success = (elem != nullptr);
  if (success) {
    ret = static_cast<ret_t>(std::forward<elem_t>(*elem));
    reset();
  }
  return success;
}

inline json_t
    json_t::
    decode(std::istream &strm) {
  json_t result;
  strm >> result;
  return result;
}

inline json_t
    json_t::
    decode(const std::string str) {
  std::istringstream strm { str };
  json_t result;
  strm >> result;
  return result;
}

template <typename elem_t>
constexpr json_t::kind_t
    json_t::
    get_static_kind() noexcept {
  return lookup_t<std::decay_t<elem_t>>::kind;
}

template <typename elem_t, typename... args_t>
void
    json_t::
    construct(args_t &&... args) {
  new (&null_elem) elem_t(std::forward<args_t>(args)...);
  kind = get_static_kind<elem_t>();
}

template <typename elem_t>
elem_t &
    json_t::
    throw_if_null(elem_t *elem) const {
  if (!elem) {
    throw usage_error_t {
      [expected = get_static_kind<elem_t>(), actual = this->kind]
      (std::ostream &msg) {
        msg << "expected " << expected << " but found " << actual;
      }
    };
  }
  return *elem;
}

template <typename range_t>
int
    json_t::
    compare_ranges(const range_t &lhs, const range_t &rhs) noexcept {
  // Iterate through both ranges in parallel.
  auto lhs_iter = std::begin(lhs);
  auto lhs_end  = std::end(lhs);
  auto rhs_iter = std::begin(rhs);
  auto rhs_end  = std::end(rhs);
  for (;;) {
    // If we've reached the ends of both ranges simultaneously, the ranges
    // are equal.
    if (lhs_iter == lhs_end && rhs_iter == rhs_end) {
      return 0;
    }
    // If we've reached the end of the lhs range but not that of the rhs range,
    // the lhs sorts before the rhs.
    if (lhs_iter == lhs_end) {
      return -1;
    }
    // If we've reached the end of the rhs range but not that of the lhs range,
    // the lhs sorts after the rhs.
    if (rhs_iter == rhs_end) {
      return 1;
    }
    // We have not reached the end of either range, so compare the values at
    // the current position.  If they are non-equal, return the comparison.
    int result = compare_range_items(*lhs_iter, *rhs_iter);
    if (!result) {
      return result;
    }
    // Advance both iterators.
    ++lhs_iter;
    ++rhs_iter;
  }  // for
}

inline bool
    json_t::
    compare_range_items(
    const json_t &lhs, const json_t &rhs) noexcept {
  return lhs.compare(rhs);
}

inline bool
    json_t::
    compare_range_items(
    const field_t &lhs, const field_t &rhs) noexcept {
  int result = lhs.first.compare(rhs.first);
  if (!result) {
    result = lhs.second.compare(rhs.second);
  }
  return result;
}

template <typename value_t>
int
    json_t::
    compare_values(const value_t &lhs, const value_t &rhs) noexcept {
  return (lhs < rhs) ? -1 : (rhs < lhs) ? 1 : 0;
}

inline void
    json_t::
    match(std::istream &strm, int expected) {
  if (!try_match(strm, expected)) {
    throw json_t::stream_error_t {
      strm.tellg(),
      [expected, c = strm.peek()](std::ostream &msg) {
        msg << "expected ";
        write_char(msg, expected);
        msg << "; found ";
        write_char(msg, c);
      }
    };
  }
}

inline void
    json_t::
    pump_digits(std::ostream &out, std::istream &in) {
  int c = in.peek();
  if (!isdigit(c)) {
    throw json_t::stream_error_t {
      in.tellg(),
      [c](std::ostream &msg) {
        msg << "expected digit; found ";
        write_char(msg, c);
      }
    };
  }
  do {
    in.ignore();
    out.put(static_cast<char>(c));
    c = in.peek();
  } while (isdigit(c));
}

inline json_t::array_t
    json_t::
    read_array(std::istream &strm) {
  array_t result;
  match(strm, '[');
  if (!try_match(strm, ']')) {
    do {
      result.emplace_back(decode(strm));
    } while (try_match(strm, ','));
    match(strm, ']');
  }
  return result;
}

inline json_t::number_t
    json_t::
    read_number(std::istream &strm) {
  std::ostringstream accum;
  int c = std::ws(strm).peek();
  if (c == '-') {
    strm.ignore();
    accum.put('-');
    c = strm.peek();
  }
  if (c == '0') {
    strm.ignore();
    accum.put('0');
  } else {
    pump_digits(accum, strm);
  }
  c = strm.peek();
  if (c == '.') {
    strm.ignore();
    accum.put(static_cast<char>(c));
    pump_digits(accum, strm);
  }
  c = strm.peek();
  if (c == 'e' || c == 'E') {
    strm.ignore();
    accum.put(static_cast<char>(c));
    c = strm.peek();
    if (c == '+' || c == '-') {
      strm.ignore();
      accum.put(static_cast<char>(c));
      c = strm.peek();
    }
    pump_digits(accum, strm);
  }
  return std::stod(accum.str());
}

inline json_t::object_t
    json_t::
    read_object(std::istream &strm) {
  object_t result;
  match(strm, '{');
  if (!try_match(strm, '}')) {
    do {
      auto key = read_string(strm);
      match(strm, ':');
      result.emplace(std::move(key), decode(strm));
    } while (try_match(strm, ','));
    match(strm, '}');
  }
  return result;
}

inline json_t::string_t
    json_t::
    read_string(std::istream &strm) {
  match(strm, '"');
  std::ostringstream accum;
  for (;;) {
    int c = strm.peek();
    if (c == '"') {
      strm.ignore();
      break;
    }
    if (c == '\\') {
      strm.ignore();
      c = strm.peek();
      switch (c) {
        case 'b': strm.ignore(); c = '\b'; break;
        case 'f': strm.ignore(); c = '\f'; break;
        case 'n': strm.ignore(); c = '\n'; break;
        case 'r': strm.ignore(); c = '\r'; break;
        case 't': strm.ignore(); c = '\t'; break;
        case '"': case '\\': case '/': {
          strm.ignore();
          break;
        }
        case 'u': {
          strm.ignore();
          char digits[5];
          for (size_t i = 0; i < 4; ++i) {
            c = strm.peek();
            if (!isxdigit(c)) {
              throw json_t::stream_error_t {
                strm.tellg(),
                [c](std::ostream &msg) {
                  msg << "expected hex digit; found ";
                  write_char(msg, c);
                }
              };
            }
          }  // for
          digits[4] = '\0';
          c = static_cast<int>(strtol(digits, nullptr, 16));
          break;
        }
        default: {
          throw stream_error_t {
            strm.tellg(),
            [c](std::ostream &msg) {
              msg << "illegal escape character ";
              write_char(msg, c);
              msg << " in quoted string";
            }
          };
        }
      }  // switch
    } else if (c < 0) {
      throw json_t::stream_error_t {
        strm.tellg(),
        [](std::ostream &msg) {
          msg << "eof in quoted string";
        }
      };
    } else if (!isprint(c)) {
      throw json_t::stream_error_t {
        strm.tellg(),
        [c](std::ostream &msg) {
          msg << "unescaped control character ";
          write_char(msg, c);
          msg << " in quoted string";
        }
      };
    } else {
      strm.ignore();
    }
    accum.put(static_cast<char>(c));
  }  // for
  return accum.str();
}

inline std::string
    json_t::
    read_word(std::istream &strm) {
  int c = std::ws(strm).peek();
  if (!isalpha(c)) {
    throw json_t::stream_error_t {
      strm.tellg(),
      [c](std::ostream &msg) {
        msg << "expected letter; found ";
        write_char(msg, c);
      }
    };
  }
  std::ostringstream accum;
  do {
    strm.ignore();
    accum.put(static_cast<char>(c));
    c = strm.peek();
  } while (isalpha(c));
  return accum.str();
}

inline bool
    json_t::
    try_match(std::istream &strm, int expected) {
  int c = std::ws(strm).peek();
  bool success = (c == expected);
  if (success) {
    strm.ignore();
  }
  return success;
}

inline void
    json_t::
    write(std::ostream &strm, const null_t &/*that*/) {
  strm << "null";
}

inline void
    json_t::
    write(std::ostream &strm, const array_t &that) {
  bool first = true;
  for (const auto &item: that) {
    strm << (first ? "[ " : ", ") << item;
    first = false;
  }  // for
  strm << (first ? "[]" : " ]");
}

inline void
    json_t::
    write(std::ostream &strm, const boolean_t &that) {
  strm << (that ? "true" : "false");
}

inline void
    json_t::
    write(std::ostream &strm, const number_t &that) {
  strm << that;
}

inline void
    json_t::
    write(std::ostream &strm, const object_t &that) {
  bool first = true;
  for (const auto &item: that) {
    strm << (first ? "{ " : ", ");
    write(strm, item.first);
    strm << ": " << item.second;
    first = false;
  }  // for
  strm << (first ? "{}" : " }");
}

inline void
    json_t::
    write(std::ostream &strm, const string_t &that) {
  strm << '"';
  for (char c: that) {
    switch (c) {
      case '"':  strm << "\\\""; break;
      case '\\': strm << "\\\\"; break;
      case '\b': strm << "\\b";  break;
      case '\f': strm << "\\f";  break;
      case '\n': strm << "\\n";  break;
      case '\r': strm << "\\r";  break;
      case '\t': strm << "\\t";  break;
      default: {
        if (isprint(c)) {
          strm.put(static_cast<char>(c));
        } else {
          uint16_t code = reinterpret_cast<uint8_t &>(c);
          io_flags_saver_t io_flags_saver { strm };
          strm
              << "\\u"
              << std::setfill('0') << std::hex << std::setw(4) << code;
        }
      }
    }  // switch
  }  // for
  strm << '"';
}

inline void
    json_t::
    write_char(std::ostream &strm, int c) {
  switch (c) {
    case '\\': strm << "\\\\"; break;
    case '\'': strm << "\\'";  break;
    case '\b': strm << "\\b";  break;
    case '\f': strm << "\\f";  break;
    case '\n': strm << "\\n";  break;
    case '\r': strm << "\\r";  break;
    case '\t': strm << "\\t";  break;
    default: {
      if (isprint(c)) {
        strm << '\'' << static_cast<char>(c) << '\'';
      } else if (c < 0) {
        strm << "eof";
      } else {
        io_flags_saver_t io_flags_saver { strm };
        strm
            << std::showbase << std::internal << std::setfill('0')
            << std::hex << std::setw(4) << c;
      }
    }
  }  // switch
}

///////////////////////////////////////////////////////////////////////////////

inline std::istream &operator>>(std::istream &strm, json_t &that) {
  // Peek at the next non-whitespace byte to see what kind of JSON instance
  // is encoded.
  int c = std::ws(strm).peek();
  if (c == '[') {
    // This is an array.
    that = json_t::read_array(strm);
  } else if (c == '{') {
    // This is an object.
    that = json_t::read_object(strm);
  } else if (c == '"') {
    // This is a string.
    that = json_t::read_string(strm);
  } else if (c == '-' || isdigit(c)) {
    // This is an number.
    that = json_t::read_number(strm);
  } else if (isalpha(c)) {
    // This is keyword of some kind.
    auto pos = strm.tellg();
    auto word = json_t::read_word(strm);
    if (word == "null") {
      that = json_t::null;
    } else if (word == "true") {
      that = true;
    } else if (word == "false") {
      that = false;
    } else {
      throw json_t::stream_error_t {
        pos,
        [word](std::ostream &msg) {
          msg
              << "expected \"null\", \"true\", or \"false\"; found "
              << std::quoted(word);
        }
      };
    }
  } else {
    throw json_t::stream_error_t {
      strm.tellg(),
      [c](std::ostream &msg) {
        msg << "expected '[', '{', '\"', '-', digit, or letter; found ";
        json_t::write_char(msg, c);
      }
    };
  }
  return strm;
}

inline std::ostream &operator<<(std::ostream &strm, const json_t &that) {
  switch (that.kind) {
    case json_t::null:    json_t::write(strm, that.null_elem   ); break;
    case json_t::array:   json_t::write(strm, that.array_elem  ); break;
    case json_t::boolean: json_t::write(strm, that.boolean_elem); break;
    case json_t::number:  json_t::write(strm, that.number_elem ); break;
    case json_t::object:  json_t::write(strm, that.object_elem ); break;
    case json_t::string:  json_t::write(strm, that.string_elem ); break;
  }
  return strm;
}

inline std::ostream &operator<<(std::ostream &strm, json_t::kind_t that) {
  const char *text;
  switch (that) {
    case json_t::null:    text = "null";    break;
    case json_t::array:   text = "array";   break;
    case json_t::boolean: text = "boolean"; break;
    case json_t::number:  text = "number";  break;
    case json_t::object:  text = "object";  break;
    case json_t::string:  text = "string";  break;
  }  // switch
  return strm << text;
}