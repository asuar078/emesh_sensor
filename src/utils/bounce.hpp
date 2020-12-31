//
// Created by bigbywolf on 12/27/20.
//

#ifndef EMESH_SENSOR_BOUNCE_HPP
#define EMESH_SENSOR_BOUNCE_HPP

/**
* This is our "bounce" function template.
* This function is intended to be passed to a C-style callback system which takes
* a function pointer and private data pointer as input values. Using C-style callbacks
* is problematic with object member functions, as you need the object's pointer.
*
* This bounce function requires use of the private data pointer for the object's
* instance pointer. Any other provided arguments are forwarded to the object's member
* function, and the return value is passed to the caller.
*/
template<class T, class Method, Method m, class ...Params>
static auto bounce(void *priv, Params... params) ->
decltype(((*reinterpret_cast<T *>(priv)).*m)(params...))
{
  return ((*reinterpret_cast<T *>(priv)).*m)(params...);
}

/**
* Convenience macro to simplify bounce statement usage
*/
#define BOUNCE(c,m) bounce<c, decltype(&c::m), &c::m>

#endif //EMESH_SENSOR_BOUNCE_HPP
