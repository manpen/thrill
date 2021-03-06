/*******************************************************************************
 * thrill/io/block_alloc_strategy.hpp
 *
 * Copied and modified from STXXL https://github.com/stxxl/stxxl, which is
 * distributed under the Boost Software License, Version 1.0.
 *
 * Part of Project Thrill - http://project-thrill.org
 *
 * Copyright (C) 2002-2007 Roman Dementiev <dementiev@mpi-sb.mpg.de>
 * Copyright (C) 2007-2009 Andreas Beckmann <beckmann@cs.uni-frankfurt.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once
#ifndef THRILL_IO_BLOCK_ALLOC_STRATEGY_HEADER
#define THRILL_IO_BLOCK_ALLOC_STRATEGY_HEADER

#include <thrill/io/config_file.hpp>

#include <algorithm>
#include <random>
#include <vector>

namespace thrill {
namespace io {

//! \defgroup io_layer_alloc Disk Allocation Functors
//! \ingroup io_layer
//! Standard disk allocation strategies encapsulated in functors.
//! \{

//! Example disk allocation scheme functor.
//! \remarks model of \b allocation_strategy concept
struct BasicAllocationStrategy
{
    BasicAllocationStrategy(int disks_begin, int disks_end);
    BasicAllocationStrategy();
    int operator () (int i) const;
    static const char * name();
};

//! Striping disk allocation scheme functor.
//! \remarks model of \b allocation_strategy concept
struct Striping
{
    size_t            begin_, diff_;

public:
    Striping(size_t b, size_t e) : begin_(b), diff_(e - b)
    { }

    Striping() : begin_(0) {
        diff_ = Config::GetInstance()->disks_number();
    }

    size_t operator () (size_t i) const {
        return begin_ + i % diff_;
    }

    static const char * name() {
        return "striping";
    }
};

//! Fully randomized disk allocation scheme functor.
//! \remarks model of \b allocation_strategy concept
struct FullyRandom : public Striping
{
private:
    mutable std::default_random_engine rng_ { std::random_device { } () };

public:
    FullyRandom(size_t b, size_t e) : Striping(b, e)
    { }

    FullyRandom() : Striping()
    { }

    size_t operator () (size_t /*i*/) const {
        return begin_ + rng_() % diff_;
    }

    static const char * name() {
        return "fully randomized striping";
    }
};

//! Simple randomized disk allocation scheme functor.
//! \remarks model of \b allocation_strategy concept
struct SimpleRandom : public Striping
{
private:
    size_t offset_;

    void init() {
        std::default_random_engine rng { std::random_device { } () };
        offset_ = rng() % diff_;
    }

public:
    SimpleRandom(size_t b, size_t e) : Striping(b, e) {
        init();
    }

    SimpleRandom() : Striping() {
        init();
    }

    size_t operator () (size_t i) const {
        return begin_ + (i + offset_) % diff_;
    }

    static const char * name() {
        return "simple randomized striping";
    }
};

//! Randomized cycling disk allocation scheme functor.
//! \remarks model of \b allocation_strategy concept
struct RandomCyclic : public Striping
{
private:
    std::vector<size_t> perm_;

    void init() {
        for (size_t i = 0; i < diff_; i++)
            perm_[i] = i;

        std::random_shuffle(perm_.begin(), perm_.end());
    }

public:
    RandomCyclic(size_t b, size_t e) : Striping(b, e), perm_(diff_) {
        init();
    }

    RandomCyclic() : Striping(), perm_(diff_) {
        init();
    }

    size_t operator () (size_t i) const {
        return begin_ + perm_[i % diff_];
    }

    static const char * name() {
        return "randomized cycling striping";
    }
};

struct RCDisk : public RandomCyclic
{
    RCDisk(size_t b, size_t e) : RandomCyclic(b, e)
    { }

    RCDisk() : RandomCyclic(Config::GetInstance()->regular_disk_range().first,
                            Config::GetInstance()->regular_disk_range().second)
    { }

    static const char * name() {
        return "Randomized cycling striping on regular disks";
    }
};

struct RCFlash : public RandomCyclic
{
    RCFlash(size_t b, size_t e) : RandomCyclic(b, e)
    { }

    RCFlash() : RandomCyclic(Config::GetInstance()->flash_range().first,
                             Config::GetInstance()->flash_range().second)
    { }

    static const char * name() {
        return "Randomized cycling striping on flash devices";
    }
};

//! 'Single disk' disk allocation scheme functor.
//! \remarks model of \b allocation_strategy concept
struct SingleDisk
{
    size_t            disk_;
    explicit SingleDisk(size_t d, size_t = 0) : disk_(d)
    { }

    SingleDisk() : disk_(0)
    { }

    size_t operator () (size_t /*i*/) const {
        return disk_;
    }

    static const char * name() {
        return "single disk";
    }
};

//! Allocator functor adaptor.
//!
//! Gives offset to disk number sequence defined in constructor
template <typename BaseAllocator>
struct OffsetAllocator
{
    BaseAllocator base_;
    size_t        offset_;

    //! Creates functor based on instance of \c BaseAllocator functor
    //! with offset \c offset.
    //! \param offset offset
    //! \param base used to create a copy
    OffsetAllocator(int offset, const BaseAllocator& base)
        : base_(base), offset_(offset) { }

    //! Creates functor based on instance of \c BaseAllocator functor.
    //! \param base used to create a copy
    explicit OffsetAllocator(const BaseAllocator& base)
        : base_(base), offset_(0) { }

    //! Creates functor based on default \c BaseAllocator functor.
    OffsetAllocator() : offset_(0) { }

    size_t operator () (size_t i) const {
        return base_(offset_ + i);
    }

    int  get_offset() const {
        return offset_;
    }

    void set_offset(size_t i) {
        offset_ = i;
    }
};

#ifndef THRILL_DEFAULT_ALLOC_STRATEGY
  #define THRILL_DEFAULT_ALLOC_STRATEGY ::thrill::io::FullyRandom
#endif

//! \}

} // namespace io
} // namespace thrill

#endif // !THRILL_IO_BLOCK_ALLOC_STRATEGY_HEADER

/******************************************************************************/
