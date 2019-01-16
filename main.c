#include <stdio.h>
#include <stdlib.h>

#define STACK_MAX 256
#define INITIAL_GC_THRESHOLD 8

// ----------------------------- Data Structures

typedef enum {
	OBJ_INT,
	OBJ_PAIR
} ObjectType;

typedef struct sObject { // Object for dynamically typed language with two kinds of datatypes
	unsigned char marked;
	ObjectType type;
	struct sObject* next; // next pointer (support for the linked list of all Objects)

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

	/* VM has its own references to Objects that are distinct from the semantics
	visible to the language user (stack). This is done by maintaining a linked list
	of every Object ever allocated. */

	Object* firstObject; // VM keeps track of head of linked list

	int numObjects; // number of currently allocated Objects
	int maxObjects;
} VM;

// ----------------------------- Utility functions

void assert(int condition, const char* msg) {
	if (!condition) {
		printf("%s\n", msg);
		exit(1);
	}
}

void objectPrint(Object* obj) {
	switch (obj->type) {
		case OBJ_INT:
			printf("%d", obj->value);
			break;
		case OBJ_PAIR:
			printf("(");
			objectPrint(obj->head);
			printf(", ");
			objectPrint(obj->tail);
			printf(")");
			break;
	}
}

// ----------------------------- Helper functions

VM* newVM() {
	VM* vm = malloc(sizeof(VM));
	vm->stackSize = 0;
	vm->firstObject = NULL;
	vm->numObjects = 0;
	vm->maxObjects = INITIAL_GC_THRESHOLD;
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

// ----------------------------- MARK

void mark(Object* obj) {
	if (obj->marked)
		return; // to prevent cycles
	
	obj->marked = 1;
	if (obj->type == OBJ_PAIR) {
		mark(obj->head); mark(obj->tail);
	}
}

void markAll(VM* vm) {
	for (int i = 0; i < vm->stackSize; ++i)
		mark(vm->stack[i]);
}

// ----------------------------- SWEEP

void sweep(VM* vm) {
	Object** obj = &vm->firstObject;
	while (*obj) {
		if (!(*obj)->marked) {
			Object* unreached = *obj;
			*obj = unreached->next; // skipping object in linked list
			free(unreached);
			--vm->numObjects;
		} else {
			(*obj)->marked = 0; // unmarking for next gc cycle
			obj = &(*obj)->next;
		}
	}
}

void gc(VM* vm) {
	int numObjects = vm->numObjects;
	markAll(vm); sweep(vm);

	/* After every collection, update maxObjects based on the number of live objects left.
	The multiplier there lets the heap grow as the number of living objects increases.
	Likewise, it will shrink automatically if a bunch of objects end up being freed. */
	vm->maxObjects = vm->numObjects * 2;

	printf("Collected %d objects, %d remain...\n", numObjects - vm->numObjects, vm->numObjects);
}

Object* newObject(VM* vm, ObjectType type) {
	if (vm->numObjects == vm->maxObjects) gc(vm); // gc kicks in when maxObjects reached

	Object* obj = malloc(sizeof(Object));
	obj->type = type;
	obj->marked = 0;

	obj->next = vm->firstObject; // new object added to left/head end of linked list
	vm->firstObject = obj;

	++vm->numObjects;

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

void freeVM(VM *vm) {
	vm->stackSize = 0; gc(vm);
	free(vm);
}

// ----------------------------- TESTS

void test1() {
	printf("Test 1: Objects on stack preserved\n");
	VM* vm = newVM();
	pushInt(vm, 1); pushInt(vm, 2);

	gc(vm);
	assert(vm->numObjects == 2, "Should have preserved objects");
	freeVM(vm);
}

void test2() {
	printf("Test 2: Unreached objects collected\n");
	VM* vm = newVM();
	pushInt(vm, 1); pushInt(vm, 2); pop(vm); pop(vm);

	gc(vm);
	assert(vm->numObjects == 0, "Should have collected objects");
	freeVM(vm);
}

void test3() {
	printf("Test 3: Reach pairs\n");
	VM* vm = newVM();
	pushInt(vm, 1); pushInt(vm, 2); pushPair(vm);
	pushInt(vm, 3); pushInt(vm, 4); pushPair(vm);
	pushPair(vm); // nesting

	gc(vm);
	assert(vm->numObjects == 7, "Should have reached objects");
	freeVM(vm);
}

void test4() {
	printf("Test 4: Handle cycles\n");
	VM* vm = newVM();
	pushInt(vm, 1); pushInt(vm, 2);
	Object *a = pushPair(vm);
	pushInt(vm, 3); pushInt(vm, 4);
	Object *b = pushPair(vm);

	// Create cycle (also making 2 and 4 unreachable)
	a->tail = b;
	b->tail = a;

	gc(vm);
	assert(vm->numObjects == 4, "Should have collected objects");
	freeVM(vm);
}

void perfTest() {
	printf("Performance Test\n");
	VM *vm = newVM();
	for (int i = 0; i < 100; ++i) {
		for (int j = 0; j < 20; ++j)
			pushInt(vm, i);
		for (int k = 0; k < 20; ++k)
			pop(vm);
	}
	freeVM(vm);
}


int main(int argc, const char* argv[]) {

	test1(); test2(); test3(); test4();
	perfTest();

	return 0;
}
