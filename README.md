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

## Useful classes

  1. PersistentVector
  2. PersistentBuffer



