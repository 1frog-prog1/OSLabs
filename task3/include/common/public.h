#include <stdexcept>

#define TOSTRING(token) #token
#define ASSERT(condition) if (!(condition)) { throw std::runtime_error(std::string() + "Condition is false (" TOSTRING(condition) ") in " + __PRETTY_FUNCTION__ + " -- " + __FILE__ + ":" + std::to_string(__LINE__)); }
#define VERIFY(condition) if (!(condition)) { __builtin_trap(); }
