# Mark-and-Sweep Garbage Collector

The basic idea behind garbage collection is that the language appears to have access to infinite memory. The developer can just keep allocating and allocating and allocating and, as if by magic, it never fails. But of course, machines don't have infinite memory. So the way the implementation does this is that when it needs to allocate a bit of memory and it realizes it's running low, it collects garbage.

"Garbage" in this context means memory it previously allocated that is no longer being used. For the illusion of infinite memory to work, the language needs to be very safe about "no longer being used". It would be no fun if random objects just started getting reclaimed while your program was trying to access them.

In order to be collectible, the language has to ensure there's no way for the program to use that object again. If it can't get a reference to the object, then it obviously can't use it again. So the definition of "in use" is actually pretty simple:

* Any object that's being referenced by a variable that's still in scope is in use.
* Any object that's referenced by another object that's in use is in use.

The end result is a graph of reachable objects — all objects that you can get to by starting at a variable and traversing through objects. Any object not in that graph of reachable objects is dead to the program and its memory is ripe for a reaping.

## Marking and sweeping

* Starting at the roots, traverse the entire object graph. Every time you reach an object, set a "mark" bit on it to true.
* Once that's done, find all of the objects whose mark bits are not set and delete them.

## Implementation

In the implementation, the object graph is implemented in the form of a linked list (to make it easy to iterate over all of them). All the objects are part of a stack-based miniaturized VM. Even the objects are of a special type: they can either be an integer or a pair of integers. In C, this is implemented as a [Tagged union](https://en.wikipedia.org/wiki/Tagged_union).
