# Persistent memory tools

## 1. Dependencies

  The persistent memory tools depend on a some external libraries

  1. libpmemobj, provided by pmem.io

    The path to this library may be specified to cmake using -DPMEMOBJ_PATH=...

  2. eckit

## Programming model

  1. Atomic operations
  2. PersistentPtr class
  3. Atomic constructor

    There are two possibilities to ensure consistency over arbitrary power failures. Either
    using transactions, or atomic operations. This set of persistent memory tools ensures
    consistency through atomic operations. This imposes some constraints:

      * All data is accessible through a global network of objects.
      * Object data is only modified during construction.
      * The last action taken in construction is to update (and persist) the persistent
        pointer to the object.

    To make use of this, constructor _functors_ should be written in place of normal object
    constructors. These should be of the form:

    ```
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
    ```

    Constructor objects should be passed into the allocate() method of a PersistentPtr object.
    The library will ensure that only constructors for the correct type of object are passed
    into PersistentPtrs of the appropriate type.

  4. Sample class definition

      ```c++
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
      ```

## Useful classes

  1. PersistentVector
  2. PersistentBuffer



