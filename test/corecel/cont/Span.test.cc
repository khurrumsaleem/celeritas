//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file corecel/cont/Span.test.cc
//---------------------------------------------------------------------------//
#include "corecel/cont/Span.hh"

#include <iomanip>
#include <sstream>
#include <type_traits>

#include "corecel/OpaqueId.hh"
#include "corecel/cont/SpanIO.hh"
#include "corecel/data/LdgIterator.hh"

#include "celeritas_test.hh"

namespace celeritas
{
namespace test
{
namespace
{
//---------------------------------------------------------------------------//

template<class T, std::size_t E>
std::string span_to_string(Span<T, E> const& s)
{
    std::ostringstream os;
    os << s;
    return os.str();
}

//---------------------------------------------------------------------------//
}  // namespace

TEST(SpanTest, fixed_size_zero)
{
    Span<int, 0> empty_span;
    EXPECT_EQ(nullptr, empty_span.data());
    EXPECT_EQ(0, empty_span.size());
    EXPECT_TRUE(empty_span.empty());
    EXPECT_EQ(sizeof(int*), sizeof(empty_span));
    EXPECT_EQ("{}", span_to_string(empty_span));

    {
        auto templ_subspan = empty_span.subspan<0, 0>();
        EXPECT_TRUE((
            std::is_same<decltype(empty_span), decltype(templ_subspan)>::value));
    }
    {
        auto templ_subspan = empty_span.first<0>();
        EXPECT_TRUE((
            std::is_same<decltype(empty_span), decltype(templ_subspan)>::value));
    }
    {
        auto templ_subspan = empty_span.last<0>();
        EXPECT_TRUE((
            std::is_same<decltype(empty_span), decltype(templ_subspan)>::value));
    }

    // Test copy constructor
    Span<int, 0> other_span{empty_span};
    EXPECT_EQ(nullptr, other_span.data());
    EXPECT_EQ(0, other_span.size());
    empty_span = other_span;

    // Test dynamic conversion
    Span<int> dynamic{empty_span};
    EXPECT_EQ(nullptr, dynamic.data());
    EXPECT_EQ(0, dynamic.size());

    // Test type conversion
    Span<int const, 0> const_span{empty_span};
    EXPECT_EQ(0, const_span.size());

    // Test type conversion
    Span<int const> const_dynamic{empty_span};
    EXPECT_EQ(0, const_dynamic.size());

    // Test pointer constructor
    Span<int, 0> ptr_span(empty_span.begin(), empty_span.end());
    EXPECT_EQ(0, ptr_span.size());
}

TEST(SpanTest, fixed_size)
{
    int local_data[] = {123, 456};
    Span<int, 2> local_span(local_data);
    EXPECT_EQ(sizeof(int*), sizeof(local_span));
    EXPECT_EQ("{123,456}", span_to_string(local_span));

    EXPECT_EQ(local_data, local_span.begin());
    EXPECT_EQ(local_data + 2, local_span.end());
    EXPECT_EQ(local_data, local_span.data());
    EXPECT_EQ(2, local_span.size());
    EXPECT_EQ(2 * sizeof(int), local_span.size_bytes());
    EXPECT_FALSE(local_span.empty());
    EXPECT_EQ(&local_data[1], &(local_span[1]));
    EXPECT_EQ(123, local_span.front());
    EXPECT_EQ(456, local_span.back());

    auto templ_subspan = local_span.subspan<1>();
    EXPECT_TRUE((std::is_same<Span<int, 1>, decltype(templ_subspan)>::value));
    EXPECT_EQ(local_data + 1, templ_subspan.data());

    auto func_subspan = local_span.subspan(1, 1);
    EXPECT_EQ(1, func_subspan.size());
    EXPECT_EQ(local_data + 1, func_subspan.data());

    // Test copy constructor
    Span<int, 2> other_span{local_span};
    EXPECT_EQ(local_data, other_span.data());
    EXPECT_EQ(2, other_span.size());
    local_span = other_span;

    // Test dynamic conversion
    Span<int> dynamic{local_span};
    EXPECT_EQ(local_data, dynamic.data());
    EXPECT_EQ(2, dynamic.size());

    // Test type conversion
    Span<int const, 2> const_span{local_span};
    EXPECT_EQ(local_data, const_span.data());
    EXPECT_EQ(2, const_span.size());

    // Test type conversion
    Span<int const> const_dynamic{local_span};
    EXPECT_EQ(local_data, const_dynamic.data());
    EXPECT_EQ(2, const_dynamic.size());

    // Test pointer constructor
    Span<int, 2> ptr_span(local_data, local_data + 2);
    EXPECT_EQ(local_data, ptr_span.data());
}

TEST(SpanTest, dynamic_size)
{
    int local_data[] = {123, 456, 789};
    Span<int> local_span(local_data);
    EXPECT_EQ(sizeof(int*) + sizeof(std::size_t), sizeof(local_span));
    EXPECT_EQ("{123,456,789}", span_to_string(local_span));

    EXPECT_EQ(local_data, local_span.begin());
    EXPECT_EQ(local_data + 3, local_span.end());
    EXPECT_EQ(local_data, local_span.data());
    EXPECT_EQ(3, local_span.size());
    EXPECT_EQ(3 * sizeof(int), local_span.size_bytes());
    EXPECT_FALSE(local_span.empty());
    EXPECT_EQ(&local_data[1], &(local_span[1]));
    EXPECT_EQ(123, local_span.front());
    EXPECT_EQ(789, local_span.back());

    {
        auto templ_subspan = local_span.subspan<1>();
        EXPECT_TRUE((std::is_same<Span<int>, decltype(templ_subspan)>::value));
        EXPECT_EQ(local_data + 1, templ_subspan.data());
        EXPECT_EQ(2, templ_subspan.size());
    }
    {
        auto templ_subspan = local_span.subspan<1, 2>();
        EXPECT_TRUE(
            (std::is_same<Span<int, 2>, decltype(templ_subspan)>::value));
        EXPECT_EQ(local_data + 1, templ_subspan.data());
        EXPECT_EQ(2, templ_subspan.size());
    }
    {
        auto templ_subspan = local_span.first<2>();
        EXPECT_TRUE(
            (std::is_same<Span<int, 2>, decltype(templ_subspan)>::value));
        EXPECT_EQ(local_data, templ_subspan.data());
        EXPECT_EQ(2, templ_subspan.size());
    }
    {
        auto templ_subspan = local_span.first(2);
        EXPECT_TRUE((std::is_same<Span<int>, decltype(templ_subspan)>::value));
        EXPECT_EQ(local_data, templ_subspan.data());
        EXPECT_EQ(2, templ_subspan.size());
    }
    {
        auto templ_subspan = local_span.last<2>();
        EXPECT_TRUE(
            (std::is_same<Span<int, 2>, decltype(templ_subspan)>::value));
        EXPECT_EQ(local_data + 1, templ_subspan.data());
        EXPECT_EQ(2, templ_subspan.size());
    }
    {
        auto templ_subspan = local_span.last(2);
        EXPECT_TRUE((std::is_same<Span<int>, decltype(templ_subspan)>::value));
        EXPECT_EQ(local_data + 1, templ_subspan.data());
        EXPECT_EQ(2, templ_subspan.size());
    }
    {
        auto templ_subspan = local_span.last(3);
        EXPECT_TRUE((std::is_same<Span<int>, decltype(templ_subspan)>::value));
        EXPECT_EQ(local_data, templ_subspan.data());
        EXPECT_EQ(3, templ_subspan.size());
    }
    if (CELERITAS_DEBUG)
    {
        EXPECT_THROW(local_span.last(4), DebugError);
        EXPECT_THROW(local_span.last<4>(), DebugError);
    }

    auto func_subspan = local_span.subspan(1);
    EXPECT_EQ(local_data + 1, func_subspan.data());
    EXPECT_EQ(2, func_subspan.size());

    // Test copy constructor
    Span<int> other_span{local_span};
    EXPECT_EQ(local_data, other_span.data());
    EXPECT_EQ(3, other_span.size());
    local_span = other_span;

    // Test dynamic conversion
    Span<int> dynamic{local_span};
    EXPECT_EQ(local_data, dynamic.data());
    EXPECT_EQ(3, dynamic.size());

    // Test type conversion
    Span<int const> const_dynamic{local_span};
    EXPECT_EQ(local_data, const_dynamic.data());
    EXPECT_EQ(3, const_dynamic.size());

    // Test type and size conversion
    Span<int const, 3> const_span{local_span};
    EXPECT_EQ(local_data, const_span.data());
    EXPECT_EQ(3, const_span.size());

    // Test pointer constructor
    Span<int> ptr_span(local_data, local_data + 3);
    EXPECT_EQ(local_data, ptr_span.data());
}

TEST(SpanTest, io_manip)
{
    int local_data[] = {123, 456, 789};
    Span<int> local_span(local_data);

    {
        std::ostringstream os;
        os << std::setw(20) << local_span;
        EXPECT_EQ("{   123,  456,  789}", os.str());
    }
    {
        std::ostringstream os;
        os << std::setw(3) << local_span;
        EXPECT_EQ("{123,456,789}", os.str());
    }
    {
        std::ostringstream os;
        os << std::setw(20) << std::left << local_span;
        EXPECT_EQ("{123   ,456  ,789  }", os.str());
    }
}
TEST(LdgSpanTest, pod)
{
    using LdgInt = LdgValue<int const>;
    int local_data[] = {123, 456, 789};
    Span<int> mutable_span(local_data);
    EXPECT_TRUE((std::is_same_v<decltype(mutable_span[0]), int&>));
    Span<LdgInt> ldg_span(mutable_span);
    Span<LdgInt> local_span(local_data);
    EXPECT_TRUE(
        (std::is_same_v<typename Span<LdgInt>::element_type, int const>));
    EXPECT_TRUE((std::is_same_v<decltype(local_span.data()), int const*>));
    EXPECT_TRUE((std::is_same_v<decltype(local_span.front()), int>));
    EXPECT_TRUE((std::is_same_v<decltype(local_span.back()), int>));
    EXPECT_TRUE((std::is_same_v<decltype(local_span[0]), int>));
    EXPECT_TRUE((
        std::is_same_v<decltype(local_span.begin()), LdgIterator<int const>>));
    EXPECT_TRUE(
        (std::is_same_v<decltype(local_span.end()), LdgIterator<int const>>));

    EXPECT_EQ(local_span.first(2).back(), 456);
    EXPECT_TRUE(
        (std::is_same_v<decltype(local_span), decltype(local_span.first(2))>));
    EXPECT_EQ(local_span.subspan(1, 1)[1], 789);

    auto begin = local_span.begin();
    EXPECT_EQ(*begin++, 123);
    EXPECT_EQ(*begin++, 456);
    EXPECT_EQ(*begin++, 789);
    EXPECT_EQ(begin, local_span.end());
    EXPECT_EQ(local_span[2], 789);
    EXPECT_EQ(local_span.end()[-3], 123);
}

TEST(LdgSpanTest, opaque_id)
{
    using SpanTestLdgOpaqueId = OpaqueId<struct SpanTestLdgOpaqueId_>;
    using LdgId = LdgValue<SpanTestLdgOpaqueId const>;
    SpanTestLdgOpaqueId local_data[] = {SpanTestLdgOpaqueId{123},
                                        SpanTestLdgOpaqueId{456},
                                        SpanTestLdgOpaqueId{789}};
    Span<SpanTestLdgOpaqueId> mutable_span(local_data);
    EXPECT_TRUE(
        (std::is_same_v<decltype(mutable_span[0]), SpanTestLdgOpaqueId&>));
    Span<LdgId> ldg_span(mutable_span);
    Span<LdgId> local_span(local_data);
    EXPECT_TRUE((std::is_same_v<typename Span<LdgId>::element_type,
                                SpanTestLdgOpaqueId const>));
    EXPECT_TRUE((
        std::is_same_v<decltype(local_span.data()), SpanTestLdgOpaqueId const*>));
    EXPECT_TRUE(
        (std::is_same_v<decltype(local_span.front()), SpanTestLdgOpaqueId>));
    EXPECT_TRUE(
        (std::is_same_v<decltype(local_span.back()), SpanTestLdgOpaqueId>));
    EXPECT_TRUE((std::is_same_v<decltype(local_span[0]), SpanTestLdgOpaqueId>));
    EXPECT_TRUE((std::is_same_v<decltype(local_span.begin()),
                                LdgIterator<SpanTestLdgOpaqueId const>>));
    EXPECT_TRUE((std::is_same_v<decltype(local_span.end()),
                                LdgIterator<SpanTestLdgOpaqueId const>>));

    EXPECT_EQ(local_span.first(2).back(), SpanTestLdgOpaqueId{456});
    EXPECT_TRUE(
        (std::is_same_v<decltype(local_span), decltype(local_span.first(2))>));
    EXPECT_EQ(local_span.subspan(1, 1)[1], SpanTestLdgOpaqueId{789});

    auto begin = local_span.begin();
    EXPECT_EQ(*begin++, SpanTestLdgOpaqueId{123});
    EXPECT_EQ(*begin++, SpanTestLdgOpaqueId{456});
    EXPECT_EQ(*begin++, SpanTestLdgOpaqueId{789});
    EXPECT_EQ(begin, local_span.end());
    EXPECT_EQ(local_span[2], SpanTestLdgOpaqueId{789});
    EXPECT_EQ(local_span.end()[-3], SpanTestLdgOpaqueId{123});
}

//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace celeritas
