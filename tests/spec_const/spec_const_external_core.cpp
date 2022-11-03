/*******************************************************************************
//
//  SYCL 2020 Conformance Test Suite
//
//  Provides tests for specialization constants with SYCL_EXTERNAL function
//
*******************************************************************************/

#include "../common/common.h"
#include "../common/type_coverage.h"

#define TEST_CORE

#include "spec_const_external.h"

#define TEST_NAME spec_const_external_core

namespace TEST_NAMESPACE {
using namespace sycl_cts;

/** test specialization constants
 */
class TEST_NAME : public sycl_cts::util::test_base {
 public:
  /** return information about this test
   */
  void get_info(test_base::info &out) const override {
    set_test_info(out, TOSTRING(TEST_NAME), TEST_FILE);
  }

  /** execute the test
   */
  void run(util::logger &log) override {
#ifndef SYCL_EXTERNAL
    log.note("SYCL_EXTERNAL is not defined");
#else
    using namespace spec_const_external;
    {

#if !SYCL_CTS_ENABLE_FULL_CONFORMANCE
      for_all_types<check_spec_const_external>(
          get_spec_const::testing_types::types, log);
#else
      for_all_types_vectors_marray<check_spec_const_external>(
          get_spec_const::testing_types::types, log);
#endif
      for_all_types<check_spec_const_external>(
          get_spec_const::testing_types::composite_types, log);
    }
#endif  // SYCL_EXTERNAL
  }
};

// construction of this proxy will register the above test
util::test_proxy<TEST_NAME> proxy;

} /* namespace TEST_NAMESPACE */
