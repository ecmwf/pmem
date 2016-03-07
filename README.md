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



