// expansion macro for enum name definition
#define ENUM_NAME(name, assign, prettyName) name,

// expansion macro for enum value definition
#define ENUM_VALUE(name, assign, prettyName) name assign,

// expansion macro for enum to string conversion
#define ENUM_CASE(name, assign, prettyName) case name: return #prettyName;

// expansion macro for string to enum conversion
#define ENUM_STRCMP(name, assign, prettyName) if (!strcmp(str,#prettyName)) return name;

/// declare the access function and define enum values
#define DECLARE_ENUM(EnumType,ENUM_DEF) \
  typedef enum { \
    ENUM_DEF(ENUM_VALUE) \
  } EnumType; \
  const char *Get ## EnumType ## String(EnumType dummy); \
  EnumType Get ## EnumType ## Value(const char *string);

/// define the access function names
#define DEFINE_ENUM(EnumType,ENUM_DEF,enumType) \
  const char *Get ## EnumType ## String(EnumType value) \
  { \
    switch(value) \
    { \
      ENUM_DEF(ENUM_CASE) \
      default: return ""; /* handle input error */ \
    } \
  } \
  EnumType Get ## EnumType ## Value(const char *str) \
  { \
    ENUM_DEF(ENUM_STRCMP) \
    return (EnumType)0; /* handle input error */ \
  } \
  const EnumType EnumType ## Values[] = { ENUM_DEF(ENUM_NAME) }; \
  const enumType Total ## EnumType ## s = sizeof(EnumType ## Values) / sizeof((EnumType ## Values)[0]);
