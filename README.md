# Persistent memory tools

## 1. Dependencies

  The persistent memory tools depend on a some external libraries

  1. libpmemobj, provided by pmem.io

    The path to this library may be specified to cmake using -DPMEMOBJ_PATH=...

  2. eckit

## Programming model

  1. Atomic operations

    There are two possibilities to ensure consistency over arbitrary power failures. Either
    using transactions, or atomic operations. This set of persistent memory tools ensures
    consistency through atomic operations. This imposes some constraints:

      * All data is accessible through a global network of objects.
      * Object data is only modified during construction.
      * The last action taken in construction is to update (and persist) the persistent
        pointer to the object.

    To make use of this, constructor _functors_ should be written in place of normal object
    constructors.

    ...

    Note that the consistency ensured in this library only applies to process/power failure.
    No attempt is made at consistency with respect to concurrency. Threading/locking is
    entirely the remit of the library user.

  2. Persistent object types and the PersistentPtr class

    ... best place to put typeid is in the next section --- pool file

  3. PersistentPool class

    The PersistentPool class manages the persistent memory pool(s). The most straightforward
    way to use this is to define a....
    ...

        class MyPool : public pmem::PersistentPool {

        public: // methods

            MyPool(const eckit::PathName& path, const size_t size);

            pmem::PersistentPtr<TreeRoot> root() const { return getRoot<RootType>(); }
        };

  4. Atomic constructor

    ...
    These should be of the form:
    ...

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
        template<> int pmem::PersistentPtr<treetool::PersistentBuffer>::type_id = <unique no>;

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



