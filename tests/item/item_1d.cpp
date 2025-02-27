/*******************************************************************************
//
//  SYCL 2020 Conformance Test Suite
//
//  Copyright (c) 2017-2022 Codeplay Software LTD. All Rights Reserved.
//  Copyright (c) 2022-2023 The Khronos Group Inc.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
*******************************************************************************/

#include "../common/common.h"

#include <algorithm>

#define TEST_NAME item_1d

namespace test_item_1d__ {
using namespace sycl_cts;

class kernel_item_1d {
 protected:
  using read_access_t =
      sycl::accessor<int, 1, sycl::access_mode::read, sycl::target::device>;
  using write_access_t =
      sycl::accessor<int, 1, sycl::access_mode::write, sycl::target::device>;

  read_access_t in;
  write_access_t out;
  write_access_t out_deprecated;
  sycl::range<1> r_exp;
  sycl::id<1> offset_exp;

 public:
  kernel_item_1d(read_access_t in_, write_access_t out_,
                 write_access_t out_deprecated_, sycl::range<1> r)
      : in(in_),
        out(out_),
        out_deprecated(out_deprecated_),
        r_exp(r),
        offset_exp(sycl::id<1>(0)) {}

  void operator()(sycl::item<1> item) const {
    bool all_correct = true;
    sycl::id<1> gid = item.get_id();

    all_correct &= gid.get(0) == item.get_id(0);
    all_correct &= gid.get(0) == item[0];

    sycl::range<1> localRange = item.get_range();
    all_correct &= localRange == r_exp;

#if SYCL_CTS_ENABLE_DEPRECATED_FEATURES_TESTS
    sycl::id<1> offset = item.get_offset();
    out_deprecated[item] = offset == offset_exp;
#endif

    /* get work item range */
    const size_t nWidth = item.get_range(0);
    all_correct &= nWidth == r_exp.get(0);

    /* find the array id for this work item */
    size_t index = gid.get(0); /* x */

    /* get the global linear id and compare against precomputed index */
    const size_t glid = item.get_linear_id();
    all_correct &= in[int(glid)] == static_cast<int>(index);

    /* operator size_t() const */
#ifndef SYCL_CTS_COMPILING_WITH_COMPUTECPP
    size_t val = item;
#else
    // value that is guaranteed to produce a failure
    size_t val = ~static_cast<size_t>(0);
#endif
    all_correct &= val == item.get_id(0);
    const sycl::item<1> item_const = item;
#ifndef SYCL_CTS_COMPILING_WITH_COMPUTECPP
    val = item_const;
#else
    // value that is guaranteed to produce a failure
    val = ~static_cast<size_t>(0);
#endif
    all_correct &= val == item.get_id(0);

    out[int(glid)] = all_correct;
  }
};

void buffer_fill(int *buf, const int nWidth) {
  for (int i = 0; i < nWidth; i++) buf[i] = i;
}

void test_item_1d(util::logger &log) {
  const int nWidth = 64;

  /* allocate and clear host buffers */
  std::vector<int> dataIn(nWidth);
  std::vector<int> dataOut(nWidth);
  std::vector<int> dataOutDeprecated(nWidth);

  buffer_fill(dataIn.data(), nWidth);

  {
    sycl::range<1> dataRange(nWidth);

    sycl::buffer<int, 1> bufIn(dataIn.data(), dataRange);
    sycl::buffer<int, 1> bufOut(dataOut.data(), dataRange);
    sycl::buffer<int, 1> bufOutDeprecated(dataOutDeprecated.data(), dataRange);

    auto cmdQueue = util::get_cts_object::queue();

    cmdQueue.submit([&](sycl::handler &cgh) {
      auto accIn = bufIn.template get_access<sycl::access_mode::read>(cgh);
      auto accOut = bufOut.template get_access<sycl::access_mode::write>(cgh);
      auto accOutDeprecated =
          bufOutDeprecated.template get_access<sycl::access_mode::write>(cgh);

      kernel_item_1d kern =
          kernel_item_1d(accIn, accOut, accOutDeprecated, dataRange);
      cgh.parallel_for(dataRange, kern);
    });

    cmdQueue.wait_and_throw();
  }

  // check api call results
  CHECK(
      std::all_of(dataOut.begin(), dataOut.end(), [](int val) { return val; }));

#if SYCL_CTS_ENABLE_DEPRECATED_FEATURES_TESTS
  // check deprecated api call results
  CHECK(std::all_of(dataOutDeprecated.begin(), dataOutDeprecated.end(),
                    [](int val) { return val; }));
#endif

  STATIC_CHECK_FALSE(std::is_default_constructible_v<sycl::item<1>>);
}

/** test sycl::device initialization
 */
class TEST_NAME : public util::test_base {
 public:
  /** return information about this test
   */
  void get_info(test_base::info &out) const override {
    set_test_info(out, TOSTRING(TEST_NAME), TEST_FILE);
  }

  /** execute the test
   */
  void run(util::logger &log) override {
#ifdef SYCL_CTS_COMPILING_WITH_COMPUTECPP
    log.note("ComputeCpp does not provide size_t operator.");
#endif
    test_item_1d(log);
  }
};

// construction of this proxy will register the above test
util::test_proxy<TEST_NAME> proxy;

} /* namespace test_item_1d__ */
