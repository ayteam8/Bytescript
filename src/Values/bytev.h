#ifndef BYTE_VALUE_H
#define BYTE_VALUE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// well i need to access tfree yk?


#define LJ_ALIGN(x) __attribute__((aligned(x)))


// TYPEDEFS

typedef struct {
	uint16_t ref;
} Obj;//basically to have a ref counter that is for EVERY THING, instead of passing the object, you put the object casted to a Obj struct

typedef uint64_t TBool;
typedef double TNumber;

typedef LJ_ALIGN(8) union BValue {
    uint64_t nan;
    TNumber  num;
} TValue;


// MAIN TYPES

typedef TValue (*CFunc)(TValue* argv, size_t argc);

typedef struct Func {
    uint16_t ref;
    uint8_t isCfunc;
    union {
        CFunc cfunc;
        struct {
            const uint8_t* bytecode;
            TValue* const * upvalues;
            uint8_t const_len;
            const TValue constPool[];
        } bytescriptFunction;
    };
} TFunc;

typedef struct Str {
    uint16_t ref;
    size_t len;
    char str[];
} TStr;


// TABLES


typedef struct {
    uint16_t ref;
    size_t len;
    size_t capacity;
    TValue* array;
} Array;

typedef struct {
    TValue key;
    TValue value;
} Pair;

typedef struct {
    size_t len;
    size_t capacity;
    size_t tombstones;
    Pair* arr;
} PairArray;

typedef struct {
    Pair pair;
    PairArray sub;
} Bucket;

typedef struct {
    size_t len;
    size_t capacity;
    Bucket* array;
} HashTable;

typedef struct Table {
    uint16_t ref;
    Array array;
    HashTable hash_table;
} TTable;


// OOP


typedef struct {
    uint16_t ref;
    size_t inherited_count;
    TValue fields[];
} TClass;

typedef struct {
    uint16_t ref;
    TClass* class;
    TValue fields[];
} TObject;


// CDATA


typedef struct { // idk if i have to add here also the ref counter, idk
    char* name;
    // add stuff later
} TCDataType;

typedef TCDataType* TCData;
#define CDATA_HEADER TCData cdata_type


// NAN BOXING


typedef TValue* TStack;
#define TVALUESIZE sizeof(TValue)

#define EXP_SHIFT 52
#define Q_SHIFT 51

#define TYPE_AMOUNT 3
#define TYPE_SHIFT (Q_SHIFT - TYPE_AMOUNT)



#define NaN             (0x7ffULL << EXP_SHIFT)
#define QNaN_shift      (0x01ULL << 51)
#define QNaN            (NaN | QNaN_shift)

#define NAN_AS_TVALUE(v)((TValue){ .nan = (v) })
#define NUM_AS_TVALUE(v)((TValue){ .num = (v) })

#define AS_TVALUE(v)    ((v).nan)
#define AS_NUM(v)       ((v).num)

#define TYPE_MASK       (((0x01ULL << TYPE_AMOUNT) - 1) << TYPE_SHIFT)
#define TYPE_STRING     (0x01ULL << TYPE_SHIFT)
#define TYPE_FUNC       (0x02ULL << TYPE_SHIFT)
#define TYPE_TABLE      (0x03ULL << TYPE_SHIFT)
#define TYPE_CLASS      (0x04ULL << TYPE_SHIFT)
#define TYPE_OBJECT     (0x05ULL << TYPE_SHIFT)
#define TYPE_CDATA      (0x06ULL << TYPE_SHIFT)

#define PAYLOAD_MASK    (~(QNaN | TYPE_MASK | 0x01ULL << 63))

#define NIL             (NAN_AS_TVALUE(QNaN))

#define STRING          (TYPE_STRING | QNaN)
#define FUNC            (TYPE_FUNC | QNaN)
#define TABLE           (TYPE_TABLE | QNaN)
#define CLASS           (TYPE_CLASS | QNaN)
#define OBJECT          (TYPE_OBJECT | QNaN)
#define CDATA           (TYPE_CDATA | QNaN)

#define IS_NIL(v)       (AS_TVALUE(v) == AS_TVALUE(NIL))
#define IS_NUM(v)       ((AS_TVALUE(v) & NaN) != NaN)
#define IS_STRING(v)    ((AS_TVALUE(v) & ~PAYLOAD_MASK) == STRING)
#define IS_FUNC(v)      ((AS_TVALUE(v) & ~PAYLOAD_MASK) == FUNC)
#define IS_TABLE(v)     ((AS_TVALUE(v) & ~PAYLOAD_MASK) == TABLE)
#define IS_CLASS(v)     ((AS_TVALUE(v) & ~PAYLOAD_MASK) == CLASS)
#define IS_OBJECT(v)    ((AS_TVALUE(v) & ~PAYLOAD_MASK) == OBJECT)
#define IS_CDATA(v)     ((AS_TVALUE(v) & ~PAYLOAD_MASK) == CDATA)

#define PAYLOAD(v)      (AS_TVALUE(v) & PAYLOAD_MASK)
#define TYPE(v)         (AS_TVALUE(v) & (TYPE_MASK | QNaN))

#define AS_STRING(v)    ( ( TStr* )( PAYLOAD(v) ) )
#define AS_FUNC(v)      ( ( TFunc* )( PAYLOAD(v) ) )
#define AS_TABLE(v)     ( ( TTable* )( PAYLOAD(v) ) )
#define AS_CLASS(v)     ( ( TClass* )( PAYLOAD(v) ) )
#define AS_OBJECT(v)    ( ( TObject* )( PAYLOAD(v) ) )
#define AS_CDATA(v)     ( ( TCData* )( PAYLOAD(v) ) )
#define AS_OBJ(v)	( (Obj *)( PAYLOAD(v) ) )

#define STR_TO_TVALUE(v)( NAN_AS_TVALUE(( (uint64_t)(v) ) | STRING) )
#define FNC_TO_TVALUE(v)( NAN_AS_TVALUE(( (uint64_t)(v) ) | FUNC) )
#define TBL_TO_TVALUE(v)( NAN_AS_TVALUE(( (uint64_t)(v) ) | TABLE) )
#define CLS_TO_TVALUE(v)( NAN_AS_TVALUE(( (uint64_t)(v) ) | CLASS) )
#define OBJ_TO_TVALUE(v)( NAN_AS_TVALUE(( (uint64_t)(v) ) | OBJECT) )
#define CDT_TO_TVALUE(v)( NAN_AS_TVALUE(( (uint64_t)(v) ) | CDATA) )

#define EQUAL(tv1, tv2) ( AS_TVALUE(tv1) == AS_TVALUE(tv2) )
#define TRUTHY(v)       ( AS_NUM(v) && !IS_NIL(v) )
#define FALSY(v)        ( !AS_NUM(v) || IS_NIL(v) )
#define TRUE_TVALUE     ( NUM_AS_TVALUE(1) )
#define FALSE_TVALUE    ( NUM_AS_TVALUE(0) )

#define BSIZE(v)        (TVALUESIZE * v)
#define READ_CDATA(tv)  ( *AS_CDATA(tv) )


static int compare_tvalues(TValue tv1, TValue tv2) {
    if (TYPE(tv1) != TYPE(tv2)) return AS_TVALUE(tv1) == AS_TVALUE(tv2);

    switch (TYPE(tv1)) {
        case STRING: return !strcmp(AS_STRING(tv1)->str, AS_STRING(tv2)->str);
        default: return AS_TVALUE(tv1) == AS_TVALUE(tv2);
    }
}

static TValue copy_tvalue(TValue tv1) {
    switch (TYPE(tv1)) {
        case STRING: {
            TStr* str = AS_STRING(tv1);
	    str->ref = 0; // we'll
            TStr* copied = malloc(sizeof(TStr) + str->len + 1);
            strncpy(copied->str, str->str, str->len + 1);
            
            return STR_TO_TVALUE(copied);
        }

        default: return tv1;
    }
}

static TValue new_func(const uint8_t* bytecode, const TValue* const_pool, uint8_t const_len, TValue* const* upvalues) {
    TFunc* func = malloc(sizeof(TFunc) + BSIZE(const_len));
    func->ref = 0;
    func->isCfunc = 0;
    func->bytescriptFunction.bytecode = bytecode;
    func->bytescriptFunction.upvalues = upvalues;
    func->bytescriptFunction.const_len = const_len;
    memcpy((void*)func->bytescriptFunction.constPool, const_pool, BSIZE(const_len));

    return FNC_TO_TVALUE(func);
}

static TValue new_cfunc(CFunc cfunc) {
    TFunc* func = malloc(sizeof(TFunc));
    func->isCfunc = 1;
    func->cfunc = cfunc;

    return FNC_TO_TVALUE(func);
}

// OK, HERE ARE THE NEW FUNCTIONS, ADD_REF, AND DEL_REF
static void add_ref(TValue obj) {
	AS_OBJ(obj)->ref++;
}

// i have to include here because GLOBALS_H needs the rest of bytev.h

#include "../Globals/globals.h"

static void del_ref(TValue obj) { // this one has to take TValue because we have to know the type yk
	if (AS_OBJ(obj)->ref<=0) {
		// OK, TIME TO FREE THE OBJECT!
		if (IS_STRING(obj) || IS_FUNC(obj) || IS_TABLE(obj)) {
			tfree(&obj, 1); // yeah, yk, i think
					// this will get the job
					// done
		} else {
			// i'll figure out what the hell i have to do
		}

	} else {
		AS_OBJ(obj)->ref--;
	}
}

// new defines for the reference counter
#define REF_DEL(v)	( del_ref(v) )
#define REF_ADD(v)	( add_ref(v) )

#endif
