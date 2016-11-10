#ifndef _AVALUE_H
#define _AVALUE_H 1
/* Copyright 1995 Carnegie Mellon University All rights reserved. */

/** \addtogroup libbasics
 * @{ */
#include <andrewos.h>

#include <atom.H>
#include <oflex.H>

class avalueflex;
/** The types of values which may be stored in an avalue */
union avalue_u {
    long integer; /**< Long integers */
    double real; /**< Double floats */
    const char *cstr; /**< Strings */
    void *obj; /**< (Pointers to) Objects */
};

/** A container for a value and its type.
 *  This is intended for providing generic typeed arguments to functions.
 *  The value is one of those supported by avalue_u.  The type is a string,
 *  represented as an atom for easy comparison.  A name may also be
 *  associated with the value, in which case it is also a string, represented
 *  as an atom.  */
class avalue {
    avalue_u val;
    const atom *type;
    const atom *name;
  public:
    // these allow easy reference to the built in types, for example:
    // if(a.Type()==avalue::real) dosomethingwith(a.Real());
    static const atom *integer; /**< The type name "integer", for long integers.  */
    static const atom *real; /**< The type name "real", for double floating-proint values. */
    static const atom *cstring; /**< The type name "cstring", for standard C 0-terminated strings.  */
    static const atom *voidptr; /**< The type name "voidptr", for opaque data objects.  */
    static const atom *valueflex;  /**< The type name "valueflex", for an array of values.  */
    static const atom *none; /**< The name "none", for nameless values.  */
    static const atom *voidatom; /**< The type name "void", for no assigned value.  */
    static const atom *atomatom; /**< The type name "atom", for a string in the form of an atom object.  */
    static const atom *atkatom; /**< The type name "ATK", for an object derived from ATK.  */
    avalue(); /**< Default constructor.  The name and type are NULL, and the value is uninitialized.  */
    avalue(const avalue &src);
    /**< Copy constructor.
     *   \param src The avalue to copy. */
    avalue &operator=(const avalue &src);
    /**< Assignment copy operator.
     *   \param src The avalue to copy.
     *   \return the copied avalue. */
    ~avalue(); /**< Destructor.  The value is neither freed nor destroyed.  */
    // Accessor functions for the various types.  If the type given doesn't match the actual type 0 will be returned.  If no type is given the effect is as if the type indicated by the function name had been given.  The pointers returned are owned by the object which set the avalue.
    long Integer(const atom *type=NULL) const;
    /**< Obtain the integer value.
     *   \param type The type to check the value against, or avalue::integer if NULL.
     *   \return The integer value, or 0 if there is a type mismatch.  */
    double Real(const atom *type=NULL) const;
    /**< Obtain the floating-point value.
     *   \param type The type to check the value against, or avalue::real if NULL.
     *   \return The floating-point value, or 0.0 if there is a type mismatch.  */
    const char *CString(const atom *type=NULL) const;
    /**< Obtain the string value.
     *   \param type The type to check the value against, or avalue::cstring if NULL.
     *   \return The string pointer value, or NULL if there is a type mismatch.  */
    void *VoidPtr(const atom *type=NULL) const;
    /**< Obtain the generic pointer value.
     *   \param type The type to check the value against, or avalue::voidptr if NULL.
     *   \return The pointer value, or NULL if there is a type mismatch.  */
    avalueflex *ValueFlex(const atom *type=NULL) const;
    /**< Obtain the value array value.
     *   \param type The type to check the value against, or avalue::valueflex if NULL.
     *   \return The array value pointer, or NULL if there is a type mismatch.  */
    const atom *Atom(const atom *type=NULL) const;
    /**< Obtain the atom value.
     *   \param type The type to check the value against, or avalue::atomatom if NULL.
     *   \return The atom value pointer, or NULL if there is a type mismatch.  */
    ATK *ATKObject(const atom *type=NULL) const;
    /**< Obtain the ATK object value.
     *   \param type The type to check the value against, or avalue::atkatom if NULL.
     *   \return The ATK object value pointer, or NULL if there is a type mismatch.
     *           Note that in this case, the type is the name of any ATK class, and
     *           the type of the value can be that or any descendent, as defined by
     *           ATK::IsTypeByName().
     */

    // These allow avalue objects to be cast implicitly to the indicated types.  The result is 0 if the avalue is not of the proper type.
    inline operator long() const{
	return Integer();
    } /**< Return the integer value.  If the type is not avalue::integer, the value is 0.  */
    inline operator double() const {
	return Real();
    } /**< Return the floating-point value.  If the type is not avalue::real, the value is 0.0.  */
    inline operator const char *() const {
	return CString();
    } /**< Return the C string value.  If the type is not avalue::cstring, the value is NULL.  */
    inline operator void *() const {
	return VoidPtr();
    } /**< Return the generic pointer value.  If the type is not avalue::voidptr, the value is NULL.  */
    inline operator avalueflex *() const {
	return ValueFlex();
    } /**< Return the value array pointer value.  If the type is not avalue::valueflex, the value is NULL.  */
    inline operator const atom *() const {
	return Atom();
    } /**< Return the atom pointer value.  If the type is not avalue::atomatom, the value is NULL.  */
    inline const atom *Type() const {
	return type;
    } /**< Return the currently assigned type for this value.
       *   \return the type atom. */
    inline const atom *Name() const {
	return name;
    } /**< Return the currently assigned name for this value.
       *   \return the name atom. */

    // Sets the avalue to the indicated value, name and type.
    // Pointers are owned by the calling code.
    void Set(long integer,const atom *name=NULL, const atom *type=NULL);
    /**< Set integer value, with given name and type.
     *   \param integer The value
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::integer. */
    void Set(double real, const atom *name=NULL, const atom *type=NULL);
    /**< Set floating-point value, with given name and type.
     *   \param real The value
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::real. */
    void Set(const char *str, const atom *name=NULL, const atom *type=NULL);
    /**< Set string value, with given name and type.
     *   \param str The value.  Its lifetime must match the avalue lifetime.
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::cstring. */
    void Set(void *ptr, const atom *name=NULL, const atom *type=NULL);
    /**< Set generic pointer value, with given name and type.
     *   \param ptr The value.  Its lifetime must match the avalue lifetime.
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::voidptr. */
    void Set(avalueflex *alist, const atom *name=NULL, const atom *type=NULL);
    /**< Set avalueflex array pointer value, with given name and type.
     *   \param alist The value.  Its lifetime must match the avalue lifetime.
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::valueflex. */
    void Set(const atom *a, const atom *name=NULL, const atom *type=NULL);
    /**< Set atom pointer value, with given name and type.
     *   \param a The value.
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::atomatom. */
    void Set(ATK *obj, const atom *name=NULL);
    /**< Set ATK object pointer value, with given name.
     *   The type atom is set to the name of the type, given by ATK::GetTYpeName().
     *   \param obj The value.  Its lifetime must match the avalue lifetime.
     *   \param name The name.  If NULL, use avalue::none. */

    // These assignment operators allow the indicated types to be assigned to an avalue directly.
    inline avalue &operator=(long integer) {
	Set(integer);
	return *this;
    } /**< Create an integer avalue via assignment.
       *   \param integer The integer.
       *   \return the new avalue. */
     inline avalue &operator=(ATK *obj) {
	Set(obj);
	return *this;
    } /**< Create an ATK object pointer avalue via assignment.
       *   \param obj The ATK object pointer.
       *   \return the new avalue. */
    inline avalue &operator=(double real) {
	Set(real);
	return *this;
    } /**< Create a floating-point avalue via assignment.
       *   \param real The floating-point value.
       *   \return the new avalue. */
    inline avalue &operator=(char *str) {
	Set(str);
	return *this;
    }  /**< Create a C string pointer avalue via assignment.
       *   \param str The C string pointer.
       *   \return the new avalue. */
    inline avalue &operator=(void *ptr) {
	Set(ptr);
	return *this;
    } /**< Create a generic pointer avalue via assignment.
       *   \param ptr The generic pointer.
       *   \return the new avalue. */
    inline avalue &operator=(avalueflex *alist) {
	Set(alist);
	return *this;
    } /**< Create an avalueflex array pointer avalue via assignment.
       *   \param alist The avalueflex array pointer.
       *   \return the new avalue. */
    inline avalue &operator=(const atom *a) {
	Set(a);
	return *this;
    } /**< Create an atom pointer avalue via assignment.
       *   \param a The atom pointer.
       *   \return the new avalue. */

    // Sets the name of an avalue.
    void SetName(const atom *name);
    /**< Change the name of the value.
     *   \param name The new name.  */

    // These constructors allow the indicated type wherever a const avalue is expected, and may be called explicitly as well.  (Both cases are probably rare, usually the avalue objects will be managed by an avalueflex or class derived therefrom.
    inline avalue(long integer,const atom *name=NULL, const atom *type=NULL) {
	Set(integer, name, type);
    }
    /**< Constructor shortcut for creating integer avalue.
     *   \param integer The value
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::integer. */
    inline avalue(double real, const atom *name=NULL, const atom *type=NULL) {
	Set(real, name, type);
    }
    /**< Constructor shortcut for creating floating-point avalue.
     *   \param real The value
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::real. */
    inline avalue(const char *str, const atom *name=NULL, const atom *type=NULL) {
	Set(str, name, type);
    }
    /**< Constructor shortcut for creating C string pointer avalue.
     *   \param str The value.  Its lifetime must match the avalue lifetime.
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::cstring. */
    inline avalue(void *ptr, const atom *name=NULL, const atom *type=NULL) {
	Set(ptr, name, type);
    }
    /**< Constructor shortcut for creating generic pointer avalue.
     *   \param ptr The value.  Its lifetime must match the avalue lifetime.
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::voidptr. */
    inline avalue(avalueflex *alist, const atom *name=NULL, const atom *type=NULL){
	Set(alist, name, type);
    }
    /**< Constructor shortcut for creating avalueflex array pointer avalue.
     *   \param alist The value.  Its lifetime must match the avalue lifetime.
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::valueflex. */
    inline avalue(const atom *a, const atom *name=NULL, const atom *type=NULL) {
	Set(a, name, type);
    }
    /**< Constructor shortcut for creating atom pointer avalue.
     *   \param a The value.
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::atomatom. */
    inline avalue(ATK *a, const atom *name=NULL) {
	Set(a, name);
    }
    /**< Constructor shortcut for creating ATK object pointer avalue.
     *   The type atom is set to the name of the type, given by ATK::GetTYpeName().
     *   \param a The value.  Its lifetime must match the avalue lifetime.
     *   \param name The name.  If NULL, use avalue::none. */
};

/// The generic part of the avalueflex functionality.  Do not use directly.
DEFINE_OFLEX_CLASS(avalueflex_base,avalue,5U);

// Define the specialized parts of the avalueflex functionality.

/** The avalueflex is a flexible array supporting indexed access,
 *  insertion, and deletion of avalues.
 *  Several convenience features are added onto the standard flexible
 *  array for quick construction:
 *
 *   - a normal value can be (auto)cast into or assigned to an avalueflex
 *     containing only that value as a single unnamed element.
 *
 *   - unnamed elements may be appended using the | operator: `avalueflex()|2|"hello";`.
 *
 *   - optionally named elements may be appended using add:  `avalueflex().add(2,"N").add("hello","greeting");`.
 *
 *   - explicit constructors can be used to create single-value arrays.
 */
class avalueflex : public avalueflex_base {
  protected:
    // These allow avalueflex objects to be quite cheap to create and throw away assuming they have fewer that 16 elements.
    char *Allocate(size_t len);
    
  public:
    // Constructors to allow building an avalueflex containing one value.
    // Only the first argument is required so where an avalueflex or
    // a reference to a const avalueflex are required any of long, double, char *
    // void *, avalueflex *, or atom * can be passed.
    avalueflex(long integer, const atom *name=NULL, const atom *type=NULL);
    /**< Constructor to create avalueflex array with only one integer value.
     *   It allows e.g. automatic creation of single-value arrays for
     *   parameter passing.
     *   \param integer The value
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::integer. */
    avalueflex(double real, const atom *name=NULL, const atom *type=NULL);
    /**< Constructor to create avalueflex array with only one floating-point value.
     *   It allows e.g. automatic creation of single-value arrays for
     *   parameter passing.
     *   \param real The value
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::real. */
    avalueflex(const char *str, const atom *name=NULL, const atom *type=NULL);
    /**< Constructor to create avalueflex array with only one C string value.
     *   It allows e.g. automatic creation of single-value arrays for
     *   parameter passing.
     *   \param str The value.  Its lifetime must match the avalue lifetime.
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::cstring. */
    avalueflex(void *ptr, const atom *name=NULL, const atom *type=NULL);
    /**< Constructor to create avalueflex array with only one generic pointer value.
     *   It allows e.g. automatic creation of single-value arrays for
     *   parameter passing.
     *   \param ptr The value.  Its lifetime must match the avalue lifetime.
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::voidptr. */
    avalueflex(avalueflex *avf, const atom *name=NULL, const atom *type=NULL);
    /**< Constructor to create avalueflex array with only one avalueflex array pointer value.
     *   \param avf The value.  Its lifetime must match the avalue lifetime.
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::valueflex. */
   avalueflex(const atom *a, const atom *name=NULL, const atom *type=NULL);
    /**< Constructor to create avalueflex array with only one atom pointer value.
     *   It allows e.g. automatic creation of single-value arrays for
     *   parameter passing.
     *   \param a The value.
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::atomatom. */
    avalueflex(ATK *a, const atom *name=NULL);
    /**< Constructor to create avalueflex array with only one ATK object pointer value.
     *   It allows e.g. automatic creation of single-value arrays for
     *   parameter passing.
     *   The type atom is set to the name of the type, given by ATK::GetTYpeName().
     *   \param a The value.  Its lifetime must match the avalue lifetime.
     *   \param name The name.  If NULL, use avalue::none. */

    avalueflex(); /**< Create an empty avalueflex array.  */
    
    /** Returns the first avalue with the given name and type.
     *  This includes depth-first recursively searching avalues with avalue::avalueflex type.
     *  \param name The name of the desired avalue.
     *  \param type The type of the desired avalue, or NULL if it doesn't matter.
     *  \return an avalue with the given name and type, or NULL if none match. */
    avalue *Find(const atom *name, const atom *type=NULL);
    /** Returns the first avalue with the given name and type.
     *  This includes depth-first recursively searching avalues with avalue::avalueflex type.
     *  \param n The name of the desired avalue.
     *  \param t The type of the desired avalue, or NULL if it doesn't matter.
     *  \return an avalue with the given name and type, or NULL if none match. */
    inline const avalue *Find(const atom *n, const atom *t=NULL) const {
	avalueflex *huh=(avalueflex *)this;
	avalue *huh2=huh->Find(n, t);
	return huh2;
    }
    // Convenience operators to allow easy creation of argument lists without types or names.  Used like: avalueflex()|9|"foo"|((void *)bar)
    inline avalueflex &operator|(ATK *a) {
	Append()->Set(a);
	return *this;
    }
    /**< Convenience operator for creating nameless arrays in code.
     *   To the left is an array.  To the right is an ATK object pointer to add to that array.
     *   \param a An ATK object pointer.
     *   \return an array, so it can be chained. */
    inline avalueflex &operator|(long integer) {
	Append()->Set(integer);
	return *this;
    }
    /**< Convenience operator for creating nameless arrays in code.
     *   To the left is an array.  To the right is a long integer to add to that array.
     *   \param integer An integer.
     *   \return an array, so it can be chained. */
    inline avalueflex &operator|(double real) {
	Append()->Set(real);
	return *this;
    }
    /**< Convenience operator for creating nameless arrays in code.
     *   To the left is an array.  To the right is a floating-point value to add to that array.
     *   \param real A floating-point number.
     *   \return an array, so it can be chained. */
    inline avalueflex &operator|(char *str) {
	Append()->Set(str);
	return *this;
    }
    /**< Convenience operator for creating nameless arrays in code.
     *   To the left is an array.  To the right is a C string pointer to add to that array.
     *   \param str A C string pointer.
     *   \return an array, so it can be chained. */
    inline avalueflex &operator|(void *ptr) {
	Append()->Set(ptr);
	return *this;
    }
    /**< Convenience operator for creating nameless arrays in code.
     *   To the left is an array.  To the right is a generic pointer to add to that array.
     *   \param ptr A generic pointer.
     *   \return an array, so it can be chained. */
    inline avalueflex &operator|(avalueflex *avf) {
	Append()->Set(avf);
	return *this;
    }
    /**< Convenience operator for creating nameless arrays in code.
     *   To the left is an array.  To the right is an array pointer to add to that array.
     *   \param avf An avalueflex array pointer.
     *   \return an array, so it can be chained. */
    inline avalueflex &operator|(const atom *a) {
	Append()->Set(a);
	return *this;
    }
    /**< Convenience operator for creating nameless arrays in code.
     *   To the left is an array.  To the right is an atom pointer to add to that array.
     *   \param a An atom pointer.
     *   \return an array, so it can be chained. */
    // Support for adding named or typed elements in a more list-like syntax.
    inline avalueflex &add(ATK *a, const atom *name=NULL) {
	Append()->Set(a, name);
	return *this;
    }
    /**< Convenience function for creating named arrays in code.
     *   Add an ATK object pointer to an array, and return the array so more calls can be chained.
     *   The type will always be the registered name for the object type.
     *   \param a The ATK object pointer.
     *   \param name The name.  If NULL, use avalue::none.
     *   \return modified array. */
    inline avalueflex &add(long integer, const atom *name=NULL, const atom *type=NULL) {
	Append()->Set(integer, name, type);
	return *this;
    }
    /**< Convenience function for creating named arrays in code.
     *   Add an integer to an array, and return the array so more calls can be chained.
     *   \param integer The integer.
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::integer.
     *   \return modified array. */
    inline avalueflex &add(double real, const atom *name=NULL, const atom *type=NULL) {
	Append()->Set(real, name, type);
	return *this;
    }
    /**< Convenience function for creating named arrays in code.
     *   Add a floating-point number to an array, and return the array so more calls can be chained.
     *   \param real The floating-point number.
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::real.
     *   \return modified array. */
    inline avalueflex &add(const char *str, const atom *name=NULL, const atom *type=NULL) {
	Append()->Set(str, name, type);
	return *this;
    }
    /**< Convenience function for creating named arrays in code.
     *   Add a C string pointer to an array, and return the array so more calls can be chained.
     *   \param str The C string pointer.
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::cstring.
     *   \return modified array. */
    inline avalueflex &add(void *ptr, const atom *name=NULL, const atom *type=NULL) {
	Append()->Set(ptr, name, type);
	return *this;
    }
    /**< Convenience function for creating named arrays in code.
     *   Add a generic pointer to an array, and return the array so more calls can be chained.
     *   \param ptr The generic pointer.
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::voidptr.
     *   \return modified array. */
    inline avalueflex &add(avalueflex *avf, const atom *name=NULL, const atom *type=NULL) {
	Append()->Set(avf, name, type);
	return *this;
    }
    /**< Convenience function for creating named arrays in code.
     *   Add an avalueflex array pointer to an array, and return the array so more calls can be chained.
     *   \param avf The avalueflex array pointer.
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::valueflex.
     *   \return modified array. */
    inline avalueflex &add(const atom *a, const atom *name=NULL, const atom *type=NULL) {
	Append()->Set(a, name, type);
	return *this;
    }
    /**< Convenience function for creating named arrays in code.
     *   Add an atom pointer to an array, and return the array so more calls can be chained.
     *   \param a The atom pointer.
     *   \param name The name.  If NULL, use avalue::none.
     *   \param type The type.  If NULL, use avalue::atomatom.
     *   \return modified array. */
    int InterpretString(const char **str);
    /**< Add array elements from a string descriptor.
     *   Parses an array descriptor.  The descriptor has the following
     *   format, loosely in EBNF:
     *   \qcode{.unparsed}
     *     array = '(' [whitespace] { element [whitespace] } ')'
     *     element = [name '=' [whitespace]] value [ ':' type ]
     *     name = atom
     *     type = { not-whitespace-or-) }
     *     value = int    // integer type by default
     *           | float  // real type by default
     *           | string // cstring type by default [allocated by not freed!]
     *           | atom   // atomatom type by default
     *           | array  // nested array: valueflex type by default
     *     int = { [0-9] }+
     *     float = C-style floating point; must have . and start with digit
     *     string = '"' { string-char }+ '"'
     *     string-char = not-"\ \e \b \r \n \t \nnn \\ \" etc.
     *     atom = [alpha] { [^ \t=:)] }
     *   \endqcode
     *
     *   \param str A pointer to a variable pointing to the string to
     *              interpret.  On successful parse, this will be advanced
     *              past the closing parenthesis of the array descriptor.
     *   \return 0 on success, -1 on parse failure.  This array will have
     *           all successfully parsed elements added. */

    /** Check types of unnamed arguments in this array.
     *  Returns TRUE if and only if the types of the unnamed arguments
     *  in this avalueflex object match the type atoms listed in argtypes.
     *  If there are fewer elements in this avalueflex than in the
     *  argtypes FALSE will be returned.  If there are unnamed arguments
     *  in this avalueargs after the arguments matched to argtypes
     *  elements FALSE will be returned.  If there is a type mismatch,
     *  not only will FALSE be returned, but a message will also be printed
     *  on standard error.
     *  \param argtypes An array of atom values.  This function only cares
     *                  about initial unnamed values; their atom value
     *                  must match the type of this array's same-indexed
     *                  element.
     *  \return TRUE if the arguments' types match; FALSE otherwise. */
    boolean TypeCheck(const avalueflex &argtypes) const;

    /** Destroy the array.  Note that no values are freed or destroyed.  */
    virtual ~avalueflex();
};

DEFINE_OFLEX_CONST_ITER(avalueflex,avalue);
DEFINE_OFLEX_ITER(avalueflex, avalue);
/** @} */
#endif /* _AVALUE_H */
