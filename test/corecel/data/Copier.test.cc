//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file corecel/data/Copier.test.cc
//---------------------------------------------------------------------------//
#include "corecel/data/Copier.hh"

#include <vector>

#include "corecel/data/DeviceVector.hh"

#include "celeritas_test.hh"

namespace celeritas
{
namespace test
{
//---------------------------------------------------------------------------//

TEST(CopierTest, host)
{
    // Copy host --> host
    std::vector<int> src_vec(128, 1234);
    std::vector<int> dst_vec(src_vec.size() + 1);

    Copier<int, MemSpace::host> copy{
        {dst_vec.data() + 1, dst_vec.data() + dst_vec.size()}};
    copy(MemSpace::host, make_span(src_vec));
    EXPECT_EQ(0, dst_vec.front());
    EXPECT_EQ(1234, dst_vec[1]);
    EXPECT_EQ(1234, dst_vec.back());
}

TEST(CopierTest, TEST_IF_CELER_DEVICE(device))
{
    // Copy host --> device
    std::vector<int> host_vec(128);
    host_vec.front() = 1;
    host_vec.back() = 1234;
    DeviceVector<int> device_vec(host_vec.size());
    {
        Copier<int, MemSpace::device> copy{device_vec.device_ref()};
        copy(MemSpace::host, make_span(host_vec));
    }

    // Copy device --> device
    DeviceVector<int> new_device_vec(host_vec.size());
    {
        Copier<int, MemSpace::device> copy{new_device_vec.device_ref()};
        copy(MemSpace::device, device_vec.device_ref());
    }

    // Copy device --> host
    std::vector<int> new_host_vec(host_vec.size());
    {
        Copier<int, MemSpace::host> copy{make_span(new_host_vec)};
        copy(MemSpace::device, new_device_vec.device_ref());
    }
    EXPECT_EQ(1, new_host_vec.front());
    EXPECT_EQ(1234, new_host_vec.back());
}

TEST(ItemCopierTest, TEST_IF_CELER_DEVICE(device))
{
    // Initialize data on device
    std::vector<int> host_vec{0, 1, 2, 3, 4};
    DeviceVector<int> device_vec(host_vec.size());
    device_vec.copy_to_device(make_span(host_vec));

    // Copy elements to host
    ItemCopier<int> copy{};
    for (int i : range(host_vec.size()))
    {
        int result = copy(device_vec.data() + i);
        EXPECT_EQ(i, result);
    }
}

//---------------------------------------------------------------------------//
}  // namespace test
}  // namespace celeritas
