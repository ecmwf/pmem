# Persistent memory tools

## Dependencies

  The persistent memory tools depend on a some external libraries

  1. libpmemobj, provided by pmem.io

    The path to this library may be specified to cmake using -DPMEMOBJ_PATH=...

  2. eckit

## Programming model

  1. Overview

    Each memory pool (see PersistentPool below) can have only one persistent root object.
    All other objects within the memory pool must ultimately be accessible exploration
    of a network of connected objects within (or between) the memory pools. These
    connections are described by PersistentPtrs within the objects.

    This implies that the entire range of possible types that can be stored within the
    memory pool can be described at compile time (as no pointer can be stored to an
    unknown type). To ensure consistency over restart, the library stores a type tag
    (type_id) along with each allocated object. This is checked against the expected
    type every time that a PersistentPtr is resolved. This requires **unique** type_ids
    to be defined for every storable object at compile time.

    **It is strongly advised that these are stored in a single place, normally in the c++
    file associated with the PersistentPool object***

    There are two possibilities to ensure consistency over arbitrary power failures. Either
    using transactions, or atomic operations. This set of persistent memory tools ensures
    consistency through atomic operations. This imposes some constraints:

      * All data is accessible through a global network of objects.
      * Object data is only modified during construction.
      * The last action taken in construction is to update (and persist) the persistent
        pointer to the object.

    To make use of this, constructor _functors_ should be written in place of normal object
    constructors.

    Note that the consistency ensured in this library only applies to process/power failure.
    No attempt is made at consistency with respect to concurrency. Threading/locking is
    entirely the remit of the library user.

  2. Persistent object types and the PersistentPtr class

    Objects that can be stored in persistent memory have a certain number of rules.

      1. They must not contain mutable data, other than PersistentPtrs to other objects
         which can be atomically allocated (or types that wrap such PersistentPtrs such
         as PersistentVector).

      2. They must not have any virtual methods, or derive from classes with virtual
         methods. Virtual methods rely on having a vtable which maps the base class
         methods to their actual implementation. This vtable contains actual poniters
         to volatile memory, which are invalid when a pool is re-memory-mapped.

      3. They must have constructors and destructors that do nothing, so that this
         behaviour can be managed by the Constructor functor objects (see Atomic
         Constructor section below), and the library.

    Pointers to objects stored in persistent memory are of type PersistentPtr<ObjectType>.
    This wraps the PMEMoid type in the underlying libpmemobj library, and contains a
    pool uuid, and a memory offset. No further information is stored, and the data
    structures stored are in principle accessible using the libpmemobj C API.

    Each of the types available **must** have a globally unique type_id declared. This is
    used for type checking on access to the persistent memory, which helps with
    maintining long-term consistency. All type definitions should be placed in the same
    place, to describe the ensemble behaviour of the persistent pool in consideration.
    This should normally be in the .cc file associated with the local PersistentPool
    definition:

        template<> int pmem::PersistentPtr<PersistentObject>::type_id = <unique no>;

    Further objects should be allocated by calling the `allocate()` method of the
    persistent pointer. The library will ensure that this action is atomic, with
    persisting the updated pointer being the last operation to occur. Memory can
    be freed using the `free()` method.

    If more complicated data structures are to be built (those that could not be written
    as a branching tree, e.g. that contain loops of pointers, a great deal of care is
    required. These *cannot* be built using single atomic operations, and so the loops
    will need to be built in such a way that the state can be "recovered" and the desired
    structure rebuilt on restart and reopening of the pool.

    TODO: Add some utility functions for manipulating pointers to create more complex
          data structures.

  3. PersistentPool class

    The PersistentPool class manages the persistent memory pool(s), and takes care of either
    creating or allocating (with atomic initialisation of the root object) as appropriate.
    The most straightforward way to use this is to define a derived type. This type would
    know the type of the root object.

        class MyPool : public pmem::PersistentPool {

        public: // methods

            MyPool(const eckit::PathName& path, const size_t size) :
                PesistentPool(path, size, "pool-id-name", RootType::Constructor()) {}

            pmem::PersistentPtr<TreeRoot> root() const { return getRoot<RootType>(); }
        };

  4. Atomic constructor

    These should be of the form:

        class Constructor : public pmem::AtomicConstructor<ObjectType> {

        public: // methods

            // User defined interface
            // Any information that is required to encode the object must be passed in
            // during construction of the Constructor functor.
            Constructor(...);

            // This is the actual constructor method. It is passed a pointer to an uninitialised
            // (but allocated) region of persistent memory.
            virtual void make (ObjectType * object) const;

            // (Optional)
            // This routine is called to determine how much persistent memory should be allocated
            // (and similarly how much should be persisted after make() is called). If this
            // method is not provided the default implementation returns sizeof(ObjectType).
            //
            // This is primarily intended for allocating variable sized objects, such as buffers.
            virtual size_t size() const;

        private: // members

            // Information stored by the constructor, and required in the make()
            // method should be stored here.
        };

    Constructor objects should be passed into the allocate() method of a PersistentPtr object.
    The library will ensure that only constructors for the correct type of object are passed
    into PersistentPtrs of the appropriate type.

    The "root" object cannot be allocated in the normal way, as it must always be present.


  5. Sample class definition

    Build off this sample class definition

        class PersistentObject {

        public: // Constructor function objects

            class Constructor : public pmem::AtomicConstructor<PersistentObject> {
            };

        public: // methods

            // Methods to access the data, and manipulate overall data structures.
            //
            // i) All data access should be read-only.

        private: // members

            // All of the data members to be stored should be in this private section. They should
            // not be public, to protect them from external modification.

        priavte: // friends

            // Constructor functors need  access to the internal data
            friend class PersistentObject::Constructor;
        };


        // Somewhere in a global cpp file
        template<> int pmem::PersistentPtr<PersistentObject::type_id = <unique no>;

## Useful classes

  1. PersistentVector<T>

    This stores an extensible array of persistent pointers to a given object type. This object
    may be stored as a normal member of any object stored in persistent memory. It provides
    normal routines as would be expected from a vector type:

      * `size_t size() const` - return the number of elements in the list
      * `const T& operator[] (size_t i) const` - Obtain the i'th element
      * `push_back(const AtomicConstructor<T>& constructor)` - Append an item

    To make use of this, the data type stored internally in the PersistentVector must be
    assigned a unique type_id as part of the macroscopic type management system.

        template<> int pmem::PersistentPtr<pmem::PersistentVector<T>::data_type>::type_id = <unique no>;

  2. PersistentBuffer

    This type does what it says on the tin. Its constructor object accepts a pointer to a
    region of memory and a length. This data is then stored persistently.



