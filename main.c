#include <stdio.h>
#include <stdlib.h>

#define STACK_MAX 256

// ----------------------------- Data Structures

typedef enum {
	OBJ_INT,
	OBJ_PAIR
} ObjectType;

typedef struct sObject { // Object for dynamically typed language with only two kinds of datatypes
	ObjectType type;

	union { // stores either one, not both

		/* OBJ_INT */
		int value;

		/* OBJ_PAIR */
		struct {
			struct sObject* head;
			struct sObject* tail;
		};
	};
} Object;

typedef struct { // stack containing Objects, emulating a VM
	Object* stack[STACK_MAX];
	int stackSize;
} VM;

// ----------------------------- Helper functions

VM* newVM() {
	VM* vm = malloc(sizeof(VM));
	vm->stackSize = 0;
	return vm;
}

void push(VM* vm, Object* value) {
	assert(vm->stackSize < STACK_MAX, "Stack overflow!");
	vm->stack[vm->stackSize++] = value;
}

Object* pop(VM* vm) {
	assert(vm->stackSize > 0, "Stack underflow!");
	return vm->stack[--vm->stackSize];
}

Object* newObject(VM* vm, ObjectType type) {
	Object* obj = malloc(sizeof(Object));
	obj->type = type;
	return obj;
}

// ----------------------------- User functions

void pushInt(VM* vm, int intValue) {
	Object* obj = newObject(vm, OBJ_INT);
	obj->value = intValue;
	push(vm, obj);
}

Object* pushPair(VM* vm) { // to push a pair, push two ints and call pushPair()
	Object* obj = newObject(vm, OBJ_PAIR);
	obj->tail = pop(vm);
	obj->head = pop(vm);
	push(vm, obj);
	return obj;
}