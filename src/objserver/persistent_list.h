/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   Feb 2016


#ifndef objserver_persistent_list_H
#define objserver_persistent_list_H

#include "eckit/types/FixedString.h"

#include "objserver/persistent_ptr.h"


template <typename T>
class persistent_list {
public: // types

    // NOTE: These types MUST be included in the layout for the overall poll

    struct persistent_list_elem {
        pmem::persistent_ptr<persistent_list_elem> prev;
        pmem::persistent_ptr<persistent_list_elem> next;

        T data;
    };

    struct persistent_list_root {
        eckit::FixedString<8> tag;

        uint8_t version;
        char unused[7];

        pmem::persistent_ptr<persistent_list_elem> head;
        pmem::persistent_ptr<persistent_list_elem> tail;
    };


    // Object constructor
    typedef pmem::atomic_constructor<T> data_constructor;

    class const_iterator {
    public:
        const_iterator();
        const_iterator(pmem::persistent_ptr<persistent_list_elem>& item);

        const_iterator operator++(int);
        const_iterator& operator++();
    private:
        pmem::persistent_ptr<persistent_list_elem> item_;

        friend bool operator== (const const_iterator& lhs, const const_iterator& rhs) {
            return lhs.item_ == rhs.item_;
        }
        friend bool operator!= (const const_iterator& lhs, const const_iterator& rhs) {
            return !(lhs == rhs);
        }
    };

protected: // types

    // Construtor for list elements
    class element_constructor : public pmem::atomic_constructor<persistent_list_elem> {
    public:
        element_constructor(const data_constructor& dc) :
            dataConstructor_(dc) {}

        virtual size_t size() const {
            return sizeof(persistent_list_elem) - sizeof(T) + dataConstructor_.size();
        }

    protected: // data
        const data_constructor& dataConstructor_;
    };

public: // methods

    persistent_list(PMEMobjpool * pop, bool allocate_new = false);

    void push_back(const data_constructor& constructor);

    const_iterator begin() const;

    const_iterator end() const;

private: // data

    PMEMobjpool * pop_;

    pmem::persistent_ptr<persistent_list_root> root_;
};


// -------------------------------------------------------------------------------------------------


template <typename T>
persistent_list<T>::persistent_list(PMEMobjpool * pop, bool allocate_new) :
    pop_(pop) {

    // Decribe the building of the root object
    class ConstructListRoot : public pmem::atomic_constructor<persistent_list_root> {
        virtual void make(persistent_list_root * obj) const {
            obj->tag = eckit::FixedString<8>("PMEM0001");
            obj->version = 1;
        }
    };

    root_ = pmem::persistent_ptr<persistent_list_root>::get_root_object(pop_);

    if (allocate_new) {
        ConstructListRoot root_builder;
        root_.allocate_root(pop, root_builder);
    }
}


// -------------------------------------------------------------------------------------------------


template <typename T>
void persistent_list<T>::push_back(const data_constructor& constructor) {

    // Build the element to insert for a push back.
    class ElementConstructor : public element_constructor {
    public:

        ElementConstructor(const data_constructor& constructor, 
                           pmem::persistent_ptr<persistent_list_elem> prev, 
                           pmem::persistent_ptr<persistent_list_elem> next) :
            element_constructor(constructor),
            prev_(prev), next_(next) {}

        virtual void make(persistent_list_elem * elem) const {
            elem->next = next_;
            elem->prev = prev_;
            this->dataConstructor_.make(&elem->data);
        }
    private:
        pmem::persistent_ptr<persistent_list_elem> prev_;
        pmem::persistent_ptr<persistent_list_elem> next_;
    };

    ElementConstructor elem_constructor(constructor, root_->tail, pmem::persistent_ptr<persistent_list_elem>());

    root_->tail.allocate(pop_, elem_constructor);

    // TODO: The persistence and modification should be pulled into persistent_ptr
    if (root_->head.null()) {
        root_->head = root_->tail;
        ::pmemobj_persist(pop_, &root_->head, sizeof(root_->head));
    }
}


// -------------------------------------------------------------------------------------------------


template<typename T>
persistent_list<T>::const_iterator::const_iterator() :
    item_() {}


template<typename T>
persistent_list<T>::const_iterator::const_iterator(pmem::persistent_ptr<persistent_list_elem>& item) :
    item_(item) {}


template<typename T>
typename persistent_list<T>::const_iterator& persistent_list<T>::const_iterator::operator++() {
    item_ = item_->next;
    return *this;
}


template<typename T>
typename persistent_list<T>::const_iterator persistent_list<T>::const_iterator::operator++(int) {
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}



template<typename T>
typename persistent_list<T>::const_iterator persistent_list<T>::begin() const {
    return const_iterator(root_->head);
}


template<typename T>
typename persistent_list<T>::const_iterator persistent_list<T>::end() const {
    return const_iterator();
}


#endif // objserver_persistent_list_H
