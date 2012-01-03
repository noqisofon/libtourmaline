//  
//  GenericTypes.h
//  
//  Auther:
//       ned rihine <ned.rihine@gmail.com>
// 
//  Copyright (c) 2011 rihine All rights reserved.
// 
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 
#ifndef coreHezelnut_GenericTypes_h
#define coreHezelnut_GenericTypes_h

#include <stddef.h>

typedef unsigned char Boolean;

typedef char* CSTR;

typedef signed int ErrorResult;
typedef signed int SystemStatus;



#ifndef TRUE
#   define TRUE 1
#endif  /* ndef TRUE */

#ifndef FALSE
#   define FALSE 0
#endif  /* ndef FALSE */


#if !defined(_EXTERN_C_BEGIN)
#   ifdef __cplusplus
#       define _EXTERN_C_BEGIN extern "C" {
#       define _EXTERN_C_END   }
#   else
#       define _EXTERN_C_BEGIN
#       define _EXTERN_C_END
#   endif   /* def __cplusplus */
#endif  /* !defined(_EXTERN_C_BEGIN) */


#if defined(CORE_HEZELNUT_TARGET_OS_WIN32) && CORE_HEZELNUT_TARGET_OS_WIN32
#else
#   define _EXPORT
#endif  /* defined(CORE_HEZELNUT_TARGET_OS_WIN32) && CORE_HEZELNUT_TARGET_OS_WIN32 */

_EXTERN_C_BEGIN

#if !defined(NULL)
#   if defined(__GNUG__)
#       define NULL __null
#   elif defined(__cplusplus)
#       define NULL 0
#   else
#       define NULL ((void *)0)
#   endif  /* defined(__GNUG__) */
#endif  /* !defined(NULL) */


typedef unsigned int ClassID;
typedef unsigned int BitField;
typedef unsigned int Index;
typedef unsigned int HashCode;

typedef const void* Value_ref;

typedef const struct __String* ConstantString_ref;
typedef       struct __String* MutableString_ref;

#define String_ref ConstantString_ref


enum __ComparisonResult {
    Compare_LessThan = -1,
    Compare_EqualTo = 0,
    Compare_GreaterThan = 1
};
typedef Index ComparisonResult;


typedef ComparisonResult (*ComparisonFunction)(Value_ref left, Value_ref right, void* context);


enum __AccessResult {
    AccessResult_NotFound = -1
};


typedef struct __Range {
    Index index;
    Index length;
} Range;


typedef struct __Nil* Nil_ref;



typedef struct __Allocator* Allocator_ref;

typedef struct tour_class* MetaClass_ref;
typedef struct tour_class* Class_ref;


_EXTERN_C_END

#endif  /* coreHezelnut_GenericTypes_h */
